// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionResultWidget.h"

#include "DemolitionPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDemolitionResultWidget::SetResultData(bool bInWonLevel, int32 InFinalScore, int32 InMoneyEarned,
                                            float InDestructionRatio) {
	if (Text_ResultTitle)
		Text_ResultTitle->SetText(
			bInWonLevel 
			? FText::FromString(TEXT("Job Complete"))
			: FText::FromString(TEXT("Job Failed"))
		);
	
	if (Text_Score)
		Text_Score->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), InFinalScore)));
	
	if (Text_MoneyEarned)
		Text_MoneyEarned->SetText(FText::FromString(FString::Printf(TEXT("Money Earned: £%d"), InMoneyEarned)));
	
	if (Text_DestructionRatio) {
		const int32 DestructionPercent = FMath::RoundToInt(InDestructionRatio * 100.f);
		Text_DestructionRatio->SetText(FText::FromString(FString::Printf(TEXT("Destruction: %d%%"), DestructionPercent)));
	}
}

void UDemolitionResultWidget::NativeConstruct() {
	Super::NativeConstruct();
	
	if (Button_Continue)
		Button_Continue->OnClicked.AddDynamic(this, &UDemolitionResultWidget::HandleContinueClicked);
}

void UDemolitionResultWidget::HandleContinueClicked() {
	if (ADemolitionPlayerController* PC = GetOwningPlayer<ADemolitionPlayerController>())
		PC->ShowLevelSelect();
}
