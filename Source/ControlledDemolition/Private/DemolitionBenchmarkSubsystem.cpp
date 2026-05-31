// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionBenchmarkSubsystem.h"

#include "BuildingPiece.h"
#include "ChargeActor.h"
#include "StructureActor.h"
#include "StructureStabilityModel_CONN.h"
#include "StructureStabilityModel_PHYS.h"
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
			return NewObject<UStructureStabilityModel_PHYS>(Owner);
		case EBenchmarkModel::CONN:
			return NewObject<UStructureStabilityModel_CONN>(Owner);
		case EBenchmarkModel::LOAD:
			return NewObject<UStructureStabilityModel_LOAD>(Owner);
		default:
			return nullptr;
		}
	}
	
	FString GetEnumValueString(FString Full) {
		int32 ColonPos;
		if (!Full.FindLastChar(':', ColonPos)) return Full;
		// removes the :: separator for cleaner output names
		return Full.RightChop(ColonPos + 1);
	}
}

namespace {
	struct FSweepProfileParams {
		TArray<int32> Counts;
		int32 Repeats;
	};
	
	FSweepProfileParams GetProfileParams(EBenchmarkScenario Scenario, EBenchmarkProfile Profile) {
		const bool bIsFullSweep = Profile == EBenchmarkProfile::Full;
		switch (Scenario) {
		case EBenchmarkScenario::AnchorRemoval:
		case EBenchmarkScenario::SupportRemoval:
			return bIsFullSweep 
				? FSweepProfileParams { {10, 25, 50, 100, 250}, 10 }
				: FSweepProfileParams { {5, 10}, 1 };
		case EBenchmarkScenario::LoadRedistribution:
			return bIsFullSweep 
				? FSweepProfileParams { {7}, 10 }
				: FSweepProfileParams { {7}, 1 };
		case EBenchmarkScenario::ProtectedPreservation:
			return bIsFullSweep 
				? FSweepProfileParams { {4, 5, 8, 15, 20}, 10 }
				: FSweepProfileParams { {5, 10}, 1 };
		}
		return {};
	}
}

void UDemolitionBenchmarkSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);
}

void UDemolitionBenchmarkSubsystem::Deinitialize() {
	CSVHandle.Reset();
	Super::Deinitialize();
}

FBox UDemolitionBenchmarkSubsystem::GetCurrentStructureBounds() const {
	const AStructureActor* Structure = CurrentStructure.Get();
	if (!IsValid(Structure)) return FBox(ForceInit);

	FBox Bounds(ForceInit);
	for (const ABuildingPiece* Piece : Structure->GetPieces())
		if (IsValid(Piece))
			Bounds += Piece->GetComponentsBoundingBox(true);
	return Bounds;
}

void UDemolitionBenchmarkSubsystem::SetPendingSingleSweep(EBenchmarkScenario Scenario, EBenchmarkProfile Profile) {
	PendingScenario = Scenario;
	PendingProfile = Profile;
}

void UDemolitionBenchmarkSubsystem::SetPendingAllSweeps(EBenchmarkProfile Profile) {
	PendingScenario.Reset();
	PendingProfile = Profile;
}

void UDemolitionBenchmarkSubsystem::RunPendingSweeps() {
	Queue.Empty();
	TotalRunsPerSweep.Empty();
	LastScenario = INDEX_NONE;
	CurrentSweepIndex = 0;
	CurrentRun = 0;
	TotalSweeps = PendingScenario.IsSet() ? 1 : 4;
	
	auto EnqueueSweep = [&](EBenchmarkScenario Scenario) {
		const FSweepProfileParams Params = GetProfileParams(Scenario, PendingProfile);
		int32 Count = 0;
		
		switch (Scenario) {
		case EBenchmarkScenario::AnchorRemoval:
			Count = EnqueueSweep_AnchorRemoval(Params.Counts, Params.Repeats); break;
		case EBenchmarkScenario::SupportRemoval:
			Count = EnqueueSweep_SupportRemoval(Params.Counts, Params.Repeats); break;
		case EBenchmarkScenario::LoadRedistribution:
			Count = EnqueueSweep_LoadRedistribution(Params.Repeats); break;
		case EBenchmarkScenario::ProtectedPreservation:
			Count = EnqueueSweep_ProtectedPreservation(Params.Counts, Params.Repeats); break;
		}
		
		TotalRunsPerSweep.Add(Count);
	};
	
	if (PendingScenario.IsSet())
		EnqueueSweep(*PendingScenario);
	else {
		EnqueueSweep(EBenchmarkScenario::AnchorRemoval);
		EnqueueSweep(EBenchmarkScenario::SupportRemoval);
		EnqueueSweep(EBenchmarkScenario::LoadRedistribution);
		EnqueueSweep(EBenchmarkScenario::ProtectedPreservation);
	}
	
	StartNextRun();
}

int32 UDemolitionBenchmarkSubsystem::EnqueueSweep_AnchorRemoval(const TArray<int32>& Counts, int32 Repeats) {
	const TArray<EBenchmarkModel> Models {EBenchmarkModel::PHYS, EBenchmarkModel::CONN, EBenchmarkModel::LOAD};
	
	int32 TotalRuns = 0;
	for (const EBenchmarkModel Model : Models)
		for (const int32 PieceCount : Counts)
			for (int32 i = 0; i < Repeats; i++) {
				FBenchmarkRunSpec Spec;
				Spec.Model = Model;
				Spec.Scenario = EBenchmarkScenario::AnchorRemoval;
				Spec.PieceCount = PieceCount;
				Spec.RunIndex = i;
				Spec.Seed = HashCombine(HashCombine(HashCombine(GetTypeHash(Model), GetTypeHash(Spec.Scenario)), PieceCount), i);
				Queue.Enqueue(Spec);
				TotalRuns++;
			}
	return TotalRuns;
}

int32 UDemolitionBenchmarkSubsystem::EnqueueSweep_SupportRemoval(const TArray<int32>& Counts, int32 Repeats) {
	const TArray<EBenchmarkModel> Models {EBenchmarkModel::PHYS, EBenchmarkModel::CONN, EBenchmarkModel::LOAD};
	
	int32 TotalRuns = 0;
	for (const EBenchmarkModel Model : Models)
		for (const int32 PieceCount : Counts)
			for (int32 i = 0; i < Repeats; i++) {
				FBenchmarkRunSpec Spec;
				Spec.Model = Model;
				Spec.Scenario = EBenchmarkScenario::SupportRemoval;
				Spec.PieceCount = PieceCount;
				Spec.RunIndex = i;
				Spec.Seed = HashCombine(HashCombine(HashCombine(GetTypeHash(Model), GetTypeHash(Spec.Scenario)), PieceCount), i);
				Queue.Enqueue(Spec);
				TotalRuns++;
			}
	return TotalRuns;
}

int32 UDemolitionBenchmarkSubsystem::EnqueueSweep_LoadRedistribution(int32 Repeats) {
	const TArray<EBenchmarkModel> Models {EBenchmarkModel::PHYS, EBenchmarkModel::CONN, EBenchmarkModel::LOAD};
	constexpr int32 PieceCount = 7;
	
	int32 TotalRuns = 0;
	for (const EBenchmarkModel Model : Models)
		for (int32 i = 0; i < Repeats; i++) {
			FBenchmarkRunSpec Spec;
			Spec.Model = Model;
			Spec.Scenario = EBenchmarkScenario::LoadRedistribution;
			Spec.PieceCount = PieceCount;
			Spec.RunIndex = i;
			Spec.Seed = HashCombine(HashCombine(HashCombine(GetTypeHash(Model), GetTypeHash(Spec.Scenario)), PieceCount), i);
			Queue.Enqueue(Spec);
			TotalRuns++;
		}
	return TotalRuns;
}

int32 UDemolitionBenchmarkSubsystem::EnqueueSweep_ProtectedPreservation(const TArray<int32>& Widths, int32 Repeats) {
	const TArray<EBenchmarkModel> Models {EBenchmarkModel::PHYS, EBenchmarkModel::CONN, EBenchmarkModel::LOAD};
	
	int32 TotalRuns = 0;
	for (const EBenchmarkModel Model : Models)
		for (const int32 BridgeWidth : Widths)
			for (int32 i = 0; i < Repeats; i++) {
				FBenchmarkRunSpec Spec;
				Spec.Model = Model;
				Spec.Scenario = EBenchmarkScenario::ProtectedPreservation;
				Spec.PieceCount = BridgeWidth; 
				Spec.RunIndex = i;
				Spec.Seed = HashCombine(HashCombine(HashCombine(GetTypeHash(Model), GetTypeHash(Spec.Scenario)), BridgeWidth), i);
				Queue.Enqueue(Spec);
				TotalRuns++;
			}
	return TotalRuns;
}

void UDemolitionBenchmarkSubsystem::BeginRun() {
	TriggeredPieceIndex = INDEX_NONE;
	
	const FBenchmarkRunSpec& Spec = *CurrentSpec;
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) { StartNextRun(); return; }
	
	
	CurrentRow = FBenchmarkRow{};
	CurrentRow.Model = GetEnumValueString(StaticEnum<EBenchmarkModel>()->GetNameByValue((int64)Spec.Model).ToString());
	CurrentRow.Scenario = GetEnumValueString(StaticEnum<EBenchmarkScenario>()->GetNameByValue((int64)Spec.Scenario).ToString());
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
	
	CurrentRun++;
	const int32 TotalRuns = TotalRunsPerSweep.IsValidIndex(CurrentSweepIndex-1) ? TotalRunsPerSweep[CurrentSweepIndex - 1] : 0;
	OnRunStarted.Broadcast(CurrentRun, TotalRuns);
	
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
	
	// Apply trigger
	switch (Spec.Scenario) {
	case EBenchmarkScenario::AnchorRemoval:
	case EBenchmarkScenario::LoadRedistribution:
		CurrentRow.TriggerEventMs = BreakPieceByRole(Structure, EPieceRole::Anchor, CurrentRng);
		Structure->RefreshStructureState();
		break;
	case EBenchmarkScenario::SupportRemoval:
		CurrentRow.TriggerEventMs = BreakPieceByRole(Structure, EPieceRole::Support, CurrentRng);
		Structure->RefreshStructureState();
		break;
	case EBenchmarkScenario::ProtectedPreservation:
		CurrentRow.TriggerEventMs = ExplodeAtRole(Structure, EPieceRole::Objective, CurrentRng);
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
		Structure->RefreshStructureState();
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
		
		// Read PHYS-specific metrics
		CurrentRow.ConstraintCount = Metrics.ConstraintCount;
		CurrentRow.ConstraintBreaks = Metrics.ConstraintBreaks;
		CurrentRow.ConstraintSpawnMs = Metrics.ConstraintSpawnMs;
		
		if (Spec.Model == EBenchmarkModel::PHYS)
			CurrentRow.DestructionRatio = CurrentRow.ConstraintCount > 0
				? float(CurrentRow.ConstraintBreaks) / CurrentRow.ConstraintCount
				: 0.f;
		
		// Read LOAD-specific metrics
		CurrentRow.CascadeIterations = Metrics.CascadeIterations;
		CurrentRow.OverloadFails = Metrics.OverloadFails;
		
		CurrentRow.ProtectedFailureRatio = CurrentRow.ProtectedTotal > 0
			? float(CurrentRow.ProtectedBroken) / CurrentRow.ProtectedTotal
			: 0.f;
		
		if (Spec.Scenario == EBenchmarkScenario::ProtectedPreservation)
			CurrentRow.PieceCount = Structure->GetPieces().Num();
		
		EvaluatePassFail(CurrentRow, Spec);
		
		Structure->Destroy();
	}
	
	WriteRow(CurrentRow);
	CurrentStructure.Reset();
	CurrentSpec.Reset();
	StartNextRun();
}

void UDemolitionBenchmarkSubsystem::StartNextRun() {
	FBenchmarkRunSpec Spec;
	if (!Queue.Dequeue(Spec)) {
		UE_LOG(LogTemp, Log, TEXT("[Benchmark] Sweep complete.")); 
		OnAllSweepsComplete.Broadcast();
		return;
	}
	
	const int32 ScenarioInt = (int32)Spec.Scenario;
	if (ScenarioInt != LastScenario) {
		LastScenario = ScenarioInt;
		CurrentSweepIndex++;
		CurrentRun = 0;
		OnSweepStarted.Broadcast(CurrentSweepIndex, TotalSweeps, Spec.Scenario);
	}
	
	CurrentSpec = Spec;
	CurrentRng = FRandomStream(Spec.Seed);
	BeginRun();
}

AStructureActor* UDemolitionBenchmarkSubsystem::
SpawnStructureForScenario(UWorld* World, const FBenchmarkRunSpec& Spec) {
	const FVector Origin(0.f, 0.f, 0.f);
	
	switch (Spec.Scenario) {
	case EBenchmarkScenario::AnchorRemoval:
	case EBenchmarkScenario::SupportRemoval:
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
		Target->BreakPiece();
	return (FPlatformTime::Seconds() - StartTime) * 1000.f;
}

float UDemolitionBenchmarkSubsystem::ExplodeAtRole(AStructureActor* Structure, const EPieceRole Role, const FRandomStream& Rng) {
	const double StartTime = FPlatformTime::Seconds();
	ABuildingPiece* Target = PickByRole(Structure, Role, Rng);
	if (!Target) return 0.f;
	const FVector SurfaceOffset {0.f, -52.5f, 0.f}; // CHEAT: placing charge on surface of target piece, assumes 100x100x100 cube
	const FVector Origin = Target->GetPieceCentreLocation() + SurfaceOffset;
	
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World) return 0.f;
	
	const FTransform Xform(FRotator::ZeroRotator, Origin);
	AChargeActor* Charge = World->SpawnActorDeferred<AChargeActor>(
		AChargeActor::StaticClass(),
		Xform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!Charge) return 0.f;
	
	Charge->SetFuseTime(0.5f); 
	Charge->SetExplosionRadius(100.f);
	Charge->SetExplosionImpulseStrength(1000.f);
	
	Charge->FinishSpawning(Xform);
		
	return (FPlatformTime::Seconds() - StartTime) * 1000.f;
}

ABuildingPiece* UDemolitionBenchmarkSubsystem::PickByRole(const AStructureActor* Structure, const EPieceRole Role,
	const FRandomStream& Rng) {
	if (!IsValid(Structure)) return nullptr;
	auto& Pieces = Structure->GetPieces();
	
	TArray<int32> MatchIndexes;
	for (int32 i = 0; i < Pieces.Num(); i++) {
		const ABuildingPiece* Piece = Pieces[i];
		if (IsValid(Piece) && Piece->GetPieceRole() == Role && !Piece->IsBroken())
			MatchIndexes.Add(i);
	}
	if (MatchIndexes.IsEmpty()) return nullptr;
	
	int32 TargetIndex = Rng.RandRange(0, MatchIndexes.Num()-1);
	TriggeredPieceIndex = MatchIndexes[TargetIndex];
	return Pieces[TriggeredPieceIndex];
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
			"StructureSpawnMs,BuildConnectionsMs,ModelInitialiseMs,ConstraintSpawnMs,TriggerEventMs,SolveMs,"
			"PeakFrameMs,AverageFrameMs,FramesObserved,"
			"BrokenPieces,SupportedPieces,ProtectedTotal,ProtectedBroken,"
			"ObjectiveTotal,ObjectiveBroken,AnchorTotal,AnchorBroken,LoadTotal,LoadBroken,ConstraintBreaks,"
			"CascadeIterations,OverloadFails,DestructionRatio,ProtectedFailureRatio,Passed,OutcomeLabel\n"
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
			 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
			 "%.3f,%.3f,%s,\"%s\"\n"),
		*Row.RunID, *Row.Timestamp, *Row.MapName, *Row.StructureName, *Row.Model, *Row.Scenario,
		Row.PieceCount, Row.ConnectionCount, Row.ConstraintCount, Row.RunIndex, Row.Seed,
		Row.StructureSpawnMs, Row.BuildConnectionsMs, Row.ModelInitialiseMs,Row.ConstraintSpawnMs, Row.TriggerEventMs, Row.SolveMs,
		Row.PeakFrameMs, Row.AverageFrameMs, Row.FramesObserved,
		Row.BrokenPieces, Row.SupportedPieces, Row.ProtectedTotal, Row.ProtectedBroken, 
		Row.ObjectiveTotal, Row.ObjectiveBroken, Row.AnchorTotal, Row.AnchorBroken, Row.LoadTotal, Row.LoadBroken, Row.ConstraintBreaks,
		Row.CascadeIterations, Row.OverloadFails, Row.DestructionRatio, Row.ProtectedFailureRatio,
		Row.Passed ? TEXT("PASS") : TEXT("FAIL"), *Row.OutcomeLabel
	);
	
	const FTCHARToUTF8 Utf8(*Line);
	CSVHandle->Write((const uint8*)Utf8.Get(), Utf8.Length());
	CSVHandle->Flush(); // won't lose data written to file if crash mid-sweep
}

void UDemolitionBenchmarkSubsystem::EvaluatePassFail(FBenchmarkRow& OutRow, const FBenchmarkRunSpec& Spec) const {
	const AStructureActor* Structure = CurrentStructure.Get();
	const bool bIsPHYS = Spec.Model == EBenchmarkModel::PHYS;
	bool bPHYSObjectiveSeparated = false;
	bool bPHYSProtectedDamaged = false;
			
	if (IsValid(Structure)) {
		const UStructureStabilityModel* Model = Structure->GetStabilityModelObj();
		bPHYSObjectiveSeparated = Model && Model->OverridesJobConditions() && Model->HasMetObjectiveCondition();
		bPHYSProtectedDamaged = Model && Model->OverridesJobConditions() && Model->HasFailedProtectedCondition();
	}
	
	if (bIsPHYS ? bPHYSProtectedDamaged : OutRow.ProtectedBroken > 0) {
		OutRow.Passed = false;
		OutRow.OutcomeLabel = bIsPHYS ? TEXT("ProtectedConstraintBroken") : TEXT("ProtectedBroken");
		return;
	}
	
	switch (Spec.Scenario) {
	case EBenchmarkScenario::AnchorRemoval:
	{
		/*
		* Simple tower:	Removing anchor should detach all pieces (PHYS: No effect - controlled by Chaos)
		* Pass:			For tower of N pieces, (N-1) non-anchor pieces should be broken
		*/
		if (bIsPHYS) {
			OutRow.Passed = OutRow.BrokenPieces == 1 && OutRow.ConstraintBreaks == 0;
			OutRow.OutcomeLabel = OutRow.Passed	? TEXT("ExpectedNoCollapse") : TEXT("UnexpectedCollapse");
			break;
		}
		const int32 ExpectedBrokenNonAnchorCount = Spec.PieceCount - 1;
		OutRow.Passed = OutRow.BrokenPieces - OutRow.AnchorBroken == ExpectedBrokenNonAnchorCount;
		OutRow.OutcomeLabel = OutRow.Passed ? TEXT("ExpectedCollapse") : TEXT("NoCollapse");
		break;
	}
	case EBenchmarkScenario::SupportRemoval:
	{
		if (bIsPHYS) {
			OutRow.Passed = OutRow.BrokenPieces == 1 && OutRow.ConstraintBreaks == 0;
			OutRow.OutcomeLabel = OutRow.Passed	? TEXT("ExpectedNoCollapse") : TEXT("UnexpectedCollapse");
			break;
		}
		// Pieces added to array such that the anchor is at the bottom (i=0) and others added in height order (ascending)
		const int32 ExpectedBrokenCount = Spec.PieceCount - TriggeredPieceIndex;
		OutRow.Passed = OutRow.BrokenPieces == ExpectedBrokenCount;
		OutRow.OutcomeLabel = OutRow.Passed ? TEXT("ExpectedCollapse") : TEXT("NoCollapse");
		break;
	}
	case EBenchmarkScenario::LoadRedistribution:
		switch (Spec.Model) {
		case EBenchmarkModel::PHYS:
			OutRow.Passed = OutRow.BrokenPieces == 1 && OutRow.ConstraintBreaks == 0;
			OutRow.OutcomeLabel = OutRow.Passed	? TEXT("ExpectedNoCollapse") : TEXT("UnexpectedCollapse");
			break;
		case EBenchmarkModel::CONN: // Pieces still connected to other anchor only trigger piece should break
			OutRow.Passed = OutRow.BrokenPieces == 1;
			OutRow.OutcomeLabel = OutRow.Passed	? TEXT("ExpectedNoLoadCascade") : TEXT("UnexpectedConnBehaviour");
			break;
		case EBenchmarkModel::LOAD:
			OutRow.Passed = OutRow.OverloadFails >= 1 && OutRow.BrokenPieces >= 4; 
			OutRow.OutcomeLabel = OutRow.Passed ? TEXT("ExpectedLoadCascade") : TEXT("InsufficientLoadCascade");
			break;
		} break;
	case EBenchmarkScenario::ProtectedPreservation:
		if (bIsPHYS) {
			OutRow.Passed = bPHYSObjectiveSeparated;
			OutRow.OutcomeLabel = OutRow.Passed ? TEXT("ObjectiveSeparatedProtectedPreserved") : TEXT("ObjectiveNotSeparated");
			break;
		}
		OutRow.Passed = OutRow.ObjectiveBroken >= 1;
		OutRow.OutcomeLabel = OutRow.Passed ? TEXT("ObjectiveDestroyedProtectedPreserved") : TEXT("ObjectiveSurvived");
		break;
	}
	
}
