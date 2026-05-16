// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BenchmarkTypes.h"
#include "FrameTimeSampler.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DemolitionBenchmarkSubsystem.generated.h"


struct FStructureResult;
class ABuildingPiece;
enum class EPieceRole : uint8;
class AStructureActor;

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionBenchmarkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// Entry point - called from console, can be called from blueprint
	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void RunSweep(int32 Sweep);
	
private:
	// Run loop ---------------------------------
	void EnqueueSweep_AnchorRemoval();
	void EnqueueSweep_SupportRemoval();
	void EnqueueSweep_LoadRedistribution();
	void BeginRun();
	void EndRun();
	void StartNextRun();
	
	// Scenario set-up --------------------------
	AStructureActor* SpawnStructureForScenario(UWorld* World, const FBenchmarkRunSpec& Spec);
	
	// Scenario trigger functions ---------------
	
	// Deterministically breaks a piece matching the given role
	float BreakPieceByRole(AStructureActor* Structure, EPieceRole Role, const FRandomStream& Rng);
	
	// Applies explosion centered on piece matching given role
	float ExplodeAtRole(AStructureActor* Structure, EPieceRole Role, const float Radius, const float MaxDamage, const FRandomStream& Rng);
	
	ABuildingPiece* PickByRole(const AStructureActor* Structure, const EPieceRole Role, const FRandomStream& Rng);
	
	// CSV --------------------------------------
	void EnsureFileOpen();
	void WriteRow(const FBenchmarkRow& Row);
	
	// Pass/Fail dispatch -----------------------
	void EvaluatePassFail(FBenchmarkRow& OutRow, const FBenchmarkRunSpec& Spec, const FStructureResult& Result) const;
	
	// Console command handler ------------------
	void HandleConsoleCommand(const TArray<FString>& Args);
	IConsoleCommand* RegisteredCommand = nullptr;
	
	// Per-run state ----------------------------
	TQueue<FBenchmarkRunSpec> Queue;
	TOptional<FBenchmarkRunSpec> CurrentSpec;
	FBenchmarkRow CurrentRow;
	TWeakObjectPtr<AStructureActor> CurrentStructure;
	TUniquePtr<FFrameTimeSampler> Sampler;
	FRandomStream CurrentRng;
	int32 TriggeredPieceIndex = INDEX_NONE;
	
	FTimerHandle WaitHandle;
	
	// Output -----------------------------------
	TUniquePtr<IFileHandle> CSVHandle; 
	float ObservationSeconds = 3.f;
};
