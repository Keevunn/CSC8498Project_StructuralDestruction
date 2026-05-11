// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingPiece.h"

#include "DemolitionGameState.h"
#include "StructureActor.h"

// Sets default values
ABuildingPiece::ABuildingPiece()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PieceMesh"));
	PieceMesh->SetupAttachment(SceneRoot);
	PieceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PieceMesh->SetCollisionObjectType(ECC_GameTraceChannel1); // Building Piece Channel
	PieceMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PieceMesh->SetSimulatePhysics(false);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
	TEXT("/Game/Meshes/SM_BuildingPiece_Box"));
	if (CubeMeshAsset.Succeeded())
		PieceMesh->SetStaticMesh(CubeMeshAsset.Object);
	
	
}

// Called when the game starts or when spawned
void ABuildingPiece::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	ApplyRoleDefaults();
	
	if (PieceMesh) {
		// reliably set collision channel
		PieceMesh->SetCollisionObjectType(ECC_GameTraceChannel1); // Building Piece Channel
		PieceMesh->SetCollisionResponseToAllChannels(ECR_Block);
		DynamicMaterial = PieceMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	
	
	if (DynamicMaterial) 
		DynamicMaterial->SetVectorParameterValue(
			TEXT("BaseColour"),
			GetRoleDebugColour()
		);
	
}

void ABuildingPiece::ApplyRoleDefaults() {
	bAnchored = PieceRole == EPieceRole::Anchor;
	bShouldProtect = PieceRole == EPieceRole::Protected;
	bIsObjective = PieceRole == EPieceRole::Objective;
}

void ABuildingPiece::ApplyExplosionDamage(const FVector& ExplosionOrigin, float ExplosionRadius, float MaxDamage) {
	if (bBroken) return;
	if (!PieceMesh) return;
	
	FVector ClosestPoint;
	const float HasClosestPoint = PieceMesh->GetClosestPointOnCollision(ExplosionOrigin, ClosestPoint);
	const FVector DamagePoint = HasClosestPoint > 0.f ? ClosestPoint : PieceMesh->GetComponentLocation();
	const float Distance = FVector::Distance(DamagePoint, ExplosionOrigin);
	
	const float Alpha = FMath::Clamp(Distance / ExplosionRadius, 0.f, 1.f);
	const float IncomingDmg = FMath::Lerp(MaxDamage, 0.f, Alpha);
	
	CurrentHealth -= IncomingDmg;
	if (CurrentHealth <= 0.f)
		BreakPiece();
	
	if (DynamicMaterial) {
		const float DamageRatio = MaxHealth > 0.f ? 1.f - CurrentHealth / MaxHealth : 1.0f;
		DynamicMaterial->SetScalarParameterValue(TEXT("DamageAmount"), DamageRatio);
	}
	
	if (GEngine) {
		const FString DebugMessage = FString::Printf(
			TEXT("BuildingPiece '%s' | Damage: -%.1f | Health: %.1f"),
			*GetDebugName(),
			IncomingDmg,
			CurrentHealth
		);
		
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, DebugMessage);
	}
}

void ABuildingPiece::ForceBreakPiece() {
	if (bBroken) return;
	BreakPiece();
}

void ABuildingPiece::MarkBrokenFromPhysics() {
	if (bBroken) return;
	
	bBroken = true;
	bSupported = false;
	
	OnPieceBroken.Broadcast(this);
	
	if (GEngine) 
		UE_LOG(LogTemp, Log, TEXT("BuildingPiece '%s' | BROKE (from physics)"), *GetDebugName());
}

FVector ABuildingPiece::GetPieceCentreLocation() const {
	if (!IsValid(PieceMesh)) return FVector::ZeroVector;
	
	const FBox Bounds = PieceMesh->Bounds.GetBox();
	return Bounds.GetCenter();
}

void ABuildingPiece::SetOwningStructure(AStructureActor* InStructure) { OwningStructure = InStructure; }

FString ABuildingPiece::GetDebugName() const {
	if (!DebugPieceName.IsNone()) return DebugPieceName.ToString();
	
#if WITH_EDITOR
	return GetActorLabel();
#else 
	return GetName();
#endif
}

void ABuildingPiece::BreakPiece() {
	if (bBroken) return;  
	bBroken = true;
	bSupported = false;
	
	if (PieceMesh)
		PieceMesh->SetSimulatePhysics(true);
	
	OnPieceBroken.Broadcast(this);
	
	if (GEngine) 
		UE_LOG(LogTemp, Log, TEXT("BuildingPiece '%s' | BROKE"), *GetDebugName());
}

FLinearColor ABuildingPiece::GetRoleDebugColour() const {
	switch (PieceRole) {
	case EPieceRole::Anchor:
		return FLinearColor(0.05f, 0.05f, 0.05f);
	case EPieceRole::Support:
		return FLinearColor(1.0f, 0.7f, 0.1f);
	case EPieceRole::Load:
		return FLinearColor(0.5f, 0.5f, 0.5f);
	case EPieceRole::Objective:
		return FLinearColor(0.1f, 1.0f, 0.1f);
	case EPieceRole::Protected:
		return FLinearColor(0.1f, 0.3f, 1.0f);
	case EPieceRole::Decorative:
	default:
		return FLinearColor::White;
	}
}
