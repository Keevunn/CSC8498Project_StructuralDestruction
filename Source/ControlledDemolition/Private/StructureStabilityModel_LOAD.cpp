// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureStabilityModel_LOAD.h"

#include "BuildingPiece.h"
#include "StructureActor.h"

void UStructureStabilityModel_LOAD::Initialise(AStructureActor* InStructure) {
	OwningStructure = InStructure;
}

void UStructureStabilityModel_LOAD::RefreshState() {
	if (bRefreshActive) return;
	
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	bRefreshActive = true;
	int32 CascadeIters = 0;
	int32 TotalOverloads = 0;
	
	while (CascadeIters < MaxCascadeIterations) {
		TMap<TObjectKey<ABuildingPiece>, int32> Layers;
		BuildLayers(Layers); // builds a map of nodes and their distance from nearest anchor
		
		TArray<ABuildingPiece*> BreakList;
		for (ABuildingPiece* Piece : Structure->GetPieces()) {
			if (!IsValid(Piece) || Piece->IsBroken() || Piece->IsAnchor()) continue;
			if (!Layers.Contains(TObjectKey<ABuildingPiece>(Piece))) BreakList.Add(Piece); // Break all disconnected pieces (no path to anchor)
		}
		
		TMap<TObjectKey<ABuildingPiece>, float> AccumLoad;
		AccumulateLoad(Layers, AccumLoad);
		
		for (const auto& Pair : AccumLoad) {
			ABuildingPiece* Piece = Pair.Key.ResolveObjectPtr();
			if (!IsValid(Piece) || Piece->IsAnchor() || Piece->IsBroken()) continue;
			// break piece if piece load exceeds capacity
			if (Pair.Value > Piece->GetCapacity()) {
				BreakList.AddUnique(Piece);
				TotalOverloads++;
			}
		}
		if (BreakList.IsEmpty()) break; // nothing to break
		
		for (ABuildingPiece* Piece : BreakList)
			Piece->BreakPiece();
		
		CascadeIters++;
		DrawLoadDebug();
	}
	
	LastCascadeIterations = CascadeIters;
	LastOverloadFailsCount = TotalOverloads;
	
	bRefreshActive = false;
}

void UStructureStabilityModel_LOAD::WriteMetrics(FStructureRuntimeMetrics& OutMetrics) const {
	OutMetrics.CascadeIterations += LastCascadeIterations;
	OutMetrics.OverloadFails += LastOverloadFailsCount;
}

void UStructureStabilityModel_LOAD::BuildLayers(TMap<TObjectKey<ABuildingPiece>, int32>& OutLayers) const {
	const AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	const auto& Pieces = Structure->GetPieces();
	const auto& Connections = Structure->GetConnections();
	
	TQueue<ABuildingPiece*> Queue;
	
	// All anchors given layer value = 0
	for (ABuildingPiece* Piece : Pieces)
		if (IsValid(Piece) && Piece->IsAnchor() && !Piece->IsBroken()) {
			OutLayers.Add(TObjectKey<ABuildingPiece>(Piece), 0);
			Queue.Enqueue(Piece);
		}
	
	// BFS - neighbour gets layer = current + 1
	while (!Queue.IsEmpty()) {
		ABuildingPiece* CurrentPiece;
		Queue.Dequeue(CurrentPiece);
		if (!IsValid(CurrentPiece)) continue;
		
		const TObjectKey<ABuildingPiece> CurrentKey(CurrentPiece);
		const int32 CurrentLayer = OutLayers[CurrentKey];
		
		const auto* Neighbours = Connections.Find(CurrentKey);
		if (!Neighbours) continue;
		
		for (const TWeakObjectPtr<ABuildingPiece>& Weak : *Neighbours) {
			ABuildingPiece* Neighbour = Weak.Get();
			if (!IsValid(Neighbour) || Neighbour->IsBroken()) continue;
			
			const TObjectKey<ABuildingPiece> NeighbourKey(Neighbour);
			if (OutLayers.Contains(NeighbourKey)) continue; // Already visited at shorter distance
			
			OutLayers.Add(NeighbourKey, CurrentLayer + 1);
			Queue.Enqueue(Neighbour);
		}
	}
}

void UStructureStabilityModel_LOAD::AccumulateLoad(const TMap<TObjectKey<ABuildingPiece>, int32>& Layers,
	TMap<TObjectKey<ABuildingPiece>, float>& OutAccumLoad) const {
	const AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	const auto& Connections = Structure->GetConnections();
	
	for (const auto& Pair : Layers) 
		if (ABuildingPiece* Piece = Pair.Key.ResolveObjectPtr())
			OutAccumLoad.Add(Pair.Key, Piece->GetLoad());
	
	// Descending order: furtherest from anchor first
	TArray<TObjectKey<ABuildingPiece>> SortedKeys;
	Layers.GenerateKeyArray(SortedKeys);
	SortedKeys.Sort([&Layers](const TObjectKey<ABuildingPiece>& A, const TObjectKey<ABuildingPiece>& B) {
		return Layers[A] > Layers[B];
	});
	
	// Top-down traversal 
	for (const TObjectKey<ABuildingPiece>& Key : SortedKeys) {
		const int32 CurrentLayer = Layers[Key];
		if (CurrentLayer == 0) continue; // Anchor, no supporters
		
		TArray<TObjectKey<ABuildingPiece>> Supporters;
		const auto* Neighbours = Connections.Find(Key);
		if (!Neighbours) continue;
		
		for (const TWeakObjectPtr<ABuildingPiece>& Weak : *Neighbours) {
			ABuildingPiece* Neighbour = Weak.Get();
			if (!IsValid(Neighbour) || Neighbour->IsBroken()) continue;
			
			const TObjectKey<ABuildingPiece> NeighbourKey(Neighbour);
			const int32* NeighbourLayer = Layers.Find(NeighbourKey);
			if (!NeighbourLayer || *NeighbourLayer >= CurrentLayer) continue; // further from the anchor
			
			Supporters.Add(NeighbourKey);
		}
		if (Supporters.IsEmpty()) continue;
		
		const float Share = OutAccumLoad[Key] / Supporters.Num();
		for (const TObjectKey<ABuildingPiece>& SupporterKey : Supporters)
			if (float* ExistingLoad = OutAccumLoad.Find(SupporterKey))
				*ExistingLoad += Share;
		
	}
}

void UStructureStabilityModel_LOAD::DrawLoadDebug() const {
	if (!bDrawLoadInfoDebug) return;
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	TMap<TObjectKey<ABuildingPiece>, int32> Layers;
	TMap<TObjectKey<ABuildingPiece>, float> AccumLoad;
	BuildLayers(Layers);
	AccumulateLoad(Layers, AccumLoad);
	
	for (const ABuildingPiece* Piece : Structure->GetPieces()) {
		if (!IsValid(Piece)) continue;
		
		FColor Colour;
		if (Piece->IsBroken()) Colour = FColor::Red;
		else if (Piece->IsAnchor()) Colour = FColor::Blue;
		else {
			const float* Found = AccumLoad.Find(TObjectKey<ABuildingPiece>(Piece));
			const float Load = Found ? *Found : 0.f;
			const float Capacity = Piece->GetCapacity();
			const float Stress = Capacity > 0.f ? FMath::Clamp(Load / Capacity, 0.f, 1.f) : 0.f;
			
			Colour = FColor::MakeRedToGreenColorFromScalar(1.f - Stress);
		}
		
		DrawDebugSphere(
			Structure->GetWorld(),
			Piece->GetPieceCentreLocation(),
			25.f, 8,
			Colour,
			false, 2.f, -1
		);
	}
}
