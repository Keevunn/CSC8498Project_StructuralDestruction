// Fill out your copyright notice in the Description page of Project Settings.


#include "StructurePhysicsBaselineModel.h"

#include "BuildingPiece.h"
#include "StructureActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

void UStructurePhysicsBaselineModel::Initialise(AStructureActor* InStructure) {
	// Cleanup
	for (FBaselineConstraintEdge& Edge : ConstraintEdges)
		if (IsValid(Edge.Constraint)) Edge.Constraint->DestroyComponent();
	ConstraintEdges.Empty();
	InitialPieceLocations.Empty();
	
	OwningStructure = InStructure;
	if (!OwningStructure.IsValid()) return;
	
	ConfigurePiecesForSetup(); // Sets mobility, mass, disables physics
	SpawnConstraintsFromAdjacency();
	EnablePhysicsOnAllPieces(); // Enables physics for non-anchored pieces
}

void UStructurePhysicsBaselineModel::SpawnConstraintsFromAdjacency() {
	AStructureActor* Structure = OwningStructure.Get();
	if (!Structure) return;
	
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
			Constraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = 200.f;
			
			Constraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = true;
			Constraint->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = 1500.f;
			Constraint->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = 50.f;
			
			Constraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = true;
			Constraint->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = 1500.f;
			Constraint->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = 50.f;
			
			Constraint->SetLinearBreakable(true, LinearBreakThreshold);
			Constraint->SetAngularBreakable(true, AngularBreakThreshold);
			
			RegisterConstraintEdge(Constraint, PieceA, PieceB);
			
			Constraint->OnConstraintBroken.AddDynamic(this, &UStructurePhysicsBaselineModel::HandleConstraintBroken);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[B] StructureActor '%s' | Constraints spawned: %d"),
		*Structure->GetDebugName(), 
		ConstraintEdges.Num()
	);
}

void UStructurePhysicsBaselineModel::RegisterConstraintEdge(UPhysicsConstraintComponent* Constraint,
	ABuildingPiece* PieceA, ABuildingPiece* PieceB) {
	if (!IsValid(Constraint) || !IsValid(PieceA) || !IsValid(PieceB)) return;
	const int32 EdgeIdx = ConstraintEdges.Num();
	
	FBaselineConstraintEdge Edge;
	Edge.Constraint = Constraint;
	Edge.PieceA = PieceA;
	Edge.PieceB = PieceB;
	Edge.bBroken = false;
	
	ConstraintEdges.Add(Edge);
	
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[B] Registered edge %d | %s <-> %s"),
		EdgeIdx,
		*PieceA->GetDebugName(),
		*PieceB->GetDebugName()
	);
}

void UStructurePhysicsBaselineModel::ConfigurePiecesForSetup() {
	for (ABuildingPiece* Piece : OwningStructure->GetPieces()) {
		if (!IsValid(Piece)) continue;
		UStaticMeshComponent* Mesh = Piece ? Piece->GetPieceMesh() : nullptr;
		if (!IsValid(Mesh)) continue; 
	
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetMassOverrideInKg(NAME_None, PieceMass, true); // TEMP should move mass to BuildingPiece
	
		Mesh->SetSimulatePhysics(false); // no physics simulation before constraints exist
		
		InitialPieceLocations.Add(TObjectKey<ABuildingPiece>(Piece), Piece->GetPieceCentreLocation());
	}
}

void UStructurePhysicsBaselineModel::EnablePhysicsOnAllPieces() {
	for (ABuildingPiece* Piece : OwningStructure->GetPieces()) {
		if (!IsValid(Piece)) continue;
		
		UStaticMeshComponent* Mesh = Piece->GetPieceMesh();
		if (!IsValid(Mesh)) continue;
	
		Mesh->SetSimulatePhysics(!Piece->IsAnchored());
	}
}

void UStructurePhysicsBaselineModel::HandleConstraintBroken(const int32 ConstraintIndex) {
	// Cost negligible, doesn't rely on physics index
	for (FBaselineConstraintEdge& Edge : ConstraintEdges) {
		if (Edge.bBroken || !IsValid(Edge.Constraint) || Edge.Constraint->IsBroken()) continue;
	
		Edge.bBroken = true;
		if (ABuildingPiece* A = Edge.PieceA.Get()) A->MarkBrokenFromPhysics();
		if (ABuildingPiece* B = Edge.PieceB.Get()) B->MarkBrokenFromPhysics();
	
		Edge.Constraint->DestroyComponent();
		Edge.Constraint = nullptr;
	}
		
}