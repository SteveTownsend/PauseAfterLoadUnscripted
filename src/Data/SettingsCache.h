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
	// delay acceptance of unpause input - set in seconds - 0.0 means allow immediate unpause
	[[nodiscard]] double CanUnpauseAfter() const { return _canUnpauseAfter; }
	// delay the pause of the game engine to allow CELL setup to complete
	[[nodiscard]] double PauseDelay() const { return _pauseDelay; }
	//optional pause-on-save
	[[nodiscard]] bool PauseOnSave() const { return _pauseOnSave; }
	//optional pause-on-save
	[[nodiscard]] bool PauseOnLoad() const { return _pauseOnLoad; }
	//optional pause-on-save
	[[nodiscard]] bool PauseOnLoadScreen() const { return _pauseOnLoadScreen; }
	// input filtering
	[[nodiscard]] bool IgnoreKeyPressAndButton() const { return _ignoreKeyPressAndButton; }
	[[nodiscard]] bool IgnoreMouseMove() const { return _ignoreMouseMove; }
	[[nodiscard]] bool IgnoreThumbstick() const { return _ignoreThumbstick; }

private:
	const std::wstring GetFileName() const;

	static std::unique_ptr<SettingsCache> m_instance;

	// SimpleIni normalizes to lowercase
	inline static const char * SectionName = "pause";
	inline static const wchar_t* IniFileName = L"PauseAfterLoadUnscripted.ini";
	static constexpr double DefaultResumeAfter = 5.0;
	static constexpr double DefaultCanUnpauseAfter = 0.0;
	static constexpr double DefaultPauseDelay = 1.0;
	static constexpr bool DefaultPauseOnSave = false;
	static constexpr bool DefaultPauseOnLoad = true;
	static constexpr bool DefaultPauseOnLoadScreen = true;
	static constexpr bool DefaultIgnoreKeyPressAndButton = false;
	static constexpr bool DefaultIgnoreMouseMove = true;
	static constexpr bool DefaultIgnoreThumbstick = true;

	double _resumeAfter = DefaultResumeAfter;
	double _canUnpauseAfter = DefaultCanUnpauseAfter;
	double _pauseDelay = DefaultPauseDelay;
	bool _pauseOnSave = DefaultPauseOnSave;
	bool _pauseOnLoad = DefaultPauseOnLoad;
	bool _pauseOnLoadScreen = DefaultPauseOnLoadScreen;
	bool _ignoreKeyPressAndButton = DefaultIgnoreKeyPressAndButton;
	bool _ignoreMouseMove = DefaultIgnoreMouseMove;
	bool _ignoreThumbstick = DefaultIgnoreThumbstick;
};

}