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

	bool StartPause()
	{
		bool result(false);

		// check controls state
		auto controls = RE::ControlMap::GetSingleton();
		if (!controls)
		{
			REL_ERROR("ControlMap Singleton not valid");
			return false;
		}
		if (controls->IsPOVSwitchControlsEnabled() &&
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
				bool expected(false);
				bool desired(true);
				if (_paused.compare_exchange_strong(expected, desired))
				{
					REL_MESSAGE("OK to freeze time");
					result = true;

					// Activate InputHandler here, blocks input until ProgressPause called
					_listener->Enable();

					// pause game using CLSSE 'easy button'
					RE::Main::GetSingleton()->freezeTime = true;
				}
				else
				{
					REL_WARNING("Already paused, ignore new request");
				}
			}
		}
		else
		{
			REL_WARNING("Controls-Enabled State not all true: fighting {} looking {} menu {} movement {} sneaking {}",
				controls->IsFightingControlsEnabled(),
				controls->IsLookingControlsEnabled(),
				controls->IsMenuControlsEnabled(),
				controls->IsMovementControlsEnabled(),
				controls->IsSneakingControlsEnabled());
		}
		return result;
	}

	void ProgressPause()
	{
		// allow input listener to filter without blocking all
		const double ignoreInput(SettingsCache::Instance().CanUnpauseAfter());
		_listener->PrepareToUnpause(ignoreInput);

		// Optionally, resume after configured delay
		double delay(SettingsCache::Instance().ResumeAfter());
		bool expected2(false);
		bool desired2(true);
		if (delay > 0.0 && _delayed.compare_exchange_strong(expected2, desired2))
		{
			REL_DMESSAGE("Resume game if no input for {:.1f} seconds, ignoring input for {:.1f} seconds", delay, ignoreInput);
			_timer.expires_from_now(boost::posix_time::millisec(static_cast<int>((delay + ignoreInput) * 1000.0)));
			_timer.async_wait([this](const boost::system::error_code& ec) {
				if (!ec)
				{
					REL_DMESSAGE("Pause timed out");
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

protected:

	/*
	 * menu handling - on the first pass after launch, we do not get a menu-opened event and should not pause
	 * on menu-closed event, so we use the menu-opened event to prime the menu-closed event
	 *
		2022-01-03 19:10:38.871     info  35716 Plugin Data load complete!
		2022-01-03 19:10:38.871     info  35716 Initialized Data, Pause available
		2022-01-03 19:10:39.176     info  41680 Loading Menu closed
		2022-01-03 19:10:39.176     info  41680 OK to pause
		2022-01-03 19:10:39.176    debug  41680 Ran Console command sgtm 0.001
		2022-01-03 19:10:39.176    debug  41680 Resume game if no input for 10.0 seconds, ignoring input for 20.0 seconds
		2022-01-03 19:10:39.177    debug  43452 Starting timer thread
		2022-01-03 19:10:42.849     info  41680 kPreLoadGame message
		2022-01-03 19:10:42.979     info  41680 Loading Menu opened
		2022-01-03 19:10:44.813     info  41680 kPostLoadGame message
		2022-01-03 19:10:56.182     info  41680 Loading Menu closed
		2022-01-03 19:10:56.182     info  41680 OK to pause
		2022-01-03 19:10:56.182  warning  41680 Already paused, ignore new request
		2022-01-03 19:10:59.183     info  12576 CanUnpauseAfter timer expired 6 milliseconds ago
		2022-01-03 19:11:01.120     info  12576 Pause terminated by Input Event type 0
		2022-01-03 19:11:01.120    debug  12576 Cancel active pause timer
		2022-01-03 19:11:01.120    debug  43452 Exiting timer thread
		2022-01-03 19:11:01.120    debug  12576 Ran Console command sgtm 1.000
	 *
	 */
	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
	{
		auto intfcStr = RE::InterfaceStrings::GetSingleton();
		if (intfcStr &&
			a_event &&
			a_event->menuName == intfcStr->loadingMenu) {
			if (!a_event->opening)
			{
				// skip ProgressPause() if this is first pass after process launch, per log output above
				if (_canPause)
				{
					// Loading Menu closed - need to pause
					REL_MESSAGE("Loading Menu closed after preceding Opened event - pause OK");
					ProgressPause();
					_canPause = false;
				}
				else
				{
					REL_MESSAGE("Loading Menu closed without preceding Opened event - no pause");
				}
			}
			// to confirm timings wrt Loading Menu handling
			else
			{
				// set the stage for Pause when the corresponding menu-closed arrives
				_canPause = StartPause();
				REL_MESSAGE("Loading Menu opened - pause OK {}", _canPause);
			}
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
		// Use active effects directly - CLSSE has no appropriate function
		auto effects = RE::PlayerCharacter::GetSingleton()->GetActiveEffectList();
		if (!effects) {
			return false;
		}

		RE::EffectSetting* setting = nullptr;
		for (auto& effect : *effects) {
			setting = effect ? effect->GetBaseObject() : nullptr;
			if (!setting)
				continue;
			if (_slowTimeEffects.contains(setting))
			{
				// Inactive effects should not prevent Pause
				if (effect->flags.any(RE::ActiveEffect::Flag::kInactive))
				{
					REL_DMESSAGE("Skip Inactive SlowTime or ValueModifier-BowSpeedBonus archetype effect : {}({:08x})",
						setting->GetName(), setting->GetFormID());
					continue;
				}
				REL_WARNING("Player subject to Active non-constant-cast SlowTime or ValueModifier-BowSpeedBonus archetype effect : {}({:08x})",
					setting->GetName(), setting->GetFormID());
				return true;
			}
		}
		return false;
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
			REL_DMESSAGE("Restart game");
			_listener->Disable();
			// Resume game
			RE::Main::GetSingleton()->freezeTime = false;
		}
		else
		{
			REL_WARNING("Already unpaused, ignore new request");
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
	// indicates not first pass after launch - menu-closed must be preceded by menu-opened
	bool _canPause{ false };
	// acts as a guard for event sink management
	std::atomic<bool> _paused{ false };
	std::atomic<bool> _delayed{ false };
	std::unordered_set<RE::EffectSetting*> _slowTimeEffects;
	boost::asio::io_service _io_service;
	boost::asio::deadline_timer _timer;
	std::optional<std::jthread> _thread;
};

}
