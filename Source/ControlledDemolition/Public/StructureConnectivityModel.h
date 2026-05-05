// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureStabilityModel.h"
#include "StructureConnectivityModel.generated.h"


UCLASS(DisplayName = "V1: Connectivity (BFS from anchors)")
class CONTROLLEDDEMOLITION_API UStructureConnectivityModel : public UStructureStabilityModel
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) override;
	
	virtual void RefreshState() override;
	virtual void OnPieceBroken(ABuildingPiece* BrokenPiece) override;
	
	virtual FName GetModelType() const override { return TEXT("V1_Connectivity"); }
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawSupportDebug = true;
	
protected:
	void ReconfigureSupport();
	void DetachUnsupportedPieces();
};
