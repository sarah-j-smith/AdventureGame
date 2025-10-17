#include "TestBarkController.h"

void UTestBarkController::PlayerBark(const FText& BarkText, int32 BarkTaskUid)
{
    FBarkRequest* Request = FBarkRequest::CreatePlayerRequest(BarkText);
    BarkRequests.Add(Request);
    IsBarking = true;
}

void UTestBarkController::PlayerBarkAndEnd(const FText& BarkText)
{
    FBarkRequest* Request = FBarkRequest::CreatePlayerRequest(BarkText);
    BarkRequests.Add(Request);
    ShouldInterruptAction = true;
    IsBarking = true;
}

void UTestBarkController::PlayerBarkLines(const TArray<FText>& BarkTextArray, int32 BarkTaskUid)
{
    FBarkRequest* Request = FBarkRequest::CreatePlayerMultilineRequest(BarkTextArray);
    BarkRequests.Add(Request);
    IsBarking = true;
}

void UTestBarkController::ClearBark()
{
    IsBarking = false;
    ShouldInterruptAction = false;
    BarkRequests.Empty();
}
