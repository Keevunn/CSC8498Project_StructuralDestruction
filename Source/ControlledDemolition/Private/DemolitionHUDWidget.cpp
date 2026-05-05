// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionHUDWidget.h"

#include "Components/TextBlock.h"

void UDemolitionHUDWidget::SetChargesData(const int32 InChargesUsed, const int32 InMaxCharges) {
	if (!Text_Charges) return;
	
	const FString HUDText = FString::Printf(
		TEXT("Charges: %d / %d"),
		InChargesUsed,
		InMaxCharges
	);
	
	Text_Charges->SetText(FText::FromString(HUDText));
}
