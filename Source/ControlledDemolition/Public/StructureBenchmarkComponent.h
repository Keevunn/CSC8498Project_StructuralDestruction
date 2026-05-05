// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructureBenchmarkComponent.generated.h"

class AStructureActor;

USTRUCT(BlueprintType)
struct FStructureBenchmarkMetrics {
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 Iterations = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 PieceCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 ConnectionCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	int32 SupportedCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float AverageBuildMs = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	float AverageSolveMs = 0.0f;
};

UENUM(BlueprintType)
enum class EStructureBenchmarkMode : uint8 {
	FullRefresh UMETA(DisplayName = "Full Refresh"),
	SupportOnly UMETA(DisplayName = "Support only")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTROLLEDDEMOLITION_API UStructureBenchmarkComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStructureBenchmarkComponent();
	
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "Benchmark")
	void RunBenchmark();
	
	UFUNCTION(BlueprintCallable, Category = "Benchmark")
	void RunBenchmarkIterations(int32 Iterations);

protected:
	AStructureActor* GetOwningStructure() const;
	
	void LogSingleRunResults(const AStructureActor* Structure) const;
	void LogAverageResults(const FStructureBenchmarkMetrics& Metrics) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	EStructureBenchmarkMode BenchmarkMode = EStructureBenchmarkMode::FullRefresh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bRunOnBeginPlay = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark", meta = (ClampMin = "1"))
	int32 DefaultIterationCount = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Benchmark")
	bool bLogCsvStyleOutput = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Benchmark")
	FStructureBenchmarkMetrics LastBenchmarkResult;
};
