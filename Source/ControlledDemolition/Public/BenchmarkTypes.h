#pragma once

#include "CoreMinimal.h"
#include "BenchmarkTypes.generated.h"

UENUM() 
enum class EBenchmarkModel : uint8 {
	PHYS, CONN, LOAD
};

UENUM()
enum class EBenchmarkScenario : uint8 {
	AnchorRemoval, SupportRemoval, LoadRedistribution, ProtectedPreservation
};

UENUM()
enum class EBenchmarkProfile : uint8 {
	Demo, Full
};

USTRUCT()
struct FBenchmarkRunSpec {
	GENERATED_BODY()
	
	EBenchmarkModel Model;
	EBenchmarkScenario Scenario;
	int32 PieceCount = 10;
	int32 RunIndex = 0;
	int32 Seed = 0;
};

// Could turn results into relational database structure for reduced redundancy, faster lookup, etc.
struct FBenchmarkRow { 
	// Run metadata -----------------------------
	FString	RunID; 							// Unique ID: "<Model>_<Scenario>_<PieceCount>_R<RunIndex>_S<Seed>"
	FString	Timestamp; 						// Wall-time at run start (ISO-8601 format)
	FString	MapName; 						// World the structure spawned into
	FString	StructureName; 					// Debug name of spawned StructureActor
	FString	Model;							// PHYS/CONN/LOAD
	FString	Scenario; 						// AnchorRemoval / SupportRemoval / LoadDistribution / ProtectedPreservation
	
	// Structure shape --------------------------
	int32	PieceCount				= 0;	// Total number of spawned pieces
	int32	ConnectionCount			= 0;	// Total number of edges in adjacency graph (|E|)
	int32	ConstraintCount			= 0;	// (PHYS only) Total number of UPhysicsConstraintComponents
	int32	RunIndex				= 0;	// Repeat index (0-based)
	int32	Seed					= 0;	// RNG seed used for selecting random pieces for each scenario 
	
	// Setup timings (ms) -----------------------
	float	StructureSpawnMs		= 0.f;	// Time to spawn actors, assign roles, call FinishSpawning
	float	BuildConnectionsMs		= 0.f;	// Time to execute AStructureActor::BuildConnections()
	float	ModelInitialiseMs		= 0.f;	// Time to execute UStructureStabilityModel::Initialise() (PHYS: includes spawning constraints)
	float	ConstraintSpawnMs		= 0.f;	// (PHYS only) Time to execute UStructureStabilityModel_PHYS::SpawnConstraintsFromAdjacency()
	
	// Event timings (ms) -----------------------
	float	TriggerEventMs			= 0.f;	// Time to apply scenario trigger
	float	SolveMs					= 0.f;	// Time to execute UStructureStabilityModel::RefreshState() (PHYS: no graph solve, so set to 0)
	
	// Observation window -----------------------
	float	PeakFrameMs				= 0.f;	// Max frame time sampled during the post-trigger window
	float	AverageFrameMs			= 0.f;	// Mean frame time over the post-trigger window
	int32	FramesObserved			= 0;	// Number of frames sampled
	
	// Behavioural outcome ----------------------
	int32	BrokenPieces			= 0;	// Total number of pieces with bBroken set to true at evaluation time
	int32	SupportedPieces			= 0;	// Total number of pieces considered supported by graph-based model; 0 for PHYS
	int32	ProtectedTotal			= 0;	// Total number of pieces with the 'Protected' role at spawn
	int32	ProtectedBroken			= 0;	// Total number of 'Protected' pieces with bBroken set to true at evaluation time
	int32	ObjectiveTotal			= 0;	// Total number of pieces with the 'Objective' role at spawn
	int32	ObjectiveBroken			= 0;	// Total number of 'Objective' pieces with bBroken set to true at evaluation time
	int32	AnchorTotal				= 0;	// Total number of pieces with the 'Anchor' role at spawn
	int32	AnchorBroken			= 0;	// Total number of 'Anchor' pieces with bBroken set to true at evaluation time
	int32	LoadTotal				= 0;	// Total number of pieces with the 'Load' role at spawn
	int32	LoadBroken				= 0;	// Total number of 'Load' pieces with bBroken set to true at evaluation time
	int32	ConstraintBreaks		= 0;	// (PHYS only) Total number of broken constraints at evaluation time
	
	// LOAD diagnostics -------------------------
	int32	CascadeIterations		= 0;	// Total number of load redistribution passes performed
	int32	OverloadFails			= 0;	// Total number of failed pieces via capacity check
	
	// Gameplay metrics -------------------------
	float	DestructionRatio		= 0.f;	// Fraction of broken pieces out of all destructible pieces; For PHYS, this is the fraction of broken constraints
	float	ProtectedFailureRatio	= 0.f;	// Fraction of protected broken pieces out of all protected pieces (0 if no protected)
	bool	Passed					= false;// Scenario-specific success boolean
	FString	OutcomeLabel;					// Categorical label (see UDemolitionBenchmarkSubsystem::EvaluatePassFail())
};