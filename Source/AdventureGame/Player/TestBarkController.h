#pragma once

#include "AdventureGame/Dialog/BarkRequest.h"
#include "AdventureGame/Dialog/PlayerBarkManager.h"

#include "TestBarkController.generated.h"

UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class UTestBarkController: public UPlayerBarkManager
{
    GENERATED_BODY()

public:
    virtual void PlayerBarkAndEnd(const FText& BarkText) override;
    
    virtual void PlayerBark(const FText& BarkText, int32 BarkTaskUid = 0) override;
    
    virtual void PlayerBarkLines(const TArray<FText>& BarkTextArray, int32 BarkTaskUid = 0) override;
    
    virtual void ClearBark() override;

    bool IsBarking = false;

    bool ShouldInterruptAction = false;

    TArray<FBarkRequest*> BarkRequests;
};
