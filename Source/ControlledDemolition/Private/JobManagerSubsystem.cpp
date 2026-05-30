// Fill out your copyright notice in the Description page of Project Settings.


#include "JobManagerSubsystem.h"

#include "Kismet/GameplayStatics.h"

void UJobManagerSubsystem::StartJob(const FJobDefinition& Job) {
	
	CurrentJob = Job;
	
	if (const UWorld* World = GetWorld())
		UGameplayStatics::OpenLevel(World, Job.LevelName);
}

void UJobManagerSubsystem::RestartCurrentJob() {
	if (UWorld* World = GetWorld())
		UGameplayStatics::OpenLevel(World, CurrentJob.LevelName);
}


