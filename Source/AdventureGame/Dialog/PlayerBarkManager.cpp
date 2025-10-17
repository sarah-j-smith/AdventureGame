// (c) 2025 Sarah Smith


#include "PlayerBarkManager.h"

#include "BarkRequest.h"
#include "AdventureGame/AdventureGame.h"
#include "AdventureGame/Enums/BarkAction.h"
#include "AdventureGame/HUD/AdventureGameHUD.h"
#include "AdventureGame/Player/CommandManager.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UPlayerBarkManager::UPlayerBarkManager()
    : AdventureHUDWidget(nullptr)
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UPlayerBarkManager::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}

// Called every frame
void UPlayerBarkManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UPlayerBarkManager::PlayerBarkAndEnd(const FText &BarkText)
{
    IsBarking = true;
    const FBarkRequest* Request = FBarkRequest::CreatePlayerRequest(BarkText);
    AdventureHUDWidget->Bark->AddBarkRequest(Request);
}

void UPlayerBarkManager::PlayerBark(const FText& BarkText, int32 BarkTaskUid)
{
    IsBarking = true;
    const FBarkRequest* Request = FBarkRequest::CreatePlayerRequest(BarkText, 0.0f, BarkTaskUid);
    AdventureHUDWidget->Bark->AddBarkRequest(Request);
}

void UPlayerBarkManager::PlayerBarkLines(const TArray<FText>& BarkTextArray, int32 BarkTaskUid)
{
    if (BarkTextArray.IsEmpty()) return;
    IsBarking = true;
    const FBarkRequest* Request = FBarkRequest::CreatePlayerMultilineRequest(BarkTextArray);
    AdventureHUDWidget->Bark->AddBarkRequest(Request);
}

ACommandManager* UPlayerBarkManager::GetCommandManager()
{
    static TWeakObjectPtr<ACommandManager> CachedCommandManager;
    
    if (ACommandManager *CommandManager = Cast<ACommandManager>(GetOwner()))
    {
        return CommandManager;
    }
    if (ACommandManager* CommandManager = CachedCommandManager.Get()) return CommandManager;
    if (ACommandManager* CommandManager = Cast<ACommandManager>(UGameplayStatics::GetActorOfClass(this, ACommandManager::StaticClass())))
    {
        CachedCommandManager = CommandManager;
        return CommandManager;
    }
    UE_LOG(LogAdventureGame, Warning, TEXT("Command manager not available in UPlayerBarkManager"));
    return nullptr;
}

void UPlayerBarkManager::ClearBark()
{
    UE_LOG(LogAdventureGame, Warning, TEXT("ClearBark"));
    if (IsBarking)
    {
        UE_LOG(LogAdventureGame, Warning, TEXT("ClearBark - bIsBarking"));
        for (int32 BarkTask : CurrentBarkTasks)
        {
            EndPlayerBark.Broadcast(BarkTask);
        }
        CurrentBarkTasks.Empty();
        IsBarking = false;
    }
    AdventureHUDWidget->Bark->ClearText();
}

EBarkAction UPlayerBarkManager::IsPlayerBarking() const
{
    if (AdventureHUDWidget && AdventureHUDWidget->Bark)
    {
        if (AdventureHUDWidget->Bark->IsBarking())
        {
            if (AdventureHUDWidget->Bark->IsPlayerRequest())
            {
                return EBarkAction::PlayerBarking;
            }
            return EBarkAction::NPCBarking;
        }
    }
    return EBarkAction::NotBarking;
}

void UPlayerBarkManager::SetAdventureGameHUD(class UAdventureGameHUD* HUD)
{
    AdventureHUDWidget = HUD;
    AdventureHUDWidget->Bark->BarkRequestCompleteDelegate.AddUObject(this, &UPlayerBarkManager::OnEndBark);
}

void UPlayerBarkManager::OnEndBark(int32 BarkTaskId)
{
    if (const int TaskIndex = CurrentBarkTasks.IndexOfByKey(BarkTaskId); TaskIndex != INDEX_NONE)
    {
        CurrentBarkTasks.RemoveAt(TaskIndex);
        EndPlayerBark.Broadcast(BarkTaskId);
    }
    IsBarking = AdventureHUDWidget->Bark->IsBarking();
}
