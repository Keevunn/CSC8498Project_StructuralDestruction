// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionSweepSelectWidget.h"

#include "DemolitionBenchmarkSubsystem.h"
#include "DemolitionPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

class UDemolitionBenchmarkSubsystem;

void UDemolitionSweepSelectWidget::NativeConstruct() {
    Super::NativeConstruct();

    if (Button_ProfileDemo)  
        Button_ProfileDemo->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleProfileDemoClicked);
    if (Button_ProfileFull)  
        Button_ProfileFull->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleProfileFullClicked);

    if (Button_Anchor)       
        Button_Anchor->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleAnchorClicked);
    if (Button_Support)      
        Button_Support->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleSupportClicked);
    if (Button_Load)         
        Button_Load->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleLoadClicked);
    if (Button_Protected)    
        Button_Protected->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleProtectedClicked);
    if (Button_All)          
        Button_All->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleAllClicked);

    if (Button_Back)         
        Button_Back->OnClicked.AddDynamic(this, &UDemolitionSweepSelectWidget::HandleBackClicked);

    UpdateProfileLabel();
}

void UDemolitionSweepSelectWidget::HandleProfileDemoClicked() {
    SelectedProfile = EBenchmarkProfile::Demo;
    UpdateProfileLabel();
}

void UDemolitionSweepSelectWidget::HandleProfileFullClicked() {
    SelectedProfile = EBenchmarkProfile::Full;
    UpdateProfileLabel();
}

void UDemolitionSweepSelectWidget::HandleAnchorClicked() {
    StartSingle(EBenchmarkScenario::AnchorRemoval);
}

void UDemolitionSweepSelectWidget::HandleSupportClicked() {
    StartSingle(EBenchmarkScenario::SupportRemoval);
}

void UDemolitionSweepSelectWidget::HandleLoadClicked() {
    StartSingle(EBenchmarkScenario::LoadRedistribution);
}

void UDemolitionSweepSelectWidget::HandleProtectedClicked() {
    StartSingle(EBenchmarkScenario::ProtectedPreservation);
}

void UDemolitionSweepSelectWidget::HandleAllClicked() {
    StartAll();
}

void UDemolitionSweepSelectWidget::HandleBackClicked() {
    if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
        PC->ShowMainMenu();
}

void UDemolitionSweepSelectWidget::UpdateProfileLabel() {
    if (!Text_Profile) return;
    const FString Label = SelectedProfile == EBenchmarkProfile::Demo ? TEXT("Profile: Demo") : TEXT("Profile: Full");
    Text_Profile->SetText(FText::FromString(Label));
}

void UDemolitionSweepSelectWidget::StartSingle(EBenchmarkScenario Scenario) {
    if (UGameInstance* GI = GetGameInstance())
        if (auto* BenchmarkSystem = GI->GetSubsystem<UDemolitionBenchmarkSubsystem>())
            BenchmarkSystem->SetPendingSingleSweep(Scenario, SelectedProfile);
    UGameplayStatics::OpenLevel(this, BenchmarkLevelName);
}

void UDemolitionSweepSelectWidget::StartAll() {
    if (UGameInstance* GI = GetGameInstance())
        if (auto* BenchmarkSystem = GI->GetSubsystem<UDemolitionBenchmarkSubsystem>())
            BenchmarkSystem->SetPendingAllSweeps(SelectedProfile);
    UGameplayStatics::OpenLevel(this, BenchmarkLevelName);
}

