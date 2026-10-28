#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InvoWallet.generated.h"

class FJsonObject;
class SInvoWallet;
class SWidget;

/** An in-game item the wallet's Store > Items tab sells for the game's own currency. */
USTRUCT(BlueprintType)
struct INVOSDK_API FInvoWalletItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvoSDK|Wallet")
    FString ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvoSDK|Wallet")
    FString Name;

    /** Price in game currency, a plain decimal string ("250", "4.99"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvoSDK|Wallet")
    FString Price;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvoWalletVisibilityChanged, bool, bOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvoWalletChanged);

/**
 * The INVO wallet window: Store (top-up packs and your items), Send, Transfer, Collect and the
 * player's phone, drawn in Slate over the game viewport. The same screens and copy as INVO's Unity
 * and Godot wallets. Every call goes through UInvoSubsystem, so it follows the same routing rules
 * (no Platform API key in a shipped build).
 *
 * Blueprint: Get Game Instance > Invo Wallet Subsystem > Open Wallet.
 */
UCLASS()
class INVOSDK_API UInvoWalletSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Deinitialize() override;

    /** Opens the wallet on a panel: Store, Send, Transfer, Collect or Phone. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Wallet")
    void OpenWallet(FName Panel = "Store");

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Wallet")
    void CloseWallet();

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Wallet")
    void ToggleWallet();

    UFUNCTION(BlueprintPure, Category = "InvoSDK|Wallet")
    bool IsWalletOpen() const { return bOpen; }

    /** The items the Store > Items tab sells for in-game currency. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Wallet")
    void SetItemCatalog(const TArray<FInvoWalletItem>& Items) { ItemCatalog = Items; }

    UFUNCTION(BlueprintPure, Category = "InvoSDK|Wallet")
    const TArray<FInvoWalletItem>& GetItemCatalog() const { return ItemCatalog; }

    /** Opened or closed. Pause or resume gameplay input here. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK|Wallet")
    FInvoWalletVisibilityChanged OnWalletVisibilityChanged;

    /** A purchase, send, transfer or collect finished: the balance may have changed. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK|Wallet")
    FInvoWalletChanged OnWalletChanged;

    /** Development builds: drives the wallet to one screen for store screenshots and demos, e.g.
     *  "store", "items", "items-confirm", "send", "send-review", "send-qr", "transfer", "transfer-qr",
     *  "collect", "collect-qr", "phone". Does nothing in Shipping. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Wallet", meta = (DevelopmentOnly))
    void ShowDemoScreen(const FString& Screen);

    // ---------------- used by the wallet widgets ----------------

    void NotifyWalletChanged();
    FString GetCurrencyName() const;
    FString GetGameName() const;

private:
    TSharedPtr<SInvoWallet> Wallet;
    TSharedPtr<SWidget> ViewportContent;
    TArray<FInvoWalletItem> ItemCatalog;
    bool bOpen = false;
    bool bHadCursor = false;

    void EnsureWidget();
};
