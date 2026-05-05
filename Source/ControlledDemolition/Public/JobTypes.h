#pragma once

#include "CoreMinimal.h"
#include"JobTypes.generated.h"

USTRUCT(BlueprintType)
struct FJobDefinition {
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName JobName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName LevelName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxCharges = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetDestructionRatio = 0.8f;
	
	bool operator==(const FJobDefinition& Other) const {
		if (Other.JobName == this->JobName)
			return true;
		return false;
	}
};

USTRUCT(BlueprintType)
struct FJobResult {
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DestructionRatio = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bProtectedIntact = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ChargesUsed = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FinalScore = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MoneyEarned = 0;
};
