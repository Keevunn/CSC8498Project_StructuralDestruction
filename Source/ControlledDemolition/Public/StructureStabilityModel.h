// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StructureStabilityModel.generated.h"


struct FStructureRuntimeMetrics;
class ABuildingPiece;
class AStructureActor;

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType)
class CONTROLLEDDEMOLITION_API UStructureStabilityModel : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) PURE_VIRTUAL(Initialise, );
	
	virtual void RefreshState() PURE_VIRTUAL(RefreshState, );
	
	virtual bool HasMetObjectiveCondition() const { return false; }
	virtual bool HasFailedProtectedCondition() const { return false; }
	virtual bool OverridesJobConditions() const { return false; }
	
	virtual FName GetModelType() const PURE_VIRTUAL(GetModelType, return NAME_None;);
	
	virtual void WriteMetrics(FStructureRuntimeMetrics& OutMetrics) const {}
	
protected:
	UPROPERTY()
	TWeakObjectPtr<AStructureActor> OwningStructure;
};
