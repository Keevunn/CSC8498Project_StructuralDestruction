// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureStabilityModel.h"
#include "StructureStabilityModel_LOAD.generated.h"


UCLASS(DisplayName = "LOAD: Load Propagation Model")
class CONTROLLEDDEMOLITION_API UStructureStabilityModel_LOAD : public UStructureStabilityModel
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) override {}
	
	virtual void RefreshState() override {}
	virtual void OnPieceBroken(ABuildingPiece* BrokenPiece) override {}
	
	virtual FName GetModelType() const override { return TEXT("LOAD"); }
	
	
};
