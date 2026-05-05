// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JobTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JobManagerSubsystem.generated.h"

UCLASS()
class CONTROLLEDDEMOLITION_API UJobManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void StartJob(const FJobDefinition& Job);
	void RestartCurrentJob();
	
	const FJobDefinition& GetCurrentJob() const {return CurrentJob;}
	
protected:
	UPROPERTY()
	FJobDefinition CurrentJob;
};
