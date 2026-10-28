#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "InvoWalletDemoActor.generated.h"

/**
 * Drop into any level to try the INVO wallet: opens it on BeginPlay and toggles it with F8.
 *
 * With no credentials configured (no sandbox key, token endpoint or game server) it switches the SDK
 * to the offline mock and signs in a demo player, so every screen works with no network, in
 * Development builds only. Configure Project Settings > Plugins > Invo SDK to use the real sandbox.
 */
UCLASS(Blueprintable, ClassGroup = "InvoSDK", meta = (DisplayName = "INVO Wallet Demo"))
class INVOSDK_API AInvoWalletDemoActor : public AActor
{
    GENERATED_BODY()

public:
    AInvoWalletDemoActor();

    /** Store, Send, Transfer, Collect or Phone. Empty: start closed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvoSDK")
    FName OpenOnStart = "Store";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvoSDK")
    FKey ToggleKey = EKeys::F8;

    /** Development builds with nothing configured: use the offline mock and a demo player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvoSDK")
    bool bUseOfflineMockWhenUnconfigured = true;

protected:
    virtual void BeginPlay() override;

private:
    void Toggle();
};
