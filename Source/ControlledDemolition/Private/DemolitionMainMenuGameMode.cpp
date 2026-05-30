// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionMainMenuGameMode.h"

#include "DemolitionPlayerController.h"

void ADemolitionMainMenuGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	if (auto* PC = Cast<ADemolitionPlayerController>(NewPlayer))
		PC->ShowMainMenu();
}
