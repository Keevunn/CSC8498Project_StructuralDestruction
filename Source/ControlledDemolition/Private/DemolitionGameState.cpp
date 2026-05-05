// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionGameState.h"

#include "BuildingPiece.h"
#include "DemolitionPlayerController.h"
#include "GameEconomySubsystem.h"
#include "JobManagerSubsystem.h"
#include "StructureActor.h"

void ADemolitionGameState::BeginPlay() {
	Super::BeginPlay();
	
	if (UGameInstance* GI = GetGameInstance())
		if (UJobManagerSubsystem* JobManager = GI->GetSubsystem<UJobManagerSubsystem>()) {
			FJobDefinition Job;
			Job.JobName = "Simple Tower Demo";
			Job.LevelName = "Level_SimpleTower";
			Job.MaxCharges = 5;
			MaxCharges = Job.MaxCharges;
			
			JobManager->StartJob(Job);
		}
	
}

void ADemolitionGameState::RegisterStructure(AStructureActor* Structure) {
	if (!IsValid(Structure)) return;
	
	ActiveStructures.AddUnique(Structure);
}

void ADemolitionGameState::UnregisterStructure(AStructureActor* Structure) {
	ActiveStructures.Remove(Structure);
}

void ADemolitionGameState::RegisterChargePlaced() {
	if (bGameOver) return;
	
	ChargesUsed++;
	
	OnChargePlaced.Broadcast(ChargesUsed, MaxCharges);
}

void ADemolitionGameState::CompleteJob() {
	if (bGameOver) return;
	
	bGameOver = true;
	
	FJobResult Result = EvaluateStructures();
	
	Result.FinalScore = CalculateScore(Result.DestructionRatio, Result.bProtectedIntact);
	Result.MoneyEarned = ConvertScoreToMoney(Result.FinalScore);
	
	if (const UGameInstance* GI = GetGameInstance()) 
		if (UGameEconomySubsystem* Economy = GI->GetSubsystem<UGameEconomySubsystem>())
			Economy->AddMoney(Result.MoneyEarned);
	
	UE_LOG(LogTemp, Log, TEXT("Job Complete | Score: %.1f | Money: £%d"),
		Result.FinalScore,
		Result.MoneyEarned
	);
	
	LastJobResult = Result;
	OnJobCompleted.Broadcast(Result.bProtectedIntact, Result.FinalScore, Result.MoneyEarned, Result.DestructionRatio);
	
}

void ADemolitionGameState::FailJob() {
	if (bGameOver) return;
	
	bGameOver = true;
	
	FJobResult Result = EvaluateStructures();
	Result.FinalScore = 0.f;
	Result.MoneyEarned = 0;
	
	UE_LOG(LogTemp, Log, TEXT("Job Failed | Score: 0| Money: £0"));
	
	LastJobResult = Result;
	OnJobCompleted.Broadcast(Result.bProtectedIntact, 0.f, 0.f, Result.DestructionRatio);
}

FJobResult ADemolitionGameState::EvaluateStructures() const {
	FJobResult Result;
	
	bool bProtectedIntact = true;
	int32 TotalBrokenPieces = 0;
	int32 TotalUnprotectedPieces = 0;
	for (AStructureActor* Structure : ActiveStructures) {
		const FStructureResult StructureResult = Structure->EvaluateStructure();
		
		if (!StructureResult.bProtectedIntact) bProtectedIntact = false;
		
		TotalBrokenPieces += StructureResult.BrokenPiecesCount;
		TotalUnprotectedPieces += StructureResult.UnprotectedPiecesCount;
	}
	
	Result.DestructionRatio = TotalUnprotectedPieces > 0 
		? static_cast<float>(TotalBrokenPieces) / TotalUnprotectedPieces 
		: 0.f;
	Result.bProtectedIntact = bProtectedIntact;
	Result.ChargesUsed = ChargesUsed;
	
	return Result;
}

float ADemolitionGameState::CalculateScore(const float DestructionRatio, const bool bProtectedIntact) const {
	float Score = DestructionRatio * 100.f;
	if (bProtectedIntact) Score += 50; // +50 bonus when protected pieces intact
	Score += (MaxCharges - ChargesUsed) * 10.f; // Bonus for using less than target, penalty for going above
	
	return FMath::Max(Score, 0.f);
}

int32 ADemolitionGameState::ConvertScoreToMoney(float Score) const {
	return FMath::RoundToInt(Score * 1.25f);
}
