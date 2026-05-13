// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionBenchmarkSubsystem.h"

#include "BuildingPiece.h"
#include "StructureActor.h"
#include "StructureConnectivityModel.h"
#include "StructurePhysicsBaselineModel.h"
#include "StructureSpawner.h"
#include "StructureStabilityModel_LOAD.h"

namespace {
	void CountPiecesByRole(const AStructureActor* Structure, FBenchmarkRow& Row) {
		int32 ObjectiveCount = 0, ProtectedCount = 0, AnchorCount = 0, LoadCount = 0;
		int32 ObjectiveBrokenCount = 0, ProtectedBrokenCount = 0, AnchorBrokenCount = 0, LoadBrokenCount = 0;
		
		for (const ABuildingPiece* Piece : Structure->GetPieces())
			if (IsValid(Piece))
				switch (Piece->GetPieceRole()) {
				case EPieceRole::Objective:
					ObjectiveCount++;
					if (Piece->IsBroken()) ObjectiveBrokenCount++;
					break;
				case EPieceRole::Protected:
					ProtectedCount++; 
					if (Piece->IsBroken()) ProtectedBrokenCount++;
					break;
				case EPieceRole::Anchor:
					AnchorCount++; 
					if (Piece->IsBroken()) AnchorBrokenCount++;
					break;	
				case EPieceRole::Load:
					LoadCount++; 
					if (Piece->IsBroken()) LoadBrokenCount++;
					break;
				case EPieceRole::Support:
				default:
					break;
				}
		
		Row.ObjectiveTotal = ObjectiveCount;
		Row.ObjectiveBroken = ObjectiveBrokenCount;
		Row.ProtectedTotal = ProtectedCount;
		Row.ProtectedBroken = ProtectedBrokenCount;
		Row.AnchorTotal = AnchorCount;
		Row.AnchorBroken = AnchorBrokenCount;
		Row.LoadTotal = LoadCount;
		Row.LoadBroken = LoadBrokenCount;
	}
	
	UStructureStabilityModel* CreateModelForSpec(AStructureActor* Owner, EBenchmarkModel Model) {
		switch (Model) {
		case EBenchmarkModel::PHYS:
			return NewObject<UStructurePhysicsBaselineModel>(Owner);
		case EBenchmarkModel::CONN:
			return NewObject<UStructureConnectivityModel>(Owner);
		case EBenchmarkModel::LOAD:
			return NewObject<UStructureStabilityModel_LOAD>(Owner);
		default:
			return nullptr;
		}
	}
}

void UDemolitionBenchmarkSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);
	RegisteredCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("benchmark.run"),
		TEXT("Run benchmark sweep. Usage: benchmark.run <sweep#>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDemolitionBenchmarkSubsystem::HandleConsoleCommand),
		ECVF_Default
	);
}

void UDemolitionBenchmarkSubsystem::Deinitialize() {
	if (RegisteredCommand) {
		IConsoleManager::Get().UnregisterConsoleObject(RegisteredCommand);
		RegisteredCommand = nullptr;
	}
	
	CSVHandle.Reset();
	Super::Deinitialize();
}

void UDemolitionBenchmarkSubsystem::RunSweep(int32 Sweep) {
	Queue.Empty();
	if (Sweep == 1) EnqueueSweep_AnchorRemoval();
	// call other tests from here when implemented ...
	
	StartNextRun();
}

void UDemolitionBenchmarkSubsystem::EnqueueSweep_AnchorRemoval() {
	const TArray<EBenchmarkModel> Models {EBenchmarkModel::PHYS, EBenchmarkModel::CONN};
	const TArray<int32> Counts {10, 50};
	const int32 Repeats = 3;
	
	for (EBenchmarkModel Model : Models)
	for (int32 PieceCount : Counts)
	for (int32 i = 0; i < Repeats; i++) {
		FBenchmarkRunSpec Spec;
		Spec.Model = Model;
		Spec.Scenario = EBenchmarkScenario::AnchorRemoval;
		Spec.PieceCount = PieceCount;
		Spec.RunIndex = i;
		Spec.Seed = HashCombine(HashCombine(GetTypeHash(Model), PieceCount), i);
		Queue.Enqueue(Spec);
	}
}

void UDemolitionBenchmarkSubsystem::BeginRun() {
	const FBenchmarkRunSpec& Spec = *CurrentSpec;
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) { StartNextRun(); return; }
	
	CurrentRow = FBenchmarkRow{};
	CurrentRow.Model = StaticEnum<EBenchmarkModel>()->GetNameByValue((int64)Spec.Model).ToString();
	CurrentRow.Scenario = StaticEnum<EBenchmarkScenario>()->GetNameByValue((int64)Spec.Scenario).ToString();
	CurrentRow.RunID = FString::Printf(TEXT("%s_%s_%d_R%02d_S%d"),
		*CurrentRow.Model, *CurrentRow.Scenario, Spec.PieceCount, Spec.RunIndex, Spec.Seed);
	CurrentRow.Timestamp = FDateTime::Now().ToIso8601();
	CurrentRow.MapName = World->GetMapName();
	CurrentRow.PieceCount = Spec.PieceCount;
	CurrentRow.RunIndex = Spec.RunIndex;
	CurrentRow.Seed = Spec.Seed;
	
	// Spawn Structure - no model attached
	const double SpawnStart = FPlatformTime::Seconds();
	AStructureActor* Structure = SpawnStructureForScenario(World, Spec);
	CurrentRow.StructureSpawnMs = (FPlatformTime::Seconds() - SpawnStart) * 1000.f;
	
	if (!Structure) { StartNextRun(); return; }
	CurrentStructure = Structure;
	CurrentRow.StructureName = Structure->GetDebugName();
	
	// Read post-spawn graph metrics
	const auto& Metrics = Structure->GetRuntimeMetrics();
	CurrentRow.BuildConnectionsMs = Metrics.LastBuildMs;
	CurrentRow.ConnectionCount = Metrics.ConnectionCount;
	
	// Attach model
	UStructureStabilityModel* Model = CreateModelForSpec(Structure, Spec.Model);
	if (!Model) { Structure->Destroy(); StartNextRun(); return; }
	
	Structure->SetStabilityModel(Model);
	
	const double ModelInitStart = FPlatformTime::Seconds();
	Model->Initialise(Structure);
	CurrentRow.ModelInitialiseMs = (FPlatformTime::Seconds() - ModelInitStart) * 1000.f;
	
	// Read PHYS-only setup metrics
	if (Spec.Model == EBenchmarkModel::PHYS)
		if (auto* PhysModel = Cast<UStructurePhysicsBaselineModel>(Model))
			CurrentRow.ConstraintCount = PhysModel->GetInitialConstraintCount(); // May be different to Connection count due to arrangement
		
	
	// Apply trigger
	switch (Spec.Scenario) {
	case EBenchmarkScenario::AnchorRemoval:
	case EBenchmarkScenario::LoadRedistribution:
		CurrentRow.TriggerEventMs = BreakPieceByRole(Structure, EPieceRole::Anchor, CurrentRng);
		break;
	case EBenchmarkScenario::ProtectedPreservation:
		CurrentRow.TriggerEventMs = ExplodeAtRole(Structure, EPieceRole::Objective, 200.f, 250.f, CurrentRng);
		break;
	}
	
	// Start sampling frame times
	Sampler = MakeUnique<FFrameTimeSampler>();
	// Wait async for observation window, then call EndRun
	World->GetTimerManager().SetTimer(WaitHandle, this, &UDemolitionBenchmarkSubsystem::EndRun, ObservationSeconds);	
}

void UDemolitionBenchmarkSubsystem::EndRun() {
	// Stop sampler, record frame metrics
	if (Sampler) {
		Sampler->Stop();
		CurrentRow.PeakFrameMs = Sampler->GetPeakMs();
		CurrentRow.AverageFrameMs = Sampler->GetAverageMs();
		CurrentRow.FramesObserved = Sampler->GetFramesObserved();
		Sampler.Reset();
	}

	// Evaluate structure
	if (AStructureActor* Structure = CurrentStructure.Get(); IsValid(Structure)) {
		Structure->ProcessDeferredUpdates();
		const FStructureResult Result = Structure->EvaluateStructure();
		CurrentRow.BrokenPieces = Result.BrokenPiecesCount;
		CurrentRow.DestructionRatio = Result.UnprotectedPiecesCount > 0 
					? float(Result.BrokenPiecesCount) / Result.UnprotectedPiecesCount 
					: 0.f;
		
		const auto& Metrics = Structure->GetRuntimeMetrics();
		CurrentRow.SolveMs = Metrics.LastSolveMs;
		CurrentRow.SupportedPieces = Metrics.SupportedCount;
		
		CountPiecesByRole(Structure, CurrentRow);
		
		// Read PHYS-only post-event metrics
		const FBenchmarkRunSpec& Spec = *CurrentSpec;
		if (Spec.Model == EBenchmarkModel::PHYS)
			if (auto* PhysModel = Cast<UStructurePhysicsBaselineModel>(Structure->GetStabilityModelObj()))
				CurrentRow.ConstraintBreaks = PhysModel->GetConstraintBreakCount();
		
		CurrentRow.ProtectedFailureRatio = CurrentRow.ProtectedTotal > 0
			? float(CurrentRow.ProtectedBroken) / CurrentRow.ProtectedTotal
			: 0.f;
		
		EvaluatePassFail(CurrentRow, Spec, Result);
		
		Structure->Destroy();
	}
	
	WriteRow(CurrentRow);
	CurrentStructure.Reset();
	CurrentSpec.Reset();
	StartNextRun();
}

void UDemolitionBenchmarkSubsystem::StartNextRun() {
	FBenchmarkRunSpec Spec;
	if (!Queue.Dequeue(Spec)) { UE_LOG(LogTemp, Log, TEXT("[Benchmark] Sweep complete.")); return; }
	
	CurrentSpec = Spec;
	CurrentRng = FRandomStream(Spec.Seed);
	BeginRun();
}

AStructureActor* UDemolitionBenchmarkSubsystem::
SpawnStructureForScenario(UWorld* World, const FBenchmarkRunSpec& Spec) {
	const FVector Origin(0.f, 0.f, 50.f);
	
	switch (Spec.Scenario) {
	case EBenchmarkScenario::AnchorRemoval:
		return UStructureSpawner::SpawnSimpleTower(this, Origin, Spec.PieceCount, Spec.Seed);
	case EBenchmarkScenario::LoadRedistribution:
		return UStructureSpawner::SpawnTwoSupportLoad(this, Origin, Spec.Seed);
	case EBenchmarkScenario::ProtectedPreservation:
		return UStructureSpawner::SpawnBridge(this, Origin, Spec.PieceCount, Spec.Seed);
	}
	return nullptr;
}

float UDemolitionBenchmarkSubsystem::BreakPieceByRole(AStructureActor* Structure, const EPieceRole Role,
                                                      const FRandomStream& Rng) {
	const double StartTime = FPlatformTime::Seconds();
	if (ABuildingPiece* Target = PickByRole(Structure, Role, Rng))
		Target->RecordBreak();
	return (FPlatformTime::Seconds() - StartTime) * 1000.f;
}

float UDemolitionBenchmarkSubsystem::ExplodeAtRole(AStructureActor* Structure, const EPieceRole Role, const float Radius,
	const float MaxDamage, const FRandomStream& Rng) {
	const double StartTime = FPlatformTime::Seconds();
	ABuildingPiece* Target = PickByRole(Structure, Role, Rng);
	if (!Target) return 0.f;
		
	const FVector Origin = Target->GetPieceCentreLocation();
	for (ABuildingPiece* Piece : Structure->GetPieces())
		if (IsValid(Piece) && !Piece->IsBroken())
			Piece->ApplyExplosionDamage(Origin, Radius, MaxDamage);
		
	return (FPlatformTime::Seconds() - StartTime) * 1000.f;
}

ABuildingPiece* UDemolitionBenchmarkSubsystem::PickByRole(const AStructureActor* Structure, const EPieceRole Role,
	const FRandomStream& Rng) {
	if (!IsValid(Structure)) return nullptr;
		
	TArray<ABuildingPiece*> Matches;
	for (ABuildingPiece* Piece : Structure->GetPieces()) 
		if (IsValid(Piece) && Piece->GetPieceRole() == Role && !Piece->IsBroken())
			Matches.Add(Piece);
		
	if (Matches.IsEmpty()) return nullptr;
	return Matches[Rng.RandRange(0, Matches.Num()-1)];
}

void UDemolitionBenchmarkSubsystem::EnsureFileOpen() {
	if (CSVHandle.IsValid()) return;
	
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("BenchmarkResults");
	const FString Path = Dir / TEXT("DemolitionBenchmark.csv");
	
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
	PF.CreateDirectoryTree(*Dir);
	
	const bool bNeedHeader = !PF.FileExists(*Path) || PF.FileSize(*Path) == 0;
	CSVHandle.Reset(PF.OpenWrite(*Path, true, true));
	if (!CSVHandle) return; 
	
	if (bNeedHeader) {
		const FString Header = TEXT(
			"RunID,Timestamp,MapName,StructureName,Model,Scenario,"
			"PieceCount,ConnectionCount,ConstraintCount,RunIndex,Seed,"
			"StructureSpawnMs,BuildConnectionsMs,ModelInitialiseMs,TriggerEventMs,SolveMs,CascadeMs,"
			"PeakFrameMs,AverageFrameMs,FramesObserved,"
			"BrokenPieces,SupportedPieces,DetachedPieces,ProtectedTotal,ProtectedBroken,"
			"ObjectiveTotal,ObjectiveBroken,AnchorTotal,AnchorBroken,LoadTotal,LoadBroken,ConstraintBreaks,"
			"CascadeIterations,OverloadFails,DestructionRatio,ProtectedFailureRatio,Passed,FailureModeNotes\n"
		);
		const FTCHARToUTF8 Utf8(*Header);
		CSVHandle->Write((const uint8*)Utf8.Get(), Utf8.Length());
	}
}

void UDemolitionBenchmarkSubsystem::WriteRow(const FBenchmarkRow& Row) {
	EnsureFileOpen();
	if (!CSVHandle) return;
	
	const FString Line = FString::Printf(
		TEXT("%s,%s,%s,%s,%s,%s,"
			 "%d,%d,%d,%d,%d,"
			 "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"
			 "%.3f,%.3f,%d,"
			 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
			 "%.3f,%.3f,%s,\"%s\"\n"),
		*Row.RunID, *Row.Timestamp, *Row.MapName, *Row.StructureName, *Row.Model, *Row.Scenario,
		Row.PieceCount, Row.ConnectionCount, Row.ConstraintCount, Row.RunIndex, Row.Seed,
		Row.StructureSpawnMs, Row.BuildConnectionsMs, Row.ModelInitialiseMs, Row.TriggerEventMs, Row.SolveMs, Row.CascadeMs,
		Row.PeakFrameMs, Row.AverageFrameMs, Row.FramesObserved,
		Row.BrokenPieces, Row.SupportedPieces, Row.DetachedPieces, Row.ProtectedTotal, Row.ProtectedBroken, 
		Row.ObjectiveTotal, Row.ObjectiveBroken, Row.AnchorTotal, Row.AnchorBroken, Row.LoadTotal, Row.LoadBroken, Row.ConstraintBreaks,
		Row.CascadeIterations, Row.OverloadFails, Row.DestructionRatio, Row.ProtectedFailureRatio,
		Row.Passed ? TEXT("true") : TEXT("false"), *Row.FailureModeNotes
	);
	
	const FTCHARToUTF8 Utf8(*Line);
	CSVHandle->Write((const uint8*)Utf8.Get(), Utf8.Length());
	CSVHandle->Flush(); // won't lose data written to file if crash mid-sweep
}

void UDemolitionBenchmarkSubsystem::EvaluatePassFail(FBenchmarkRow& OutRow, const FBenchmarkRunSpec& Spec,
                                                     const FStructureResult& Result) const {
	
	if (OutRow.ProtectedBroken > 0) {
		OutRow.Passed = false;
		OutRow.FailureModeNotes = TEXT("Protected pieces broken");
		return;
	}
	
	switch (Spec.Scenario) {
	case EBenchmarkScenario::AnchorRemoval:
	{
		/*
		* Simple tower:	Removing anchor should detach all pieces
		* Pass:			For tower of N pieces, (N-1) non-anchor pieces should be unsupported (anchors always marked as supported)
		* Tolerance:		70% non-anchor pieces should be broken (can be tuned) 
		*/
		
		const int32 ExpectedNonAnchorCount = Spec.PieceCount - 1;
		const int32 Threshold = FMath::Max(1, ExpectedNonAnchorCount * 0.7f); // Note: 70% threshold is more forgiving for PHYS (pieces can get stuck in each other physically)
		OutRow.Passed = OutRow.BrokenPieces - OutRow.AnchorBroken >= Threshold;
		OutRow.FailureModeNotes = OutRow.Passed ? TEXT("Collapse as expected") : TEXT("No collapse");
		break;
	}
	case EBenchmarkScenario::LoadRedistribution:
	case EBenchmarkScenario::ProtectedPreservation:
		OutRow.Passed = false;
		OutRow.FailureModeNotes = TEXT("Unimplemented");
		break;
	}
	
}

void UDemolitionBenchmarkSubsystem::HandleConsoleCommand(const TArray<FString>& Args) {
	const int32 Sweep = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1;
	RunSweep(Sweep);
}
