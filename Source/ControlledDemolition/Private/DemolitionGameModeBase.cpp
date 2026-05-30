// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionGameModeBase.h"

#include "DemolitionPlayerController.h"

ADemolitionGameModeBase::ADemolitionGameModeBase() {
	PlayerControllerClass = ADemolitionPlayerController::StaticClass();
}