// (c) 2025 Sarah Smith


#include "AdventureGameMode.h"

#include "AdventureGame/AdventureGame.h"

void AAdventureGameMode::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("AdventureGameMode::OnConstruction"));
}

void AAdventureGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("AdventureGameMode::BeginPlay"));
}
