#pragma once

#include "CoreMinimal.h"
#include "Misc/CoreDelegates.h"
#include "Misc/App.h"

struct FFrameTimeSampler {
	FFrameTimeSampler() {
		Handle = FCoreDelegates::OnEndFrame.AddRaw(this, &FFrameTimeSampler::OnFrame);
	}
	
	void Stop() {
		if (Handle.IsValid()) {
			FCoreDelegates::OnEndFrame.Remove(Handle);
			Handle.Reset();
		}
	}
	
	float GetPeakMs() const { return PeakMs; }
	float GetAverageMs() const { return Frames > 0 ? TotalMs / Frames : 0.f; }
	int32 GetFramesObserved() const { return Frames; }

private:
	void OnFrame() {
		const float dtMs = FApp::GetDeltaTime() * 1000.f;
		TotalMs += dtMs;
		PeakMs = FMath::Max(PeakMs, dtMs);
		++Frames;
	}
	
	FDelegateHandle Handle;
	float TotalMs = 0.f;
	float PeakMs = 0.f;
	int32 Frames = 0;
};