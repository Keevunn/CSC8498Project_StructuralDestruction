// Fill out your copyright notice in the Description page of Project Settings.


#include "StructureStabilityModel_PHYS.h"

#include "BuildingPiece.h"
#include "DemolitionDebug.h"
#include "StructureActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

void UStructureStabilityModel_PHYS::Initialise(AStructureActor* InStructure) {
	// Cleanup
	for (FBaselineConstraintEdge& Edge : ConstraintEdges)
		if (IsValid(Edge.Constraint)) Edge.Constraint->DestroyComponent();
	ConstraintEdges.Empty();
	ConstraintBreakCount = 0;
	
	OwningStructure = InStructure;
	if (!OwningStructure.IsValid()) return;
	
	ConfigurePiecesForSetup(); // Sets mobility, mass, disables physics
	SpawnConstraintsFromAdjacency();
	EnablePhysicsOnPieces(); // Enables physics for non-anchored pieces
}

bool UStructureStabilityModel_PHYS::HasMetObjectiveCondition() const {
	const AStructureActor* Structure = OwningStructure.Get();
	if (!IsValid(Structure)) return false;
	
	bool bFoundObjective = false;
	for (const ABuildingPiece* Piece : Structure->GetPieces()) {
		if (!IsValid(Piece) || !Piece->IsObjective()) continue;
		
		bFoundObjective = true;
		
		const int32 TotalAttached = CountAttachedConstraints(Piece, false);
		const int32 UnbrokenAttached = CountAttachedConstraints(Piece, true);
		
		// All objectives must be detached -> win
		const bool bHasMetCondition = TotalAttached > 0 && UnbrokenAttached == 0;
		
		if (!bHasMetCondition) return false;
	}
	
	return bFoundObjective;
}

bool UStructureStabilityModel_PHYS::HasFailedProtectedCondition() const {
	const AStructureActor* Structure = OwningStructure.Get();
	if (!IsValid(Structure)) return false;

	for (const ABuildingPiece* Piece : Structure->GetPieces()) {
		if (!IsValid(Piece) || !Piece->ShouldProtect()) continue;

		const int32 TotalAttached = CountAttachedConstraints(Piece, false);
		const int32 UnbrokenAttached = CountAttachedConstraints(Piece, true);

		// Any protected pieces disconnected -> lose
		const bool bHasFailedCondition = TotalAttached > 0 && UnbrokenAttached < TotalAttached;

		if (bHasFailedCondition) return true;
	}

	return false;
}

void UStructureStabilityModel_PHYS::WriteMetrics(FStructureRuntimeMetrics& OutMetrics) const {
	OutMetrics.ConstraintCount = ConstraintEdges.Num();
	OutMetrics.ConstraintBreaks = ConstraintBreakCount;
	OutMetrics.ConstraintSpawnMs = ConstraintSpawnMs;
}

void UStructureStabilityModel_PHYS::SpawnConstraintsFromAdjacency() {
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	const double StartTime = FPlatformTime::Seconds();
	
	// Constraint threshold calculations
	const int32 NumPieces = Structure->GetPieces().Num();
	const float StaticLoadCeiling = NumPieces * PieceMass * Gravity;  // worst case, all pieces stacked on one joint
	
	const float LinearBreakThreshold = StaticLoadCeiling * SafetyFactor;
	const float AngularBreakThreshold = StaticLoadCeiling * LeverArmScale;
	
	using FEdgePair = TPair<TObjectKey<ABuildingPiece>, TObjectKey<ABuildingPiece>>;
	TSet<FEdgePair> SeenEdges;
	for (const auto& Node : Structure->GetConnections()) {
		ABuildingPiece* PieceA = Node.Key.ResolveObjectPtr();
		if (!IsValid(PieceA)) continue;
		
		for (const TWeakObjectPtr<ABuildingPiece>& WeakB : Node.Value) {
			ABuildingPiece* PieceB = WeakB.Get();
			if (!IsValid(PieceB)) continue;
			
			// Adjacency map is bidirectional
			const TObjectKey<ABuildingPiece> KeyA(PieceA), KeyB(PieceB);
			auto Edge = KeyA < KeyB ? FEdgePair(KeyA, KeyB) : FEdgePair(KeyB, KeyA);
			if (SeenEdges.Contains(Edge)) continue;
			SeenEdges.Add(Edge);
			
			UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(Structure);
			if (!IsValid(Constraint)) continue;
			Structure->AddInstanceComponent(Constraint); //  Visible from editor
			
			Constraint->RegisterComponent();
			Constraint->AttachToComponent(Structure->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			
			const FVector MidPoint = (PieceA->GetPieceCentreLocation() + PieceB->GetPieceCentreLocation()) * 0.5f;
			Constraint->SetWorldLocation(MidPoint);
			
			Constraint->SetConstrainedComponents(
				PieceA->GetPieceMesh(), NAME_None,
				PieceB->GetPieceMesh(), NAME_None
			);
			
			// limited range of movement
			Constraint->SetLinearXLimit(LCM_Limited, 1.f);
			Constraint->SetLinearYLimit(LCM_Limited, 1.f);
			Constraint->SetLinearZLimit(LCM_Limited, 1.f);
			
			Constraint->SetAngularSwing1Limit(ACM_Limited, 2.f);
			Constraint->SetAngularSwing2Limit(ACM_Limited, 2.f);
			Constraint->SetAngularTwistLimit(ACM_Limited, 2.f);
			
			// Soft limits
			Constraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
			Constraint->ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = 1500.f;
			Constraint->ConstraintInstance.ProfileInstance.LinearLimit.Damping = 200.f;
			
			Constraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = true;
			Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = 1500.f;
			Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Damping = 50.f;
			
			Constraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = true;
			Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = 1500.f;
			Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Damping = 50.f;
			
			Constraint->SetLinearBreakable(true, LinearBreakThreshold);
			Constraint->SetAngularBreakable(true, AngularBreakThreshold);
			
			RegisterConstraintEdge(Constraint, PieceA, PieceB);
			
			Constraint->OnConstraintBroken.AddDynamic(this, &UStructureStabilityModel_PHYS::HandleConstraintBroken);
		}
	}
	
	ConstraintSpawnMs = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
}

void UStructureStabilityModel_PHYS::RegisterConstraintEdge(UPhysicsConstraintComponent* Constraint,
                                                           ABuildingPiece* PieceA, ABuildingPiece* PieceB) {
	if (!IsValid(Constraint) || !IsValid(PieceA) || !IsValid(PieceB)) return;
	
	FBaselineConstraintEdge Edge;
	Edge.Constraint = Constraint;
	Edge.PieceA = PieceA;
	Edge.PieceB = PieceB;
	Edge.bBroken = false;
	
	ConstraintEdges.Add(Edge);
}

void UStructureStabilityModel_PHYS::ConfigurePiecesForSetup() {
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	for (ABuildingPiece* Piece : Structure->GetPieces()) {
		if (!IsValid(Piece)) continue;
		UStaticMeshComponent* Mesh = Piece ? Piece->GetPieceMesh() : nullptr;
		if (!IsValid(Mesh)) continue; 
	
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetMassOverrideInKg(NAME_None, PieceMass, true); // TODO should move mass to BuildingPiece
	
		Mesh->SetSimulatePhysics(false); // no physics simulation before constraints exist
	}
}

void UStructureStabilityModel_PHYS::EnablePhysicsOnPieces() {
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	for (ABuildingPiece* Piece : Structure->GetPieces()) {
		if (!IsValid(Piece)) continue;
		
		UStaticMeshComponent* Mesh = Piece->GetPieceMesh();
		if (!IsValid(Mesh)) continue;
	
		Mesh->SetSimulatePhysics(!Piece->IsAnchor());
	}
}

void UStructureStabilityModel_PHYS::DrawConstraintDebug() const {
	if (!DemolitionDebug::DrawDebugEnabled()) return;
	
	const AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
	UWorld* World = Structure->GetWorld();
	if (!World) return;
	
	for (const FBaselineConstraintEdge& Edge: ConstraintEdges) {
		const ABuildingPiece* A = Edge.PieceA.Get();
		const ABuildingPiece* B = Edge.PieceB.Get();
		if (!IsValid(A) || !IsValid(B)) continue;
		
		const FColor Colour = Edge.bBroken ? FColor::Red : FColor::Green;
		const float Thickness = Edge.bBroken ? 4.f : 2.f;
		
		DrawDebugLine(
			World, 
			A->GetPieceCentreLocation(),
			B->GetPieceCentreLocation(),
			Colour,
			false, 
			2.f,
			-1,
			Thickness
		);
	}
}

void UStructureStabilityModel_PHYS::HandleConstraintBroken(const int32 ConstraintIndex) {
	// Cost negligible, doesn't rely on physics index
	for (FBaselineConstraintEdge& Edge : ConstraintEdges) {
		if (Edge.bBroken || !IsValid(Edge.Constraint) || !Edge.Constraint->IsBroken()) continue;
	
		Edge.bBroken = true;
		ConstraintBreakCount++;
	
		Edge.Constraint->DestroyComponent();
		Edge.Constraint = nullptr;
	}
	
	DrawConstraintDebug();
}

int32 UStructureStabilityModel_PHYS::CountAttachedConstraints(const ABuildingPiece* Piece, const bool bOnlyUnbroken) const {
	if (!IsValid(Piece)) return 0;
	
	int32 Count = 0;
	for (const FBaselineConstraintEdge& Edge : ConstraintEdges) {
		const bool bContainsPiece = Edge.PieceA.Get() == Piece || Edge.PieceB.Get() == Piece;
		
		if (!bContainsPiece || (bOnlyUnbroken && Edge.bBroken)) continue;
		
		Count++;
	}
	return Count;
}
