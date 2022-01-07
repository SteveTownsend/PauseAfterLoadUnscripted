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

// stripped down from Quick Loot RE InputListeners.h
class InputHandler
{
public:
	InputHandler() = default;
	void operator()(RE::InputEvent* const& a_event, std::function<void(void)> resultHandler)
	{
		if (a_event)
		{
			auto eventType(a_event->GetEventType());
			if ((eventType == RE::INPUT_EVENT_TYPE::kButton && SettingsCache::Instance().IgnoreKeyPressAndButton()) ||
				(eventType == RE::INPUT_EVENT_TYPE::kMouseMove && SettingsCache::Instance().IgnoreMouseMove()) ||
				(eventType == RE::INPUT_EVENT_TYPE::kThumbstick && SettingsCache::Instance().IgnoreThumbstick()))
			{
				return;
			}
			REL_MESSAGE("Pause terminated by Input Event type {} from Device {}",
				a_event->eventType.underlying(), a_event->device.underlying());
			resultHandler();
		}
	}
};

class InputListener :
	public RE::BSTEventSink<RE::InputEvent*>
{
public:
	InputListener() = delete;
	InputListener(std::function<void(void)> resultHandler)
	{
		_resultHandler = resultHandler;
		_callback = std::make_unique<InputHandler>();
	}

	InputListener(const InputListener&) = default;
	InputListener(InputListener&&) = default;

	~InputListener() { Disable(); }

	InputListener& operator=(const InputListener&) = default;
	InputListener& operator=(InputListener&&) = default;

	void Enable()
	{
		auto input = RE::BSInputDeviceManager::GetSingleton();
		if (input) {
			input->AddEventSink(this);
		}
	}

	void PrepareToUnpause(const double ignoreInput)
	{
		_delayUnpause = ignoreInput > 0.0;
		if (_delayUnpause)
		{
			_delayExpiry = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(static_cast<long long>(ignoreInput * 1000.0));
		}
	}

	void Disable()
	{
		auto input = RE::BSInputDeviceManager::GetSingleton();
		if (input) {
			input->RemoveEventSink(this);
		}
	}

private:
	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override
	{
		if (a_event && *a_event) {
			if (_delayUnpause)
			{
				const std::chrono::milliseconds timeAfterExpiry(
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _delayExpiry));
				if (timeAfterExpiry.count() > 0)
				{
					_delayUnpause = false;
					REL_MESSAGE("CanUnpauseAfter timer expired {} milliseconds ago", timeAfterExpiry.count());
				}
			}
			if (!_delayUnpause)
			{
				(*_callback)(*a_event, _resultHandler);
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	std::unique_ptr<InputHandler> _callback;
	std::function<void(void)> _resultHandler;
	std::chrono::time_point<std::chrono::high_resolution_clock> _delayExpiry;
	bool _delayUnpause{ false };
};

}