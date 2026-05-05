// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameEconomySubsystem.generated.h"

UCLASS()
class CONTROLLEDDEMOLITION_API UGameEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void AddMoney(int32 Amount);
	int32 GetMoney() const;
	
protected:
	UPROPERTY()
	int32 CurrentMoney = 0;
	
};
