// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JobTypes.h"
#include "GameFramework/GameStateBase.h"
#include "DemolitionGameState.generated.h"

class AStructureActor;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChargePlaced, int32 ChargesUsed, int32 MaxCharges);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnJobCompleted, bool bWonLevel, int32 FinalScore, int32 MoneyEarned, 
	float DestructionRatio);


class ABuildingPiece;

UCLASS()
class CONTROLLEDDEMOLITION_API ADemolitionGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	
	void RegisterStructure(AStructureActor* Structure);
	void UnregisterStructure(AStructureActor* Structure);
	
	void RegisterChargePlaced();
	
	void CompleteJob();
	void FailJob();
	
	int32 GetChargesUsed() const { return ChargesUsed; }
	int32 GetMaxCharges() const { return MaxCharges; }
	bool IsGameOver() const { return bGameOver; }
	
	FOnChargePlaced OnChargePlaced;
	FOnJobCompleted OnJobCompleted;
	
protected:
	void StartCurrentJob();
	
	FJobResult EvaluateStructures() const;
	
	float CalculateScore(float DestructionRatio, bool bProtectedIntact) const;
	int32 ConvertScoreToMoney(float Score) const;
	

	UPROPERTY(VisibleInstanceOnly, Category = "Game")
	TArray<TObjectPtr<AStructureActor>> ActiveStructures;
	
	UPROPERTY(VisibleAnywhere, Category = "Game")
	bool bGameOver = false;
	
	UPROPERTY(VisibleAnywhere, Category = "Job")
	FJobDefinition CurrentJob;
	
	UPROPERTY(VisibleAnywhere, Category = "Job")
	FJobResult LastJobResult;
	
	UPROPERTY(VisibleAnywhere, Category = "Job")
	int32 MaxCharges = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Job")
	int32 ChargesUsed = 0;
	
};
