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
	virtual void Initialise(AStructureActor* InStructure) override;
	
	virtual void RefreshState() override;
	
	virtual FName GetModelType() const override { return TEXT("LOAD"); }
	
private:
	void BuildLayers(TMap<TObjectKey<ABuildingPiece>, int32>& OutLayers) const;
	void AccumulateLoad(const TMap<TObjectKey<ABuildingPiece>, int32>& Layers, TMap<TObjectKey<ABuildingPiece>, float>& OutAccumLoad) const;
	
	void DrawLoadDebug() const;
	
	UPROPERTY(EditAnywhere, Category = "Load Propagation", meta = (ClampMin = "1"))
	int32 MaxCascadeIterations = 10;
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawLoadInfoDebug = true;
	
	bool bRefreshActive = false;
};
