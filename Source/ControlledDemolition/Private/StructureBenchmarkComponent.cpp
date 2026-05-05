// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureBenchmarkComponent.h"

#include "ComponentUtils.h"
#include "StructureActor.h"

// Sets default values for this component's properties
UStructureBenchmarkComponent::UStructureBenchmarkComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UStructureBenchmarkComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bRunOnBeginPlay)
		RunBenchmarkIterations(DefaultIterationCount);
	
}

void UStructureBenchmarkComponent::RunBenchmark() {
	return;
}

void UStructureBenchmarkComponent::RunBenchmarkIterations(int32 Iterations) {
	AStructureActor* Structure = GetOwningStructure();
	if (!IsValid(Structure)) return;
	
	float TotalBuildMs = 0, TotalSolveMs = 0;
	for (int32 i = 0; i < Iterations; i++) {
		switch (BenchmarkMode) {
		case EStructureBenchmarkMode::FullRefresh:
			Structure->MarkConnectionsDirty();
			Structure->MarkStabilityDirty();
			break;
		case EStructureBenchmarkMode::SupportOnly:
			Structure->MarkStabilityDirty();
			break;
		}
		
		Structure->ProcessDeferredUpdates();
		
		const FStructureRuntimeMetrics& Runtime = Structure->GetRuntimeMetrics();
		
		TotalBuildMs += Runtime.LastBuildMs;
		TotalSolveMs += Runtime.LastSolveMs;
	}
	
	const FStructureRuntimeMetrics& FinalRuntime = Structure->GetRuntimeMetrics();
	
	FStructureBenchmarkMetrics Result;
	Result.Iterations = Iterations;
	Result.PieceCount = FinalRuntime.PieceCount;
	Result.ConnectionCount = FinalRuntime.ConnectionCount;
	Result.SupportedCount = FinalRuntime.SupportedCount;
	Result.AverageBuildMs = TotalBuildMs / Iterations;
	Result.AverageSolveMs = TotalSolveMs / Iterations;
	
	LogAverageResults(Result);
	LastBenchmarkResult = Result;
}

AStructureActor* UStructureBenchmarkComponent::GetOwningStructure() const {
	return Cast<AStructureActor>(GetOwner());
}

void UStructureBenchmarkComponent::LogSingleRunResults(const AStructureActor* Structure) const {
	return;
}

void UStructureBenchmarkComponent::LogAverageResults(const FStructureBenchmarkMetrics& Metrics) const {
	AStructureActor* Structure = GetOwningStructure();
	
	const FString StructureName = IsValid(Structure) ? Structure->GetDebugName() : TEXT("UnknownStructure");
	
	UE_LOG(
		LogTemp,
		Log,
		TEXT("BENCHMARK_AVG | Structure: %s | Iterations: %d | Pieces: %d | Connections: %d | Supported: %d | AvgBuild: %.3f ms | AvgSolve: %.3f ms"),
		*StructureName, 
		Metrics.Iterations,
		Metrics.PieceCount,
		Metrics.ConnectionCount,
		Metrics.SupportedCount,
		Metrics.AverageBuildMs,
		Metrics.AverageSolveMs
	);
	
	if (bLogCsvStyleOutput)
		UE_LOG(
			LogTemp,
			Log,
			TEXT("CSV_AVG,%s,%d,%d,%d,%d,%.3f,%.3f"),
			*StructureName, 
			Metrics.Iterations,
			Metrics.PieceCount,
			Metrics.ConnectionCount,
			Metrics.SupportedCount,
			Metrics.AverageBuildMs,
			Metrics.AverageSolveMs
		);
}


