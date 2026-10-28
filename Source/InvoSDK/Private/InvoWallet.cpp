#include "InvoWallet.h"
#include "InvoSDKSettings.h"
#include "InvoSubsystem.h"
#include "Wallet/SInvoWallet.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"

void UInvoWalletSubsystem::Deinitialize()
{
    CloseWallet();
    Wallet.Reset();
    ViewportContent.Reset();
    Super::Deinitialize();
}

void UInvoWalletSubsystem::EnsureWidget()
{
    if (Wallet.IsValid())
    {
        return;
    }
    TWeakObjectPtr<UInvoWalletSubsystem> WeakThis(this);
    SAssignNew(Wallet, SInvoWallet)
        .Invo(GetGameInstance()->GetSubsystem<UInvoSubsystem>())
        .Wallet(this)
        .OnCloseRequested_Lambda([WeakThis]
        {
            if (WeakThis.IsValid())
            {
                WeakThis->CloseWallet();
            }
        });
    ViewportContent = Wallet;
}

void UInvoWalletSubsystem::OpenWallet(FName Panel)
{
    UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient();
    if (!Viewport)
    {
        return;
    }
    EnsureWidget();
    if (!bOpen)
    {
        Viewport->AddViewportWidgetContent(ViewportContent.ToSharedRef(), 200);
        bOpen = true;
        if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
        {
            bHadCursor = PC->bShowMouseCursor;
            PC->SetShowMouseCursor(true);
            FInputModeGameAndUI Mode;
            Mode.SetWidgetToFocus(Wallet);
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(Mode);
        }
        OnWalletVisibilityChanged.Broadcast(true);
    }
    Wallet->ShowPanel(Panel);
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().SetKeyboardFocus(Wallet);
    }
}

void UInvoWalletSubsystem::CloseWallet()
{
    if (!bOpen)
    {
        return;
    }
    bOpen = false;
    if (UGameViewportClient* Viewport = GetGameInstance()->GetGameViewportClient())
    {
        Viewport->RemoveViewportWidgetContent(ViewportContent.ToSharedRef());
    }
    if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
    {
        PC->SetShowMouseCursor(bHadCursor);
        PC->SetInputMode(FInputModeGameOnly());
    }
    OnWalletVisibilityChanged.Broadcast(false);
}

void UInvoWalletSubsystem::ToggleWallet()
{
    if (bOpen)
    {
        CloseWallet();
    }
    else
    {
        OpenWallet(Wallet.IsValid() ? Wallet->GetCurrentPanel() : FName("Store"));
    }
}

void UInvoWalletSubsystem::ShowDemoScreen(const FString& Screen)
{
#if !UE_BUILD_SHIPPING
    OpenWallet("Store");
    Wallet->ShowDemoScreen(Screen.ToLower());
#endif
}

void UInvoWalletSubsystem::NotifyWalletChanged()
{
    OnWalletChanged.Broadcast();
}

FString UInvoWalletSubsystem::GetCurrencyName() const
{
    const FString Name = GetDefault<UInvoSDKSettings>()->CurrencyName.TrimStartAndEnd();
    return Name.IsEmpty() ? TEXT("Gold Crowns") : Name;
}

FString UInvoWalletSubsystem::GetGameName() const
{
    const FString Name = GetDefault<UInvoSDKSettings>()->GameName.TrimStartAndEnd();
    return Name.IsEmpty() ? TEXT("This game") : Name;
}
