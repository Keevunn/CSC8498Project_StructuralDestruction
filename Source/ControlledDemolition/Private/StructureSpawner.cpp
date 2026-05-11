// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureSpawner.h"

#include "BuildingPiece.h"
#include "StructureActor.h"

AStructureActor* UStructureSpawner::SpawnSimpleTower(const UObject* WorldContextObject, const FVector AnchorPos,
                                                     const int32 NumPieces, int32 Seed) {
	
	if (!GEngine) return nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || NumPieces < 2) return nullptr; // Structure must have at least 2 pieces (i.e. 1 edge in graph)
	
	const float PieceHeight = 100.f; // cm; Unreal default cube
	TArray<ABuildingPiece*> Pieces;
	Pieces.Reserve(NumPieces);
	
	for (int32 i = 0; i < NumPieces; i++) {
		const FVector Location = AnchorPos + FVector(0.f, 0.f, i * PieceHeight);
		const FTransform PieceXForm(FRotator::ZeroRotator, Location);
		
		ABuildingPiece* Piece = World->SpawnActorDeferred<ABuildingPiece>(
			ABuildingPiece::StaticClass(), 
			PieceXForm,
			nullptr, 
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
		if (!Piece) continue;
		
		// Assign role and load/capacity
		EPieceRole Role;
		if (i == 0) Role = EPieceRole::Anchor;					// First piece on tower labelled as anchor
		else if (i == NumPieces - 1) Role = EPieceRole::Load;	// Last piece on tower labelled as load
		else Role = EPieceRole::Support;						// All pieces in between labelled as supports 
		Piece->SetPieceRole(Role);
		
		switch (Role) {
		case EPieceRole::Anchor:
			Piece->SetLoad(0.f);
			Piece->SetCapacity(1e9f); // inf
			break;
		case EPieceRole::Support:
			Piece->SetLoad(5.f);
			Piece->SetCapacity(100.f); 
			break;
		case EPieceRole::Load:
			Piece->SetLoad(80.f);
			Piece->SetCapacity(30.f); // load source, not load bearing
			break;
		default: break;
		}
		
		Piece->FinishSpawning(PieceXForm);
		Pieces.Add(Piece);
	}
	
	// Spawns actor, doesn't run BeginPlay yet
	const FTransform StructureXForm(FRotator::ZeroRotator, AnchorPos);
	AStructureActor* Structure = World->SpawnActorDeferred<AStructureActor>(
		AStructureActor::StaticClass(),
		StructureXForm,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!Structure) return nullptr;
	
	Structure->AssignPieces(Pieces);
	// Can set stability model and debug name here
	
	Structure->FinishSpawning(StructureXForm); // Runs BeginPlay
	return Structure;
}

AStructureActor* UStructureSpawner::SpawnTwoSupportLoad(const UObject* WorldContextObject, FVector Origin, int32 Seed) {
	return nullptr;
}

AStructureActor* UStructureSpawner::SpawnBridge(const UObject* WorldContextObject, FVector Origin, int32 NumSpanPieces,
	int32 Seed) {
	return nullptr;
}
