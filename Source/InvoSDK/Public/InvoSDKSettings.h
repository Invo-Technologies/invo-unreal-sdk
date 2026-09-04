#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InvoSDKSettings.generated.h"

/**
 * Project Settings > Plugins > Invo SDK.
 *
 * GameId is the numeric id from the Invo console. It forms the hosted-approval return scheme
 * `invo-sdk-<GameId>` that the browser hands control back on; the Android UPL reads the same
 * value at package time to register the intent-filter, and the iOS build takes it from the
 * CFBundleURLTypes snippet in README.md ("Hosted approval on mobile").
 */
UCLASS(Config = Engine, DefaultConfig, meta = (DisplayName = "Invo SDK"))
class INVOSDK_API UInvoSDKSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Numeric game id from the Invo console. Digits only; anything else is refused in the editor. */
    UPROPERTY(Config, EditAnywhere, Category = "Hosted Approval")
    FString GameId;

    virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }

#if WITH_EDITOR
    virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
    /** Last accepted value, restored when an edit is refused. */
    FString GameIdBeforeEdit;
#endif
};
