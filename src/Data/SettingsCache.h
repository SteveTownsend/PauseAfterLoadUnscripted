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

namespace palu
{

class SettingsCache {
public:
	static SettingsCache& Instance();
	SettingsCache() = default;

	void Refresh();

	// time out the pause - set in seconds - 0.0 means wait for user input before resuming
	[[nodiscard]] double ResumeAfter() const { return _resumeAfter; }
	// Pause and Resume values for sgtm
	[[nodiscard]] double PausedSGTM() const { return _pausedSGTM; }
	[[nodiscard]] double NormalSGTM() const { return _normalSGTM; }

private:
	const std::string GetFileName() const;

	static std::unique_ptr<SettingsCache> m_instance;

	inline static const char * SectionName = "Pause";
	inline static const char* IniFileName = "PauseAfterLoadUnscripted.ini";
	static constexpr double DefaultResumeAfter = 3.0;
	static constexpr double DefaultPausedSGTM = 0.001;
	static constexpr double DefaultNormalSGTM = 1.0;

	double _resumeAfter = DefaultResumeAfter;
	double _pausedSGTM = DefaultPausedSGTM;
	double _normalSGTM = DefaultNormalSGTM;
};

}