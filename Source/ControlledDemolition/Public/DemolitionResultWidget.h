// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemolitionResultWidget.generated.h"

class UButton;
class UTextBlock;
DECLARE_MULTICAST_DELEGATE(FOnResultRestartRequested);
DECLARE_MULTICAST_DELEGATE(FOnResultMenuRequested);

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetResultData(bool bInWonLevel, int32 InFinalScore, int32 InMoneyEarned, float InDestructionRatio);
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void HandleContinueClicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ResultTitle;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Score;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_MoneyEarned;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_DestructionRatio;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Continue;
};
