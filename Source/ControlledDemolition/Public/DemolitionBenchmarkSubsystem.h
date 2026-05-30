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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBenchmarkSweepsComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBenchmarkSweepStarted, int32, SweepNumber, int32, TotalSweeps, EBenchmarkScenario, Scenario);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBenchmarkRunStarted, int32, RunNumber, int32, TotalRuns);

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionBenchmarkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="Benchmark")
	FBox GetCurrentStructureBounds() const;
	
	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void SetPendingSingleSweep(EBenchmarkScenario Scenario, EBenchmarkProfile Profile);
	
	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void SetPendingAllSweeps(EBenchmarkProfile Profile);
	
	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void RunPendingSweeps();
	
	UPROPERTY(BlueprintAssignable, Category="Benchmark")
	FOnBenchmarkSweepsComplete OnAllSweepsComplete;
	
	UPROPERTY(BlueprintAssignable, Category="Benchmark")
	FOnBenchmarkSweepStarted OnSweepStarted;
	
	UPROPERTY(BlueprintAssignable, Category="Benchmark")
	FOnBenchmarkRunStarted OnRunStarted;
	
private:
	// Run loop ---------------------------------
	int32 EnqueueSweep_AnchorRemoval(const TArray<int32>& Counts, int32 Repeats);
	int32 EnqueueSweep_SupportRemoval(const TArray<int32>& Counts, int32 Repeats);
	int32 EnqueueSweep_LoadRedistribution(int32 Repeats);
	int32 EnqueueSweep_ProtectedPreservation(const TArray<int32>& Widths, int32 Repeats);
	void BeginRun();
	void EndRun();
	void StartNextRun();
	
	// Scenario set-up --------------------------
	AStructureActor* SpawnStructureForScenario(UWorld* World, const FBenchmarkRunSpec& Spec);
	
	// Scenario trigger functions ---------------
	
	// Deterministically breaks a piece matching the given role
	float BreakPieceByRole(AStructureActor* Structure, EPieceRole Role, const FRandomStream& Rng);
	
	// Applies explosion centered on piece matching given role
	float ExplodeAtRole(AStructureActor* Structure, EPieceRole Role, const FRandomStream& Rng);
	
	ABuildingPiece* PickByRole(const AStructureActor* Structure, const EPieceRole Role, const FRandomStream& Rng);
	
	// CSV --------------------------------------
	void EnsureFileOpen();
	void WriteRow(const FBenchmarkRow& Row);
	
	// Pass/Fail dispatch -----------------------
	void EvaluatePassFail(FBenchmarkRow& OutRow, const FBenchmarkRunSpec& Spec) const;
	
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
	
	// UI ---------------------------------------
	TOptional<EBenchmarkScenario> PendingScenario;
	EBenchmarkProfile PendingProfile = EBenchmarkProfile::Demo;
	
	int32 LastScenario = INDEX_NONE; // spec.scenario as int
	int32 CurrentSweepIndex = 0; 
	int32 TotalSweeps = 0; 
	int32 CurrentRun = INDEX_NONE;
	TArray<int32> TotalRunsPerSweep;
	
};
