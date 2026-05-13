// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureActor.h"

#include "BuildingPiece.h"
#include "DemolitionGameState.h"
#include "StructureStabilityModel.h"
#include "Engine/OverlapResult.h"

// Sets default values
AStructureActor::AStructureActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

FName AStructureActor::GetStabilityModelType() const {
	return StabilityModel ? StabilityModel->GetModelType() : NAME_None;
}

void AStructureActor::AssignPieces(const TArray<ABuildingPiece*>& InPieces) {
	Pieces.Reset(InPieces.Num());
	for (ABuildingPiece* P : InPieces) Pieces.Add(P); // Structure Actor owns all pieces
}

void AStructureActor::ProcessDeferredUpdates() {
	RefreshStructureState();
}

FStructureResult AStructureActor::EvaluateStructure() const {
	FStructureResult Result;
	
	bool bProtectedIntact = true;
	int32 BrokenPiecesCount = 0;
	int32 UnprotectedPiecesCount = 0;
	for (const ABuildingPiece* Piece : Pieces) {
		if (!IsValid(Piece)) continue;
		
		if (Piece->ShouldProtect()) {
			if (Piece->IsBroken())
				bProtectedIntact = false;
			continue;
		}
		
		UnprotectedPiecesCount++;
		
		if (Piece->IsBroken())
			BrokenPiecesCount++;
	}
	
	Result.BrokenPiecesCount = BrokenPiecesCount;
	Result.UnprotectedPiecesCount = UnprotectedPiecesCount;
	Result.bProtectedIntact = bProtectedIntact;
	
	return Result;
}

void AStructureActor::RecordMetrics(const float InElapsedMs) {
	RuntimeMetrics.LastSolveMs = InElapsedMs;
	
	int32 SupportedPieceCount = 0;
	for (const ABuildingPiece* Piece : Pieces) 
		if (IsValid(Piece) && Piece->IsSupported()) SupportedPieceCount++;
	
	RuntimeMetrics.SupportedCount = SupportedPieceCount;
	RuntimeMetrics.PieceCount = Pieces.Num();
}

FString AStructureActor::GetDebugName() const {
	if (!DebugStructureName.IsNone()) return DebugStructureName.ToString();
	
#if WITH_EDITOR
	return GetActorLabel();
#else 
	return GetName();
#endif
}

// Called when the game starts or when spawned
void AStructureActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (ADemolitionGameState* GameState = GetWorld()->GetGameState<ADemolitionGameState>())
		GameState->RegisterStructure(this);
	
	if (Pieces.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("StructureActor '%s' | No assigned pieces"), *GetDebugName()); 
		return;
	}
	
	for (ABuildingPiece* Piece : Pieces) {
		if (!IsValid(Piece)) continue;
		
		Piece->ApplyRoleDefaults();
		Piece->SetOwningStructure(this);
		Piece->OnPieceBroken.AddUObject(this, &AStructureActor::HandlePieceBroken);
	}
	
	BuildConnections();
	MarkStabilityDirty();
	
	if (StabilityModel) 
		StabilityModel->Initialise(this);
	else 
		UE_LOG(LogTemp, Warning, TEXT("StructureActor '%s' | No StabilityModel assigned"), *GetDebugName());
	
	RefreshStructureState();
	RecordMetrics(0.f); // Fills RuntimeMetrics with initial data
}

void AStructureActor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	for (ABuildingPiece* Piece : Pieces)
		if (IsValid(Piece)) {
			Piece->OnPieceBroken.RemoveAll(this);
			Piece->Destroy();
		}
	Pieces.Empty();
	
	if (ADemolitionGameState* GameState = GetWorld()->GetGameState<ADemolitionGameState>())
		GameState->UnregisterStructure(this);
	
	Super::EndPlay(EndPlayReason);
}

void AStructureActor::AddConnection(ABuildingPiece* PieceA, ABuildingPiece* PieceB) {
	if (!IsValid(PieceA) || !IsValid(PieceB) || PieceA == PieceB) return;
	
	const TObjectKey<ABuildingPiece> KeyA(PieceA);
	const TObjectKey<ABuildingPiece> KeyB(PieceB);
	
	TArray<TWeakObjectPtr<ABuildingPiece>>& NeighboursA = Connections.FindOrAdd(KeyA);
	TArray<TWeakObjectPtr<ABuildingPiece>>& NeighboursB = Connections.FindOrAdd(KeyB);
	
	NeighboursA.AddUnique(PieceB);
	NeighboursB.AddUnique(PieceA);
}

void AStructureActor::FindConnectionsForPiece(ABuildingPiece* SourcePiece) {
	if (!IsValid(SourcePiece)) return;
	
	const UStaticMeshComponent* SourceMesh = SourcePiece->GetPieceMesh();
	if (!IsValid(SourceMesh)) return;
	
	const FBox Bounds = SourceMesh->Bounds.GetBox();
	const FVector Centre = Bounds.GetCenter();
	const FVector Dimensions = Bounds.GetExtent() + FVector(ConnectionDistanceThreshold);
	
	if (bDrawConnectionDebug)
		DrawDebugBox(
			GetWorld(),
			Centre,
			Dimensions,
			FColor::Yellow,
			false,
			5.f,
			-1,
			1.5f
		);
	
	TArray<FOverlapResult> OverlapResults;
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(StructureConnectionOverlap), false);
	QueryParams.AddIgnoredActor(this);
	
	const bool bHasOverlaps = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		Centre,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeBox(Dimensions),
		QueryParams
	);
	
	if (!bHasOverlaps) return;
	
	for (const FOverlapResult& Result : OverlapResults) {
		AActor* HitActor = Result.GetActor();
		ABuildingPiece* OtherPiece = Cast<ABuildingPiece>(HitActor);
		
		if (!IsValid(OtherPiece) || OtherPiece == SourcePiece) continue;
		
		if (Pieces.Contains(OtherPiece))
			AddConnection(SourcePiece, OtherPiece);
	}
}

void AStructureActor::BuildConnections() {
	Connections.Empty();
	const double StartTime = FPlatformTime::Seconds();
	
	for (ABuildingPiece* Piece : Pieces) {
		if (!IsValid(Piece)) continue;
		Connections.FindOrAdd(TObjectKey<ABuildingPiece>(Piece));
	}
	
	if (!bAutoBuildConnects) return; // can hard code connections to debug
	
	for (ABuildingPiece* Piece : Pieces)
		FindConnectionsForPiece(Piece);
	
	DrawConnectionDebug();
	
	const double EndTime = FPlatformTime::Seconds();
	RuntimeMetrics.LastBuildMs = (EndTime - StartTime) * 1000.0;
	
	int32 TotalEdges = 0;
	
	for (const auto& Node : Connections)
		TotalEdges += Node.Value.Num();
	
	RuntimeMetrics.ConnectionCount = TotalEdges / 2; // Bidirectional graph so /2 for true count
}

void AStructureActor::CheckJobConditions() {
	ADemolitionGameState* GameState = GetWorld()->GetGameState<ADemolitionGameState>();
	if (!GameState || GameState->IsGameOver()) return;
	int32 MaxCharges = GameState->GetMaxCharges();
	int32 ChargesUsed = GameState->GetChargesUsed();
	
	if (HasFailedProtectedCondition() || (MaxCharges > 0 && ChargesUsed >= MaxCharges)) {
		GameState->FailJob();
		return;
	}
	
	if (HasMetObjectiveCondition())
		GameState->CompleteJob();
}

bool AStructureActor::HasFailedProtectedCondition() const {
	for (const ABuildingPiece* Piece : Pieces) {
		if (IsValid(Piece) && Piece->ShouldProtect() && Piece->IsBroken())
			return true;
	}
	
	return false;
}

bool AStructureActor::HasMetObjectiveCondition() const {
	bool bFoundObjective = false;
	for (const ABuildingPiece* Piece : Pieces) {
		if (!IsValid(Piece) || !Piece->IsObjective()) continue;
		
		bFoundObjective = true;
		
		if (!Piece->IsBroken())
			return false;
	}
	
	return bFoundObjective;
}

void AStructureActor::HandlePieceBroken(ABuildingPiece* BrokenPiece) {
	if (!IsValid(BrokenPiece)) return;
	
	if (StabilityModel)
		StabilityModel->OnPieceBroken(BrokenPiece);
	MarkStabilityDirty(); 
	
	if (bAutoRefreshOnBrokenPiece)
		RefreshStructureState();
}

void AStructureActor::RefreshStructureState() {
	if (bConnectionsDirty) {
		BuildConnections();
		bConnectionsDirty = false;
		bStabilityDirty = true; // would need to reconfigure supports
	}
	
	if (bStabilityDirty && StabilityModel) {
		StabilityModel->RefreshState();
		bStabilityDirty = false;
	}
	
	if (bLogStructureMetrics)
		LogStructureMetrics();
	
	CheckJobConditions();
}

void AStructureActor::DrawConnectionDebug() const {
	if (!bDrawConnectionDebug || !GetWorld()) return;
	
	for (const auto& Node : Connections) {
		ABuildingPiece* PieceA = Node.Key.ResolveObjectPtr();
		if (!IsValid(PieceA)) continue;
		
		for (const auto& WeakNeighbour : Node.Value) {
			ABuildingPiece* PieceB = WeakNeighbour.Get();
			if (!IsValid(PieceB)) continue;
			
			DrawDebugLine(
				GetWorld(),
				PieceA->GetActorLocation(),
				PieceB->GetActorLocation(),
				FColor::Cyan,
				false,
				5.f,
				-1,
				2.f
			);
		}
	}
}

void AStructureActor::LogStructureMetrics() const {
	UE_LOG(
		LogTemp,
		Log,
		TEXT("StructureActor '%s' | Pieces: %d | Connections: %d | Supported: %d | BuildConnections: %.3f ms | Resolve Graph: %.3f ms"),
		*GetDebugName(),
		RuntimeMetrics.PieceCount,
		RuntimeMetrics.ConnectionCount,
		RuntimeMetrics.SupportedCount,
		RuntimeMetrics.LastBuildMs,
		RuntimeMetrics.LastSolveMs
	);
}
