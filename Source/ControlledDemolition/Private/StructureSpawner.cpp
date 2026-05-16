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
	if (!GEngine) return nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;
	
	struct FPiecePlacement {
		FVector Offset;
		EPieceRole Role;
	};
	
	static const TArray<FPiecePlacement> Layout = {
		{FVector(-100.f, 0, 0.f), EPieceRole::Anchor},		// Anc1
		{FVector(100.f, 0, 0.f), EPieceRole::Anchor},			// Anc2
		{FVector(-100.f, 0, 100.f), EPieceRole::Support},		// Sup1
		{FVector(100.f, 0, 100.f), EPieceRole::Support},		// Sup2
		{FVector(-100.f, 0, 200.f), EPieceRole::Objective},	// Obj1
		{FVector(100.f, 0, 200.f), EPieceRole::Objective},	// Obj2
		{FVector(0.f, 0, 300.f), EPieceRole::Load},			// Load
	};
	
	TArray<ABuildingPiece*> Pieces;
	Pieces.Reserve(Layout.Num());
	for (const auto& [Offset, Role] : Layout) {
		const FTransform Xform(FRotator::ZeroRotator, Origin + Offset);
		
		ABuildingPiece* Piece = World->SpawnActorDeferred<ABuildingPiece>(
			ABuildingPiece::StaticClass(), 
			Xform, 
			nullptr, 
			nullptr, 
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
		if (!Piece) continue;
		
		Piece->SetPieceRole(Role); // Sets the LOAD properties
		Piece->FinishSpawning(Xform);
		Pieces.Add(Piece);
	}
	
	const FTransform StructureXForm(FRotator::ZeroRotator, Origin);
	AStructureActor* Structure = World->SpawnActorDeferred<AStructureActor>(
		AStructureActor::StaticClass(),
		StructureXForm,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!Structure) return nullptr;
	
	Structure->AssignPieces(Pieces);
	Structure->FinishSpawning(StructureXForm); // Runs BeginPlay
	return Structure;
}

AStructureActor* UStructureSpawner::SpawnBridge(const UObject* WorldContextObject, FVector Origin, int32 NumSpanPieces,
	int32 Seed) {
	return nullptr;
}
