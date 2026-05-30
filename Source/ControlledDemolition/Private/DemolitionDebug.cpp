#include "DemolitionDebug.h"

#include "Engine/Engine.h"

namespace {
	TAutoConsoleVariable<int32> CVarDrawDebug(
		TEXT("demolition.drawDebug"),
		0,
		TEXT("Toggle demolition debug visuals."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarVerboseLogs(
		TEXT("demolition.verboseLogs"),
		0,
		TEXT("Toggle verbose demolition logs."),
		ECVF_Default);

	void ShowToggleMessage(int32 Key, const TCHAR* Label, bool bOn) {
		if (!GEngine) return;
		const FColor Colour = bOn ? FColor::Green : FColor::Red;
		const FString Message = FString::Printf(TEXT("%s: %s"), Label, bOn ? TEXT("ON") : TEXT("OFF"));
		GEngine->AddOnScreenDebugMessage(Key, 2.f, Colour, Message);
	}
}

void DemolitionDebug::ToggleDrawDebug() {
	const int32 NewValue = CVarDrawDebug.GetValueOnGameThread() ? 0 : 1;
	CVarDrawDebug->Set(NewValue);
	ShowToggleMessage(1, TEXT("Draw Debug"), NewValue > 0);
}

void DemolitionDebug::ToggleVerboseLogs() {
	const int32 NewValue = CVarVerboseLogs.GetValueOnGameThread() ? 0 : 1;
	CVarVerboseLogs->Set(NewValue);
	ShowToggleMessage(2, TEXT("Verbose Logs"), NewValue > 0);
}