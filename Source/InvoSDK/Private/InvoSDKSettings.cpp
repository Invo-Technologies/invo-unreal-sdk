#include "InvoSDKSettings.h"
#include "InvoHostedApproval.h"

FString UInvoSDKSettings::GameIdFor(EInvoEnvironment Env) const
{
    FString Id = Env == EInvoEnvironment::Production ? ProductionGameId : SandboxGameId;
    Id.TrimStartAndEndInline();
    if (Id.IsEmpty())
    {
        Id = GameId;
        Id.TrimStartAndEndInline();
    }
    return Id;
}

#if WITH_EDITOR

static FString* GameIdField(UInvoSDKSettings* Settings, FName Name)
{
    if (Name == GET_MEMBER_NAME_CHECKED(UInvoSDKSettings, GameId)) { return &Settings->GameId; }
    if (Name == GET_MEMBER_NAME_CHECKED(UInvoSDKSettings, SandboxGameId)) { return &Settings->SandboxGameId; }
    if (Name == GET_MEMBER_NAME_CHECKED(UInvoSDKSettings, ProductionGameId)) { return &Settings->ProductionGameId; }
    return nullptr;
}

void UInvoSDKSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
    Super::PreEditChange(PropertyAboutToChange);
    if (PropertyAboutToChange)
    {
        if (FString* Field = GameIdField(this, PropertyAboutToChange->GetFName()))
        {
            BeforeEdit.Add(PropertyAboutToChange->GetFName(), *Field);
        }
    }
}

void UInvoSDKSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    const FName Name = PropertyChangedEvent.GetPropertyName();
    FString* Field = GameIdField(this, Name);
    if (!Field)
    {
        return;
    }

    // GameId forms the `invo-sdk-<GameId>` return scheme on the game activity and in Info.plist,
    // and every id is numeric on the platform. Anything but digits is refused: log it and restore
    // the last accepted value (empty is allowed - it means "not configured").
    FString Trimmed = *Field;
    Trimmed.TrimStartAndEndInline();
    if (Trimmed.IsEmpty() || UInvoHostedApprovalLibrary::IsValidGameId(Trimmed))
    {
        *Field = Trimmed;
        BeforeEdit.Add(Name, Trimmed);
        return;
    }

    const FString Previous = BeforeEdit.FindRef(Name);
    UE_LOG(LogTemp, Error, TEXT("InvoSDK: %s '%s' refused - it must be the numeric id from the Invo console (digits only). Kept '%s'."),
           *Name.ToString(), **Field, *Previous);
    *Field = Previous;
}

#endif // WITH_EDITOR
