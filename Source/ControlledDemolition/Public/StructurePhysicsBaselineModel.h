// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureStabilityModel.h"
#include "StructurePhysicsBaselineModel.generated.h"

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


UCLASS(DisplayName = "B: Physics Baseline (constraints only)")
class CONTROLLEDDEMOLITION_API UStructurePhysicsBaselineModel : public UStructureStabilityModel
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) override;
	
	// Intentionally empty - no graph to resolve
	virtual void RefreshState() override {} 
	
	// Intentionally empty - breaks handled by constraints 
	virtual void OnPieceBroken(ABuildingPiece* BrokenPiece) override {}
	
	virtual FName GetModelType() const override { return TEXT("B_Physics"); }
	
	UPROPERTY(EditAnywhere, Category = "Physics")
	float PieceMass = 50.f;
	
protected:
	void SpawnConstraintsFromAdjacency();
	void RegisterConstraintEdge(UPhysicsConstraintComponent* Constraint, ABuildingPiece* PieceA, ABuildingPiece* PieceB);
	
	void ConfigurePiecesForSetup();
	void EnablePhysicsOnAllPieces();
	
	UFUNCTION()
	void HandleConstraintBroken(int32 ConstraintIndex);
	
	TArray<FBaselineConstraintEdge> ConstraintEdges;
	
	TMap<TObjectKey<ABuildingPiece>, FVector> InitialPieceLocations;
	
	const float SafetyFactor = 4.f;	// Scales the load ceiling, absorbs startup jitter
	const float LeverArmScale = 50.f; // Torque scales with lever arm, distance from joint
	const float Gravity = 980.f;	// According to Unreal's default gravity value
};
