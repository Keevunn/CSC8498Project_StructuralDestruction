// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEconomySubsystem.h"

void UGameEconomySubsystem::AddMoney(const int32 Amount) {
	CurrentMoney += Amount;
}

int32 UGameEconomySubsystem::GetMoney() const {
	return CurrentMoney;
}
