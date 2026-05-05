// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureConnectivityModel.h"

#include "BuildingPiece.h"
#include "StructureActor.h"

void UStructureConnectivityModel::Initialise(AStructureActor* InStructure) {
	OwningStructure = InStructure;
}

void UStructureConnectivityModel::RefreshState() {
	ReconfigureSupport();
	// ? Anything else ?
}

void UStructureConnectivityModel::OnPieceBroken(ABuildingPiece* BrokenPiece) {
	// Intentionally empty - Graph resolved from refresh state
}

void UStructureConnectivityModel::ReconfigureSupport() {
	const double StartSec = FPlatformTime::Seconds();
	
	const auto& Pieces = OwningStructure->GetPieces();
	const auto& Connections = OwningStructure->GetConnections();
	
	for (ABuildingPiece* Piece : Pieces)
		if (IsValid(Piece)) Piece->SetSupported(false);
	
	TQueue<ABuildingPiece*> Queue;
	bool bFoundAnchor = false;
	
	for (ABuildingPiece* Piece : Pieces) {
		if (!IsValid(Piece) || !Piece->IsAnchored() || Piece->IsBroken()) 
			continue;
		
		// Anchors supported set to true
		Piece->SetSupported(true);
		Queue.Enqueue(Piece);
		bFoundAnchor = true;
	}
	
	if (!bFoundAnchor) {
		UE_LOG(LogTemp, Warning, TEXT("StructureActor '%s' | No assigned anchors"), *OwningStructure->GetDebugName());
		DetachUnsupportedPieces();
		return;
	}
	
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
			if (!IsValid(Neighbour) || Neighbour->IsBroken()) continue;
			
			if (Neighbour->IsSupported()) continue;
			
			Neighbour->SetSupported(true);
			Queue.Enqueue(Neighbour);
		}
	}
	
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
	
	const double EndSec = FPlatformTime::Seconds();
	const float ElapsedMs = (EndSec - StartSec) * 1000.0;
	OwningStructure->RecordMetrics(ElapsedMs);
}

void UStructureConnectivityModel::DetachUnsupportedPieces() {
	const auto& Pieces = OwningStructure->GetPieces();
	
	for (ABuildingPiece* Piece : Pieces)
		if (IsValid(Piece) && !Piece->IsBroken() && !Piece->IsSupported() && !Piece->IsAnchored()) Piece->ForceBreakPiece();
}
