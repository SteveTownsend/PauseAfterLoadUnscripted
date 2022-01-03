#pragma once
/*************************************************************************
PauseAfterLoadUnscripted
Copyright (c) Steve Townsend 2021

>>> SOURCE LICENSE >>>
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation (www.fsf.org); either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is available at
http://www.fsf.org/licensing/licenses
>>> END OF LICENSE >>>
*************************************************************************/

// stripped down from Quick Loot RE ViewHandler.h
// Pause logic per wsPauseQuestAliasScript.psc, courtesy of wskeever and bobbyclue
#include <format>
#include <boost/asio.hpp>

#include "Pausing/InputListener.h"
#include "Data/SettingsCache.h"

namespace palu
{

class PauseHandler :
	public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	PauseHandler(const PauseHandler&) = default;

	PauseHandler(PauseHandler&&) = default;

	PauseHandler() : _timer(_io_service), _thread()
	{
		_listener = std::make_unique<InputListener>(std::bind(&PauseHandler::Unpause, this));
		Register();
		LoadData();
	}

	~PauseHandler()
	{
		Unregister();
		_listener.reset();
	}

	PauseHandler& operator=(const PauseHandler&) = default;
	PauseHandler& operator=(PauseHandler&&) = default;

	void Pause()
	{
		bool expected(false);
		bool desired(true);
		if (_paused.compare_exchange_strong(expected, desired))
		{
			_paused = true;
			_listener->Enable();
			ExecuteCommand(std::format("sgtm {:.3f}", SettingsCache::Instance().PausedSGTM()));
			// Optionally, resume after configured delay
			double delay(SettingsCache::Instance().ResumeAfter());
			bool expected2(false);
			bool desired2(true);
			if (delay > 0.0 && _delayed.compare_exchange_strong(expected2, desired2))
			{
				REL_DMESSAGE("Resume game if no input for {:.1f} seconds", delay);
				_timer.expires_from_now(boost::posix_time::millisec(static_cast<int>(delay * 1000.0)));
				_timer.async_wait([this](const boost::system::error_code& ec) {
					if (!ec)
					{
						REL_DMESSAGE("Pause timed out - restart game");
						Unpause();
					}
				});
				// Start IO Service to handle timer
				if (_thread.has_value())
				{
					_thread.reset();
				}
				_thread.emplace(std::bind(&PauseHandler::IOService, this));
			}
		}
	}

protected:

	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
	{
		auto intfcStr = RE::InterfaceStrings::GetSingleton();
		if (intfcStr &&
			a_event &&
			a_event->menuName == intfcStr->loadingMenu) {
			if (!a_event->opening)
			{
				// Loading Menu closed - need to pause
				REL_MESSAGE("Loading Menu closed");
				// check controls state
				auto controls = RE::ControlMap::GetSingleton();
				if (controls && controls->IsPOVSwitchControlsEnabled() &&
					controls->IsFightingControlsEnabled() &&
					// TODO map this over from the script
					// Game.IsJournalControlsEnabled() &&
					controls->IsLookingControlsEnabled() &&
					controls->IsMenuControlsEnabled() &&
					controls->IsMovementControlsEnabled() &&
					controls->IsSneakingControlsEnabled())
				{
					// If player is in Slow Time then do not pause
					if (!IsSlowTimeEffectActive())
					{
						REL_MESSAGE("OK to pause");
						Pause();
					}
				}
			}
#ifdef _DEBUG
			// to confirm timings wrt Loading Menu handling
			else
			{
				REL_MESSAGE("Loading Menu opened");
			}
#endif
		}

		return RE::BSEventNotifyControl::kContinue;
	}

private:

	void Register()
	{
		auto menuSrc = RE::UI::GetSingleton();
		if (menuSrc) {
			menuSrc->AddEventSink(this);
		}
	}

	void Unregister()
	{
		auto menuSrc = RE::UI::GetSingleton();
		if (menuSrc) {
			menuSrc->RemoveEventSink(this);
		}
	}

	bool LoadData()
	{
#ifdef _PROFILING
		WindowsUtils::ScopedTimer elapsed("Startup: Load Game Data");
#endif
		RE::TESDataHandler* dhnd = RE::TESDataHandler::GetSingleton();
		if (!dhnd)
			return false;

		// Save MGEF with Slow Time archetype and with Value Modifier on Bow Speed Bonus.
		// Constant Effects skipped to avoid inoperable state.
		for (RE::EffectSetting* effect : dhnd->GetFormArray<RE::EffectSetting>())
		{
			if (effect->HasArchetype(RE::EffectSetting::Archetype::kSlowTime) &&
				effect->data.castingType != RE::MagicSystem::CastingType::kConstantEffect)
			{
				REL_VMESSAGE("SlowTime Magic Effect : {}({:08x})", effect->GetName(), effect->GetFormID());
				_slowTimeEffects.insert(effect);
			}
			else if (effect->HasArchetype(RE::EffectSetting::Archetype::kValueModifier) &&
				effect->data.primaryAV == RE::ActorValue::kBowSpeedBonus &&
				effect->data.castingType != RE::MagicSystem::CastingType::kConstantEffect)
			{
				REL_VMESSAGE("ValueModifier-BowSpeedBonus Magic Effect : {}({:08x})", effect->GetName(), effect->GetFormID());
				_slowTimeEffects.insert(effect);
			}
		}

		REL_MESSAGE("Plugin Data load complete!");
		return true;
	}

	[[nodiscard]] bool IsSlowTimeEffectActive() const
	{
		return std::ranges::any_of(_slowTimeEffects, [=](auto effect) -> bool
		{
			if (RE::PlayerCharacter::GetSingleton()->HasMagicEffect(effect))
			{
				REL_DMESSAGE("Player subject to non-constant-cast SlowTime or ValueModifier-BowSpeedBonus archetype effect");
				return true;
			}
			return false;
		});
	}

	void Unpause()
	{
		// cancel delay timer if active
		bool expected(true);
		bool desired(false);
		if (_delayed.compare_exchange_strong(expected, desired))
		{
			REL_DMESSAGE("Cancel active pause timer");
			_timer.cancel();
		}

		bool expected2(true);
		bool desired2(false);
		if (_paused.compare_exchange_strong(expected2, desired2))
		{
			_paused = false;
			_listener->Disable();
			// Resume game at normal speed
			ExecuteCommand(std::format("sgtm {:.3f}", SettingsCache::Instance().NormalSGTM()));
		}
	}

	void ExecuteCommand(std::string a_command)
	{
		const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		const std::unique_ptr<RE::Script> script(scriptFactory ? scriptFactory->Create() : nullptr);
		if (script) {
			const RE::NiPointer<RE::TESObjectREFR> selectedRef;
			script->SetCommand(a_command);
			script->CompileAndRun(selectedRef.get());
			REL_DMESSAGE("Ran Console command {}", a_command);
		}
	}

	void IOService()
	{
		REL_DMESSAGE("Starting timer thread");
		_io_service.run_one();
		_io_service.restart();
		REL_DMESSAGE("Exiting timer thread");
	}

	std::unique_ptr<InputListener> _listener;
	// acts as a guard for event sink management
	std::atomic<bool> _paused{ false };
	std::atomic<bool> _delayed{ false };
	std::unordered_set<RE::EffectSetting*> _slowTimeEffects;
	boost::asio::io_service _io_service;
	boost::asio::deadline_timer _timer;
	std::optional<std::jthread> _thread;
};

}
