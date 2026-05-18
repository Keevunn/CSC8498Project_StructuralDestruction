// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChargeActor.generated.h"

class USphereComponent;

UCLASS()
class CONTROLLEDDEMOLITION_API AChargeActor : public AActor {
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChargeActor();
	
	void SetFuseTime(const float InFuseTime)				{ FuseTime = InFuseTime; }
	void SetExplosionRadius(const float Radius)				{ ExplosionRadius = Radius; }
	void SetExplosionImpulseStrength(const float Strength)	{ ExplosionImpulseStrength = Strength; }
	void SetExplosionDamage(const float Damage)				{ ExplosionDamage = Damage; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION() // Allows for blueprint visibility if necessary
	void Explode();
	
	void ApplyImpulseAtLocation(UPrimitiveComponent* Component, const FVector& Origin) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ChargeMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge Information")
	float FuseTime = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge Information")
	float ExplosionRadius = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge Information")
	float ExplosionImpulseStrength = 1500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge Information")
	float ExplosionDamage = 120.0f;

	FTimerHandle FuseTimerHandle;
};
