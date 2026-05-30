// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DemolitionGameModeBase.h"
#include "DemolitionMainMenuGameMode.generated.h"

UCLASS()
class CONTROLLEDDEMOLITION_API ADemolitionMainMenuGameMode : public ADemolitionGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};
