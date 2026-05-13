// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureStabilityModel.h"
#include "StructureConnectivityModel.generated.h"


UCLASS(DisplayName = "CONN: Connectivity Model")
class CONTROLLEDDEMOLITION_API UStructureConnectivityModel : public UStructureStabilityModel
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) override;
	
	virtual void RefreshState() override;
	virtual void OnPieceBroken(ABuildingPiece* BrokenPiece) override {}
	
	virtual FName GetModelType() const override { return TEXT("CONN"); }
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawSupportDebug = true;
	
protected:
	void ReconfigureSupport();
	void DetachUnsupportedPieces();
};
