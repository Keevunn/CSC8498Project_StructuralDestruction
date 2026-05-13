// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingPiece.generated.h"

class ABuildingPiece;
class AStructureActor;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBuildingPieceBroken, ABuildingPiece*);

UENUM(BlueprintType)
enum class EPieceRole : uint8 {
	Anchor UMETA(DisplayName = "Anchor"),
	Support UMETA(DisplayName = "Support"),
	Load UMETA(DisplayName = "Load"),
	Objective UMETA(DisplayName = "Objective"),
	Protected UMETA(DisplayName = "Protected"),
};

UCLASS()
class CONTROLLEDDEMOLITION_API ABuildingPiece : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildingPiece();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	EPieceRole GetPieceRole() const				{ return PieceRole;}
	void SetPieceRole(const EPieceRole InRole)	{ PieceRole = InRole; }
	void SetLoad(const float InLoad)			{ Load = InLoad; }
	void SetCapacity(const float InCapacity)	{ Capacity = InCapacity;}
	
	void ApplyRoleDefaults();
	
	void ApplyExplosionDamage(const FVector& ExplosionOrigin, float ExplosionRadius, float MaxDamage);
	
	FOnBuildingPieceBroken OnPieceBroken;
	
	void RecordBreak();
	void RecordPhysicsBreak();
	
	void SetIsObjective(const bool bInIsObjective) { bIsObjective = bInIsObjective; }
	void SetShouldProtect(const bool bInShouldProtect) { bShouldProtect = bInShouldProtect; }
	
	bool IsObjective() const		{ return bIsObjective; }
	bool ShouldProtect() const	{ return bShouldProtect; } // TODO Is it faster to keep this or look up role (one source of truth?)
	
	void SetSupported(const bool bInSupported) { bSupported = bInSupported; }
	
	bool IsAnchored()	const { return bAnchored; }
	bool IsSupported()	const { return bSupported; }
	bool IsBroken()		const { return bBroken; }
	
	FVector GetPieceCentreLocation() const;
	UStaticMeshComponent* GetPieceMesh() const { return PieceMesh; }
	
	void SetOwningStructure(AStructureActor* InStructure);
	AStructureActor* GetOwningStructure() const { return OwningStructure.Get(); }
	
	FString GetDebugName() const;

protected:
	void BreakPiece();
	
	FLinearColor GetRoleDebugColour() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PieceMesh;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Building Piece")
	EPieceRole PieceRole = EPieceRole::Support;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building Piece|Material Properties")
	float MaxHealth = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece|Material Properties")
	float CurrentHealth = 100.0f;
	
	// Load and capacity only used in LOAD model
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building Piece|Material Properties|Load Propagation Model")
	float Load = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building Piece|Material Properties|Load Propagation Model")
	float Capacity = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece|Stability Properties")
	bool bAnchored = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece|Stability Properties")
	bool bSupported = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece|Stability Properties")
	bool bBroken = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece|Stability Properties")
	bool bIsObjective = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece|Stability Properties")
	bool bShouldProtect = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Debug")
	FName DebugPieceName;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
	
	TWeakObjectPtr<AStructureActor> OwningStructure;
};
