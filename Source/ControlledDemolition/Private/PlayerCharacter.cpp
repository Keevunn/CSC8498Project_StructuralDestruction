// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "ChargeActor.h"
#include "DemolitionGameState.h"
#include "DemolitionPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "JobManagerSubsystem.h"
#include "StructureActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore); // Ignores building piece
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
	
	bUseControllerRotationYaw = true;
	
	GetMesh()->SetOwnerNoSee(true); // No mesh set up right now though...
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return;
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	if (DefaultMappingContext)
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	
	// TEMPORARY!!!
	if (!IsValid(ActiveStructure)) {
		TArray<AActor*> FoundStructures;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStructureActor::StaticClass(), FoundStructures);
		
		if (FoundStructures.Num() > 0.f) {
			for (AActor* Actor : FoundStructures) {
				AStructureActor* Structure = Cast<AStructureActor>(Actor);
				if (IsValid(Structure) && Structure->GetDebugName() == FString("SimpleTower")) {
					ActiveStructure = Structure;
					break;
				}
			}
		}
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput) return;
	
	if (MoveAction)
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	
	if (LookAction)
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	
	if (JumpAction) {
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	
	if (PlaceChargeAction)
		EnhancedInput->BindAction(PlaceChargeAction, ETriggerEvent::Started, this, &APlayerCharacter::PlaceCharge);
	
	if (RestartAction) {
		ADemolitionGameState* GS = GetWorld()->GetGameState<ADemolitionGameState>();
		EnhancedInput->BindAction(RestartAction, ETriggerEvent::Triggered, this, &APlayerCharacter::RequestRestart);
	}
}

void APlayerCharacter::RequestRestart() {
	if (const UGameInstance* GI = GetGameInstance())
		if (UJobManagerSubsystem* JobManager = GI->GetSubsystem<UJobManagerSubsystem>())
			JobManager->RestartCurrentJob();
}

void APlayerCharacter::Move(const FInputActionValue& Value) {
	if (!Controller || bGameplayLocked) return;
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRot(0, Rotation.Yaw, 0);
	
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(Forward, MovementVector.Y);
	AddMovementInput(Right, MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value) {
	if (!Controller || bGameplayLocked) return;
	
	const FVector2D LookAxis = Value.Get<FVector2D>();
	
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void APlayerCharacter::PlaceCharge(const FInputActionValue& Value) {
	if (!ChargeActorClass || bGameplayLocked) return;
	
	const FRotator Rotation = Controller->GetControlRotation();
	const FVector Forward = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	
	const FVector SpawnPos = GetActorLocation() + Forward * ChargeSpawnDistance;
	const FRotator SpawnRot = GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	AChargeActor* SpawnedCharge = World->SpawnActor<AChargeActor>(ChargeActorClass, SpawnPos, SpawnRot, SpawnParams);
	if (!SpawnedCharge) return;
	
	if (ADemolitionGameState* GameState = World->GetGameState<ADemolitionGameState>())
		GameState->RegisterChargePlaced();
	
}
