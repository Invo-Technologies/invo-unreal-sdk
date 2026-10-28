#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InvoTypes.h"
#include "InvoSDKSettings.generated.h"

/**
 * Project Settings > Plugins > Invo SDK.
 *
 * GameId is the numeric id from the Invo console. It forms the hosted-approval return scheme
 * `invo-sdk-<GameId>` that the browser hands control back on; the Android UPL reads the same
 * value at package time to register the intent-filter, and the iOS build takes it from the
 * CFBundleURLTypes snippet in README.md ("Hosted approval on mobile").
 *
 * Nothing secret lives here: this is DefaultEngine.ini, which ships with the game. The sandbox
 * key goes in UInvoSDKLocalSettings (your own Saved folder) or the INVO_SANDBOX_SECRET
 * environment variable; the production key stays on your game server.
 */
UCLASS(Config = Engine, DefaultConfig, meta = (DisplayName = "Invo SDK"))
class INVOSDK_API UInvoSDKSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Which INVO environment the game talks to. Sandbox keys start with ivsdk_test_, production keys with ivsdk_live_. */
    UPROPERTY(Config, EditAnywhere, Category = "Environment")
    EInvoEnvironment Environment = EInvoEnvironment::Sandbox;

    /** Numeric game id from Test mode at console.invo.network/test. */
    UPROPERTY(Config, EditAnywhere, Category = "Environment", meta = (DisplayName = "Sandbox Game ID"))
    FString SandboxGameId;

    /** Numeric game id from console.invo.network. */
    UPROPERTY(Config, EditAnywhere, Category = "Environment", meta = (DisplayName = "Production Game ID"))
    FString ProductionGameId;

    /** Production: your server, which forwards /api/* to INVO with the Platform API key attached. The
     *  wallet's store, send, transfer and phone calls go here with the player's token. */
    UPROPERTY(Config, EditAnywhere, Category = "Production", meta = (DisplayName = "Game Server URL"))
    FString GameServerUrl;

    /** Your endpoint that mints player tokens: receives your own JSON, returns {"token", "identity_id"?}. */
    UPROPERTY(Config, EditAnywhere, Category = "Production", meta = (DisplayName = "Player Token Endpoint"))
    FString PlayerTokenEndpoint;

    /** Your endpoint that takes {player_email, usd_amount} and returns {checkout_url} for real-money top-ups. */
    UPROPERTY(Config, EditAnywhere, Category = "Production", meta = (DisplayName = "Checkout Session Endpoint"))
    FString CheckoutSessionEndpoint;

    /** Older single game id, used for the hosted-approval return scheme and when the per-environment id is empty. */
    UPROPERTY(Config, EditAnywhere, Category = "Hosted Approval")
    FString GameId;

    /** Shown in the wallet header. */
    UPROPERTY(Config, EditAnywhere, Category = "Display")
    FString GameName;

    /** The currency's name in the wallet ("Gems", "Coins"). Empty uses the name INVO returns. */
    UPROPERTY(Config, EditAnywhere, Category = "Display")
    FString CurrencyName;

    /** The game id for Env, falling back to GameId. */
    FString GameIdFor(EInvoEnvironment Env) const;

    virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }

#if WITH_EDITOR
    virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
    /** Last accepted values, restored when an edit is refused. */
    TMap<FName, FString> BeforeEdit;
#endif
};

/**
 * Project Settings > Plugins > Invo SDK (Local). Stored in Saved/Config (per user, never
 * committed and never packaged). Only Development and Debug builds read it; Shipping builds
 * refuse direct sandbox mode outright.
 */
UCLASS(Config = EditorPerProjectUserSettings, meta = (DisplayName = "Invo SDK (Local)"))
class INVOSDK_API UInvoSDKLocalSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Sandbox Platform API key (ivsdk_test_...) from Test mode at console.invo.network/test.
     *  The INVO_SANDBOX_SECRET environment variable wins when set. Production keys are refused. */
    UPROPERTY(Config, EditAnywhere, Category = "Sandbox Only", meta = (DisplayName = "Sandbox Platform API key", PasswordField = true))
    FString SandboxSecret;

    virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};
