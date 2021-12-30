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
			REL_MESSAGE("Pause terminated by Input Event type {}", a_event->eventType.underlying());
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
		if (a_event) {
			(*_callback)(*a_event, _resultHandler);
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	std::unique_ptr<InputHandler> _callback;
	std::function<void(void)> _resultHandler;
};

}