// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemolitionHUDWidget.generated.h"

class UTextBlock;

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetChargesData(int32 InChargesUsed, int32 InMaxCharges);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Charges;
};
