// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureStabilityModel_CONN.h"

#include "BuildingPiece.h"
#include "StructureActor.h"

void UStructureStabilityModel_CONN::Initialise(AStructureActor* InStructure) {
	OwningStructure = InStructure;
}

void UStructureStabilityModel_CONN::RefreshState() {
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	const auto& Pieces = Structure->GetPieces();
	const auto& Connections = Structure->GetConnections();
	
	for (ABuildingPiece* Piece : Pieces)
		if (IsValid(Piece)) Piece->SetSupported(false);
	
	TQueue<ABuildingPiece*> Queue;
	bool bFoundAnchor = false;
	
	for (ABuildingPiece* Piece : Pieces) {
		if (!IsValid(Piece) || !Piece->IsAnchor() || Piece->IsBroken()) 
			continue;
		
		// Unbroken Anchors supported set to true
		Piece->SetSupported(true);
		Queue.Enqueue(Piece);
		bFoundAnchor = true;
	}
	
	if (!bFoundAnchor)
		UE_LOG(LogTemp, Warning, TEXT("StructureActor '%s' | No assigned anchors"), *Structure->GetDebugName());
	
	// BFS - sets 'supported' to true as long as piece is connected
	while (!Queue.IsEmpty()) {
		ABuildingPiece* Current;
		Queue.Dequeue(Current);
		
		if (!IsValid(Current)) continue;
		
		const TObjectKey<ABuildingPiece> CurrentKey(Current);
		const TArray<TWeakObjectPtr<ABuildingPiece>>* Neighbours = Connections.Find(CurrentKey);
		if (!Neighbours) continue;
		
		for (const TWeakObjectPtr<ABuildingPiece>& WeakNeighbour : *Neighbours) {
			ABuildingPiece* Neighbour = WeakNeighbour.Get();
			if (!IsValid(Neighbour) || Neighbour->IsBroken() || Neighbour->IsSupported()) continue;
			
			Neighbour->SetSupported(true);
			Queue.Enqueue(Neighbour);
		}
	}
	
	DetachUnsupportedPieces();
	
	if (bDrawSupportDebug) {
		for (const ABuildingPiece* Piece : Pieces) {
			if (!IsValid(Piece)) continue;
		
			const FColor Colour = Piece->IsSupported() ? FColor::Green : FColor::Red;
		
			DrawDebugSphere(
				GetWorld(),
				Piece->GetActorLocation(),
				25.f,
				8,
				Colour,
				false,
				2.f,
				-1
			);
		}
	}
}

void UStructureStabilityModel_CONN::DetachUnsupportedPieces() {
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	for (ABuildingPiece* Piece : Structure->GetPieces())
		if (IsValid(Piece) && !Piece->IsBroken() && !Piece->IsSupported() && !Piece->IsAnchor()) Piece->BreakPiece();
}
