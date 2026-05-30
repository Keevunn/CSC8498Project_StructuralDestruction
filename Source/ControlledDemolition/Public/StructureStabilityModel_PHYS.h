// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureStabilityModel.h"
#include "StructureStabilityModel_PHYS.generated.h"

USTRUCT()
struct FBaselineConstraintEdge {
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UPhysicsConstraintComponent> Constraint = nullptr;
	
	UPROPERTY()
	TWeakObjectPtr<ABuildingPiece> PieceA;
	
	UPROPERTY()
	TWeakObjectPtr<ABuildingPiece> PieceB;
	
	UPROPERTY()
	bool bBroken = false;
};


UCLASS(DisplayName = "PHYS: Chaos Constraint Baseline")
class CONTROLLEDDEMOLITION_API UStructureStabilityModel_PHYS : public UStructureStabilityModel
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) override;
	
	// Intentionally empty - no graph to resolve
	virtual void RefreshState() override { DrawConstraintDebug(); } 
	
	virtual bool HasMetObjectiveCondition() const override;
	virtual bool HasFailedProtectedCondition() const override;
	virtual bool OverridesJobConditions() const override {return true;}
	
	virtual FName GetModelType() const override { return TEXT("PHYS"); }
	
	virtual void WriteMetrics(FStructureRuntimeMetrics& OutMetrics) const override;
	
protected:
	void SpawnConstraintsFromAdjacency();
	void RegisterConstraintEdge(UPhysicsConstraintComponent* Constraint, ABuildingPiece* PieceA, ABuildingPiece* PieceB);
	
	void ConfigurePiecesForSetup();
	void EnablePhysicsOnPieces();
	
	void DrawConstraintDebug() const;
	
	UFUNCTION()
	void HandleConstraintBroken(int32 ConstraintIndex);
	
	UPROPERTY(EditAnywhere, Category = "Physics")
	float PieceMass = 50.f;
	
	TArray<FBaselineConstraintEdge> ConstraintEdges;
	
	const float SafetyFactor = 4.f;		// Scales the load ceiling, absorbs startup jitter
	const float LeverArmScale = 50.f;	// Torque scales with lever arm, distance from joint
	const float Gravity = 980.f;		// According to Unreal's default gravity value
	
	int32 ConstraintBreakCount = 0;
	float ConstraintSpawnMs = 0.f;
	
private:
	int32 CountAttachedConstraints(const ABuildingPiece* Piece, const bool bOnlyUnbroken) const;
};
