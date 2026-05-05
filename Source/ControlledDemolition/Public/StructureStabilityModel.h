// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StructureStabilityModel.generated.h"


class ABuildingPiece;
class AStructureActor;

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType)
class CONTROLLEDDEMOLITION_API UStructureStabilityModel : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialise(AStructureActor* InStructure) PURE_VIRTUAL(Initialise, );
	
	virtual void RefreshState() PURE_VIRTUAL(RefreshState, );
	virtual void OnPieceBroken(ABuildingPiece* BrokenPiece) PURE_VIRTUAL(OnPieceBroken, );
	
	virtual FName GetModelType() const PURE_VIRTUAL(GetModelType, return NAME_None;);
	
protected:
	UPROPERTY()
	TWeakObjectPtr<AStructureActor> OwningStructure;
};
