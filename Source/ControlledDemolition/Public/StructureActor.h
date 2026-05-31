// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JobTypes.h"
#include "GameFramework/Actor.h"
#include "StructureActor.generated.h"

class UStructureStabilityModel;

struct FStructureResult {
	int32 BrokenPiecesCount = 0;
	int32 UnprotectedPiecesCount = 0;
	bool bProtectedIntact = true;
};

class ABuildingPiece;

USTRUCT(BlueprintType)
struct FStructureRuntimeMetrics {
	GENERATED_BODY()
	
	// All members can be viewed in editor for testing
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	int32 PieceCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	int32 ConnectionCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	float LastBuildMs = 0.0f;
	
	// CONN/LOAD - PHYS reports this as 0
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	float LastSolveMs = 0.0f;
	
	// PHYS only
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics|Physics-only Baseline")
	int32 ConstraintCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics|Physics-only Baseline")
	int32 ConstraintBreaks = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics|Physics-only Baseline")
	float ConstraintSpawnMs = 0.f;
	
	// CONN only
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics|Connectivity model")
	int32 SupportedCount = 0;
	
	// LOAD only
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics|Load Propagation Model")
	int32 CascadeIterations = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics|Load Propagation Model")
	int32 OverloadFails = 0;
};

UCLASS()
class CONTROLLEDDEMOLITION_API AStructureActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStructureActor();
	
	void SetStabilityModel(UStructureStabilityModel* InModel) { StabilityModel = InModel; }
	UStructureStabilityModel* GetStabilityModelObj() const { return StabilityModel; }
	FName GetStabilityModelType() const;
	void AssignPieces(const TArray<ABuildingPiece*>& InPieces);
	
	void MarkConnectionsDirty() { bConnectionsDirty = true; }
	void MarkStabilityDirty() { bStabilityDirty = true; }
	void RefreshStructureState();
	
	FStructureResult EvaluateStructure() const;
	
	const TArray<TObjectPtr<ABuildingPiece>>& GetPieces() const { return Pieces; }
	const TMap<TObjectKey<ABuildingPiece>, TArray<TWeakObjectPtr<ABuildingPiece>>>& GetConnections() const { return Connections; }
	
	FString GetDebugName() const;
	const FStructureRuntimeMetrics& GetRuntimeMetrics() const { return RuntimeMetrics; }
	
	void SetRunningBenchmark(const bool Status) { bRunningBenchmark = Status; }
	void SetAutoRefresh(const bool Status) { bAutoRefreshOnBrokenPiece = Status; }
	
protected:		
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AddConnection(ABuildingPiece* PieceA, ABuildingPiece* PieceB);
	void FindConnectionsForPiece(ABuildingPiece* SourcePiece);
	void BuildConnections();
	
	void CheckJobConditions();
	bool HasFailedProtectedCondition() const;
	bool HasMetObjectiveCondition() const;
	
	void HandlePieceBroken(ABuildingPiece* BrokenPiece);
	
	void DrawConnectionDebug() const;
	void LogStructureMetrics() const;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Structure")
	TArray<TObjectPtr<ABuildingPiece>> Pieces;
	
	UPROPERTY(EditInstanceOnly, Instanced, BlueprintReadOnly, Category = "Structure|Stability")
	TObjectPtr<UStructureStabilityModel> StabilityModel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure|Detection")
	bool bAutoBuildConnects = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure|Detection", meta = (ClampMin = "0.0"))
	float ConnectionDistanceThreshold = 2.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure|Update")
	bool bAutoRefreshOnBrokenPiece = true;
	
	// Can inspect the cached values in editor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Structure|Metrics")
	FStructureRuntimeMetrics RuntimeMetrics;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Debug")
	FName DebugStructureName;
	
	// Adjacency list
	TMap<TObjectKey<ABuildingPiece>, TArray<TWeakObjectPtr<ABuildingPiece>>> Connections;
	
	bool bConnectionsDirty = false;
	bool bStabilityDirty = false;
	
	// doubles for accurate timing
	
	double LastConnectionBuildTimeMs = 0.0;
	double LastSupportSolveTimeMs = 0.0;
	
	int32 LastDetectedConnectionCount = 0;
	int32 LastSupportedPieceCount = 0;
	
	bool bRunningBenchmark = false;
	
	bool bConnectionDrawPending = false;
};
