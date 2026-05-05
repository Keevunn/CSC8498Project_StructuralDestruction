// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class AStructureActor;
class AChargeActor;
class UInputMappingContext;
class UCameraComponent;
class UInputAction;
class USpringArmComponent;

UCLASS()
class CONTROLLEDDEMOLITION_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	void SetGameplayLocked(const bool bLocked) { bGameplayLocked = bLocked; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void RequestRestart();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void PlaceCharge(const FInputActionValue& Value);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PlaceChargeAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RestartAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	TSubclassOf<AChargeActor> ChargeActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charges")
	float ChargeSpawnDistance = 200.0f;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Job")
	TObjectPtr<AStructureActor> ActiveStructure;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player State")
	bool bGameplayLocked = false;
};
