#include "InvoSDKSettings.h"
#include "InvoHostedApproval.h"

#if WITH_EDITOR

void UInvoSDKSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
    Super::PreEditChange(PropertyAboutToChange);
    if (PropertyAboutToChange && PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UInvoSDKSettings, GameId))
    {
        GameIdBeforeEdit = GameId;
    }
}

void UInvoSDKSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(UInvoSDKSettings, GameId))
    {
        return;
    }

    // The value forms the `invo-sdk-<GameId>` return scheme on the game activity and in
    // Info.plist. Anything but digits cannot form a scheme, so the edit is refused: log it and
    // restore the last accepted value (empty is allowed - it means "not configured").
    FString Trimmed = GameId;
    Trimmed.TrimStartAndEndInline();
    if (Trimmed.IsEmpty() || UInvoHostedApprovalLibrary::IsValidGameId(Trimmed))
    {
        GameId = Trimmed;
        GameIdBeforeEdit = GameId;
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("InvoSDK: GameId '%s' refused - it must be the numeric id from the Invo console (digits only). Kept '%s'."),
           *GameId, *GameIdBeforeEdit);
    GameId = GameIdBeforeEdit;
}

#endif // WITH_EDITOR
