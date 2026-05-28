// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureStabilityModel.h"
#include "StructureStabilityModel_CONN.generated.h"


UCLASS(DisplayName = "CONN: Connectivity Model")
class CONTROLLEDDEMOLITION_API UStructureStabilityModel_CONN : public UStructureStabilityModel
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) override;
	
	virtual void RefreshState() override;
	
	virtual FName GetModelType() const override { return TEXT("CONN"); }
	
protected:
	void DetachUnsupportedPieces();
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawSupportDebug = true;
};
