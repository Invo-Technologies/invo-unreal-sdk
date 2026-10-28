#include "InvoWalletDemoActor.h"
#include "InvoMock.h"
#include "InvoSubsystem.h"
#include "InvoWallet.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AInvoWalletDemoActor::AInvoWalletDemoActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AInvoWalletDemoActor::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetGameInstance();
    UInvoSubsystem* Invo = GameInstance ? GameInstance->GetSubsystem<UInvoSubsystem>() : nullptr;
    UInvoWalletSubsystem* Wallet = GameInstance ? GameInstance->GetSubsystem<UInvoWalletSubsystem>() : nullptr;
    if (!Invo || !Wallet)
    {
        return;
    }

    if (APlayerController* PC = GameInstance->GetFirstLocalPlayerController())
    {
        EnableInput(PC);
        InputComponent->BindKey(ToggleKey, IE_Pressed, this, &AInvoWalletDemoActor::Toggle);
    }

    FString DemoScreen;
#if !UE_BUILD_SHIPPING
    // -InvoWalletDemo=send-qr opens one screen (store screenshots; see UInvoWalletSubsystem::ShowDemoScreen).
    FParse::Value(FCommandLine::Get(), TEXT("InvoWalletDemo="), DemoScreen);
    if (bUseOfflineMockWhenUnconfigured && !Invo->HasCredentials() && !Invo->IsOfflineMock())
    {
        Invo->EnableOfflineMock();
    }
    if (Invo->IsOfflineMock() && !Invo->HasPlayerToken())
    {
        TWeakObjectPtr<UInvoWalletSubsystem> WeakWallet(Wallet);
        const FName Panel = OpenOnStart;
        Invo->LoginSandboxNative(FInvoMock::Email, FInvoMock::Phone, FInvoMock::DisplayName, [WeakWallet, Panel, DemoScreen](const FInvoResult&)
        {
            if (!WeakWallet.IsValid())
            {
                return;
            }
            if (!DemoScreen.IsEmpty())
            {
                WeakWallet->ShowDemoScreen(DemoScreen);
            }
            else if (!Panel.IsNone())
            {
                WeakWallet->OpenWallet(Panel);
            }
        });
        return;
    }
    if (!DemoScreen.IsEmpty())
    {
        Wallet->ShowDemoScreen(DemoScreen);
        return;
    }
#endif
    if (!OpenOnStart.IsNone())
    {
        Wallet->OpenWallet(OpenOnStart);
    }
}

void AInvoWalletDemoActor::Toggle()
{
    if (UInvoWalletSubsystem* Wallet = GetGameInstance() ? GetGameInstance()->GetSubsystem<UInvoWalletSubsystem>() : nullptr)
    {
        Wallet->ToggleWallet();
    }
}
