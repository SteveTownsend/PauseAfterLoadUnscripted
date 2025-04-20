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
#pragma once


constexpr RE::FormID ClothKeyword = 0x06BBE8;
constexpr RE::FormID CurrentFollowerFaction = 0x0005C84E;

constexpr double DistanceUnitInFeet = 0.046875;
constexpr double FeetPerMile = 5280.0;
constexpr double DistanceUnitInMiles = DistanceUnitInFeet / FeetPerMile;

namespace FileUtils
{
	std::wstring GetGamePath(void);
	std::wstring GetPluginFileName(void) noexcept;
	std::wstring GetPluginPath(void) noexcept;
	inline bool CanOpenFile(const char* fileName)
	{
		std::ifstream ifs(fileName);
		return ifs.fail() ? false : true;
	}
}

namespace utils
{
	double GetGameSettingFloat(const RE::BSFixedString& name);
}

namespace WindowsUtils
{
	unsigned long long microsecondsNow();
	void LogProcessWorkingSet();
	void TakeNap(const double delaySeconds);

	class ScopedTimer {
	public:
		ScopedTimer(const std::string& context);
		~ScopedTimer();
	private:
		ScopedTimer();
		ScopedTimer(const ScopedTimer&);
		ScopedTimer& operator=(ScopedTimer&);

		unsigned long long m_startTime;
		std::string m_context;
	};

	class ScopedTimerFactory
	{
	public:
		static ScopedTimerFactory& Instance();
		ScopedTimerFactory() : m_nextHandle(0) {}
		int StartTimer(const std::string& context);
		void StopTimer(const int handle);

	private:
		static std::unique_ptr<ScopedTimerFactory> m_instance;
		RecursiveLock m_timerLock;
		std::unordered_map<int, std::unique_ptr<ScopedTimer>> m_timerByHandle;
		int m_nextHandle;
	};
}

namespace FormUtils
{
	// This can be missing, e.g. "Elementary Destruction.esp" FormID 0x31617, Github issue #28
	// Certain forms such as CONT do not load this at runtime, see 
	// https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/issues/20
	inline std::string SafeGetFormEditorID(const RE::TESForm* form)
	{
		const char* edid(form ? form->GetFormEditorID() : nullptr);
		return edid ? std::string(edid) : std::string();
	}

	inline bool IsConcrete(const RE::TESForm* form)
	{
		return form && form->GetPlayable() && !std::string(form->GetName()).empty();
	}
}

namespace StringUtils
{
	std::string FromUnicode(const std::wstring& input);
}