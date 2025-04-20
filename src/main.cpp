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

#include "Data/SettingsCache.h"
#include "Pausing/PauseHandler.h"
#include "Utilities/version.h"
#if _DEBUG
#include "Utilities/LogStackWalker.h"
#endif

#include <ShlObj.h>
#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>

#define DLLEXPORT __declspec(dllexport)

std::shared_ptr<spdlog::logger> PALULogger;
const std::string LoggerName = "PALU_Logger";
const std::string LogLevelVariable = "PALULogLevel";

std::optional<palu::PauseHandler> pauseHandler;

void SKSEMessageHandler(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case SKSE::MessagingInterface::kDataLoaded:
		pauseHandler.emplace();
		REL_MESSAGE("kDataLoaded Message - Pause available");
		break;

	case SKSE::MessagingInterface::kSaveGame:
		if (palu::SettingsCache::Instance().PauseOnSave())
		{
			REL_MESSAGE("Request Pause on kSaveGame message");
			if (pauseHandler.value().StartPause(true))
			{
				// no delay before progressing
				pauseHandler.value().ProgressPause();
			}
		}
		break;

	// to confirm timings wrt Loading Menu handling
	case SKSE::MessagingInterface::kPostLoad:
		DBG_MESSAGE("kPostLoad message");
		break;

	case SKSE::MessagingInterface::kPostPostLoad:
		DBG_MESSAGE("kPostPostLoad message");
		break;

	case SKSE::MessagingInterface::kPreLoadGame:
		DBG_MESSAGE("kPreLoadGame message");
		pauseHandler.value().SetIsLoading();
		break;

	case SKSE::MessagingInterface::kPostLoadGame:
		DBG_MESSAGE("kPostLoadGame message");
		break;

	case SKSE::MessagingInterface::kNewGame:
		DBG_MESSAGE("kNewGame message");
		break;

	default:
		break;
	}
}

#if _DEBUG
int MyCrtReportHook(int, char*, int*)
{
	__try {
		RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	__except (LogStackWalker::LogStack(GetExceptionInformation())) {
		REL_FATALERROR("PALU threw structured exception");
	}
	return 0;
}
#endif

void InitializeDiagnostics()
{
#if _DEBUG
	_CrtSetReportHook(MyCrtReportHook);
#endif
	// default log level is full (TRACE)
	spdlog::level::level_enum logLevel(spdlog::level::trace);
	char* levelValue;
	size_t requiredSize;
	if (getenv_s(&requiredSize, NULL, 0, LogLevelVariable.c_str()) == 0 && requiredSize > 0)
	{
		levelValue = (char*)malloc((requiredSize + 1) * sizeof(char));
		if (levelValue)
		{
			levelValue[requiredSize] = 0;	// ensure null-terminated
			// Get the value of the LIB environment variable.
			if (getenv_s(&requiredSize, levelValue, requiredSize, LogLevelVariable.c_str()) == 0)
			{
				try
				{
					int envLevel = std::stoi(levelValue);
					if (envLevel >= SPDLOG_LEVEL_TRACE && envLevel <= SPDLOG_LEVEL_OFF)
					{
						logLevel = (spdlog::level::level_enum)envLevel;
					}
				}
				catch (const std::exception&)
				{
				}
			}
		}
	}

	std::filesystem::path logPath(SKSE::log::log_directory().value());
	try
	{
		std::string fileName(logPath.generic_string());
		fileName.append("/");
		fileName.append(PALU_NAME);
		fileName.append(".log");
		PALULogger = spdlog::basic_logger_mt(LoggerName, fileName, true);
		PALULogger->set_pattern("%Y-%m-%d %T.%e %8l %6t %v");
	}
	catch (const spdlog::spdlog_ex&)
	{
	}
	spdlog::set_level(logLevel); // Set global log level
	spdlog::flush_on(logLevel);	// always flush
#if 0
#if _DEBUG
	SKSE::add_papyrus_sink();	// TODO what goes in here now
#endif
#endif

	REL_MESSAGE("{} v{}", PALU_NAME, VersionInfo::Instance().GetPluginVersionString().c_str());
}

EXTERN_C __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeDiagnostics();
	Hooks::Install();

	palu::SettingsCache::Instance().Refresh();

	REL_MESSAGE("{} plugin loaded", PALU_NAME);
	SKSE::Init(skse);
	SKSE::GetMessagingInterface()->RegisterListener(SKSEMessageHandler);

	return true;
}
