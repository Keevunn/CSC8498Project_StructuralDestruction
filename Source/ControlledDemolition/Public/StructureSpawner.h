// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructureSpawner.generated.h"


class AStructureActor;

UCLASS()
class CONTROLLEDDEMOLITION_API UStructureSpawner : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Benchmark|Spawner", meta=(WorldContextObject))
	static AStructureActor* SpawnSimpleTower(const UObject* WorldContextObject, FVector AnchorPos, int32 NumPieces, int32 Seed);
	
	UFUNCTION(BlueprintCallable, Category="Benchmark|Spawner", meta=(WorldContextObject))
	static AStructureActor* SpawnTwoSupportLoad(const UObject* WorldContextObject, FVector Origin, int32 Seed);
	
	UFUNCTION(BlueprintCallable, Category="Benchmark|Spawner", meta=(WorldContextObject))
	static AStructureActor* SpawnBridge(const UObject* WorldContextObject, FVector Origin, int32 NumSpanPieces, int32 Seed);
};
