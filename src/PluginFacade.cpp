/*************************************************************************
PauseAfterLoadUnscripted
Copyright (c) Steve Townsend 2020

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
#include "PrecompiledHeaders.h"
#include "PluginFacade.h"

#include "Utilities/LogStackWalker.h"
#include "Utilities/utils.h"

namespace palu
{

std::unique_ptr<PluginFacade> PluginFacade::m_instance;

PluginFacade& PluginFacade::Instance()
{
	if (!m_instance)
	{
		m_instance = std::make_unique<PluginFacade>();
	}
	return *m_instance;
}

PluginFacade::PluginFacade() : m_loadProgress(LoadProgress::NotStarted), m_threadStarted(false), m_pluginSynced(false), m_loadedSettings(false)
{
}

bool PluginFacade::OneTimeLoad(void)
{
	__try
	{
		// Use structured exception handling during game data load
		REL_MESSAGE("Plugin not initialized - Game Data load executing");
		WindowsUtils::LogProcessWorkingSet();
		if (!Load())
			return false;
		m_loadProgress = LoadProgress::Complete;
		WindowsUtils::LogProcessWorkingSet();
	}
	__except (LogStackWalker::LogStack(GetExceptionInformation()))
	{
		REL_FATALERROR("Fatal Exception during Game Data load");
		return false;
	}
	return true;
}

bool PluginFacade::Init()
{
	// Thread safety is vital to ensure Load() only fires once.
	// cf. https://github.com/SteveTownsend/SmartHarvestSE/issues/230
	// SKSE::MessagingInterface::kPostLoadGame fired twice
	bool loadRequired(false);
	{
		RecursiveLockGuard guard(m_pluginLock);
		if (m_loadProgress == LoadProgress::NotStarted)
		{
			m_loadProgress = LoadProgress::Started;
			loadRequired = true;
		}
		else if (m_loadProgress == LoadProgress::Started)
		{
			return false;
		}
		// LoadProgress::LoadComplete
	}
	if (loadRequired)
	{
		if (!OneTimeLoad())
			return false;
	}
	return true;
}

bool PluginFacade::Load()
{
#ifdef _PROFILING
	WindowsUtils::ScopedTimer elapsed("Startup: Load Game Data");
#endif
	RE::TESDataHandler* dhnd = RE::TESDataHandler::GetSingleton();
	if (!dhnd)
		return false;

	// List MGEF with Slow Time archetype, save the ones with Value Modifier on Bow Speed Bonus
	for (RE::EffectSetting* effect : dhnd->GetFormArray<RE::EffectSetting>())
	{
		if (effect->HasArchetype(RE::EffectSetting::Archetype::kSlowTime))
		{
			REL_VMESSAGE("SlowTime Magic Effect : {}({:08x})", effect->GetName(), effect->GetFormID());
		}
		else if (effect->HasArchetype(RE::EffectSetting::Archetype::kValueModifier) &&
			effect->data.primaryAV == RE::ActorValue::kBowSpeedBonus)
		{
			REL_VMESSAGE("ValueModifier-BowSpeedBonus Magic Effect : {}({:08x})", effect->GetName(), effect->GetFormID());
			m_slowTimeEffects.insert(effect);
		}
	}

	REL_MESSAGE("Plugin Data load complete!");
	return true;
}

void PluginFacade::PrepareForReloadOrNewGame()
{
	// Do not scan again until we are in sync with the scripts
	RecursiveLockGuard guard(m_pluginLock);
	m_pluginSynced = false;
	m_loadedSettings = false;	// this comes from MCM script via OnGameReady
	REL_MESSAGE("Plugin sync required");
}

void PluginFacade::ResetTransientState(const bool gameReload)
{
	DBG_MESSAGE("Transient in-game restrictions reset, reload={}", gameReload ? "true" : "false");
	// This can be called while LocationTracker lock is held. No deadlock at present but care needed to ensure it remains so
	RecursiveLockGuard guard(m_pluginLock);
}

void PluginFacade::OnVMSync()
{
	REL_MESSAGE("Plugin sync, VM ready");
	WindowsUtils::LogProcessWorkingSet();
	ResetTransientState(true);
	// reset player state

	// need to wait for the scripts to sync up before performing player house checks
	m_pluginSynced = true;
	REL_MESSAGE("Plugin sync completed");
	WindowsUtils::LogProcessWorkingSet();
}

void PluginFacade::OnGameLoaded()
{
	REL_MESSAGE("MCM OnGameLoaded completed");
	m_loadedSettings = true;
}

}
