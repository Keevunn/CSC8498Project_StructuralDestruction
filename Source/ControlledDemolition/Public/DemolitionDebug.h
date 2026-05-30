#pragma once

#include "HAL/IConsoleManager.h"

namespace DemolitionDebug {
	inline bool DrawDebugEnabled() {
		static IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("demolition.drawDebug"));
		return CVar && CVar->GetInt() > 0;
	}

	inline bool VerboseLogsEnabled() {
		static IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("demolition.verboseLogs"));
		return CVar && CVar->GetInt() > 0;
	}

	void ToggleDrawDebug();
	void ToggleVerboseLogs();
}