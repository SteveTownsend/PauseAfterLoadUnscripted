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
#include "PrecompiledHeaders.h"

#include "Data/SimpleIni.h"
#include "Data/SettingsCache.h"
#include "Utilities/utils.h"

namespace palu
{

std::unique_ptr<SettingsCache> SettingsCache::m_instance;

SettingsCache& SettingsCache::Instance()
{
	if (!m_instance)
	{
		m_instance = std::make_unique<SettingsCache>();
	}
	return *m_instance;
}

void SettingsCache::Refresh(void)
{
	SimpleIni ini;
	const std::string inFile(GetFileName());
	if (!ini.Load(inFile))
	{
		REL_WARNING("Settings cache load from {} failed, using defaults", inFile.c_str());
	}
	else
	{
		REL_MESSAGE("Refresh settings cache from valid file {}", inFile.c_str());
		for (auto section = ini.beginSection(); section != ini.endSection(); ++section)
		{
			DBG_MESSAGE("Section {}", *section);
			for (auto key = ini.beginKey(*section); key != ini.endKey(*section); ++key)
			{
				DBG_MESSAGE("Entry {}={}", *key, !key);
			}
		}
	}
	// SimpleIni normalizes names to lowercase
	_resumeAfter = ini.GetValue<double>(SectionName, "resumeafter", DefaultResumeAfter);
	REL_VMESSAGE("ResumeAfter = {:.1f} seconds", _resumeAfter);
	_pausedSGTM = ini.GetValue<double>(SectionName, "pausedsgtm", DefaultPausedSGTM);
	REL_VMESSAGE("PausedSGTM = {:.3f}", _pausedSGTM);
	_normalSGTM = ini.GetValue<double>(SectionName, "normalsgtm", DefaultNormalSGTM);
	REL_VMESSAGE("NormalSGTM = {:.3f}", _normalSGTM);
}

const std::string SettingsCache::GetFileName() const
{
	std::string iniFilePath;
	std::string RuntimeDir = FileUtils::GetGamePath();
	if (RuntimeDir.empty())
		return "";

	iniFilePath = RuntimeDir + "Data\\SKSE\\Plugins\\" + IniFileName;
	return iniFilePath;
}

}