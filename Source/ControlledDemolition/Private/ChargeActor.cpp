// Fill out your copyright notice in the Description page of Project Settings.


#include "ChargeActor.h"

#include "BuildingPiece.h"
#include "StructureActor.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"

// Sets default values
AChargeActor::AChargeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot); // For safety, RootComponent may not exist at this point
	
	ChargeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChargeMesh"));
	ChargeMesh->SetupAttachment(SceneRoot);
	ChargeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ChargeMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ChargeMesh->SetSimulatePhysics(false);
	
}

// Called when the game starts or when spawned
void AChargeActor::BeginPlay()
{
	Super::BeginPlay();
	
	DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			ExplosionRadius, 8,
			FColor::Red,
			false, FuseTime, -1
		);
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	if (FuseTime > 0.f)
		World->GetTimerManager().SetTimer(
			FuseTimerHandle, 
			this,
			&AChargeActor::Explode,
			FuseTime,
			false
		);
	else 
		Explode();
	
	
}

void AChargeActor::Explode() {
	const FVector ExplosionOrigin = GetActorLocation();
	
	TArray<FOverlapResult> OverlapResults;
	TSet<TObjectKey<AStructureActor>> AffectedStructures;
	
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Might want to let the player take damage
	
	const bool bHasOverlaps = GetWorld()->OverlapMultiByObjectType(
		OverlapResults, 
		ExplosionOrigin,
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);
	
	if (bHasOverlaps) {
		// Avoid applying affects multiple times to same piece/component
		TSet<TObjectKey<ABuildingPiece>> DamagedPieces;
		TSet<TObjectKey<UPrimitiveComponent>> ImpulsedComponents;
		
		for (const FOverlapResult& Result : OverlapResults) {
			AActor* HitActor = Result.GetActor();
			UPrimitiveComponent* PrimitiveComponent = Result.GetComponent();
			ABuildingPiece* BuildingPiece = Cast<ABuildingPiece>(HitActor);
			AStructureActor* Structure = nullptr;
			bool bIsPHYS = false;
			
			if (IsValid(BuildingPiece)) {
				Structure = BuildingPiece->GetOwningStructure();
				
				if (IsValid(Structure)) {
					AffectedStructures.Add(TObjectKey<AStructureActor>(Structure));
					bIsPHYS = Structure->GetStabilityModelType() == FName("PHYS");
				}
			}
			
			// If valid building piece, apply damage first... 
			const bool bShouldApplyDamage = IsValid(BuildingPiece) && IsValid(Structure) && !bIsPHYS && !DamagedPieces.Contains(BuildingPiece);
			if (bShouldApplyDamage) { // Physics-only pieces shouldn't call BreakPiece
				DamagedPieces.Add(BuildingPiece);

				BuildingPiece->ApplyExplosionDamage(ExplosionOrigin, ExplosionRadius, ExplosionDamage);
			}
			//... then if broken will apply impulse
			
			// If physics object, apply impulse
			const bool bShouldApplyImpulse = PrimitiveComponent && PrimitiveComponent->IsSimulatingPhysics() &&
					!ImpulsedComponents.Contains(PrimitiveComponent) && 
					(!IsValid(BuildingPiece) || bIsPHYS || BuildingPiece->IsBroken());
			
			if (bShouldApplyImpulse) {
				ImpulsedComponents.Add(PrimitiveComponent);
				ApplyImpulseAtLocation(PrimitiveComponent, ExplosionOrigin);
			}
		}
		
		for (const TObjectKey<AStructureActor>& StructureKey : AffectedStructures)
			if (AStructureActor* Structure = StructureKey.ResolveObjectPtr()) Structure->ProcessDeferredUpdates();
	}
	
	Destroy();
}

void AChargeActor::ApplyImpulseAtLocation(UPrimitiveComponent* Component, const FVector& Origin) const {
	if (!IsValid(Component)) return;
	
	Component->AddRadialImpulse(
					Origin,
					ExplosionRadius,
					ExplosionImpulseStrength,
					RIF_Linear,
					true
				);
}

