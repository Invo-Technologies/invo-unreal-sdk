#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FJsonObject;
class SWidgetSwitcher;
class SInvoWalletPage;
class SInvoStorePage;
class SInvoMovePage;
class SInvoCollectPage;
class UInvoSubsystem;
class UInvoWalletSubsystem;

/** What every wallet page shares: the SDK, the wallet subsystem, navigation and the last balance. */
struct FInvoWalletContext : public TSharedFromThis<FInvoWalletContext>
{
    TWeakObjectPtr<UInvoSubsystem> Invo;
    TWeakObjectPtr<UInvoWalletSubsystem> Wallet;
    TFunction<void(FName)> ShowPanel;
    TSharedPtr<FJsonObject> LastBalance;
    /** Development builds: hold the approval step on its QR code instead of polling (screenshots). */
    bool bDemoHoldApproval = false;

    /** "120 Gold Crowns", or "-" before the first balance arrives. */
    FString BalanceText() const;
    void RefreshBalance();
    void NotifyChanged();
};

/** The wallet sheet: header tabs, sign-in, and the Store / Send / Transfer / Collect / Phone pages. */
class SInvoWallet : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoWallet) {}
        SLATE_ARGUMENT(TWeakObjectPtr<UInvoSubsystem>, Invo)
        SLATE_ARGUMENT(TWeakObjectPtr<UInvoWalletSubsystem>, Wallet)
        SLATE_EVENT(FSimpleDelegate, OnCloseRequested)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void ShowPanel(FName Panel);
    FName GetCurrentPanel() const { return Current; }

    /** See UInvoWalletSubsystem::ShowDemoScreen. */
    void ShowDemoScreen(const FString& Screen);

    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
    TSharedPtr<FInvoWalletContext> Ctx;
    FSimpleDelegate OnCloseRequested;
    FName Current = "Store";
    TSharedPtr<SWidgetSwitcher> RootSwitcher;
    TSharedPtr<SWidgetSwitcher> PageSwitcher;
    TMap<FName, TSharedPtr<SInvoWalletPage>> Pages;
    TMap<FName, TSharedPtr<class SButton>> TabButtons;
    TArray<FName> PageOrder;
    TSharedPtr<SInvoStorePage> Store;
    TSharedPtr<SInvoMovePage> Send;
    TSharedPtr<SInvoMovePage> Transfer;
    TSharedPtr<SInvoCollectPage> Collect;
    TSharedPtr<class SEditableTextBox> SignInEmail;
    TSharedPtr<class SEditableTextBox> SignInPhone;
    TSharedPtr<class STextBlock> SignInError;

    TSharedRef<SWidget> BuildHeader();
    TSharedRef<SWidget> BuildSignIn();
    void AddPage(FName Id, TSharedRef<SInvoWalletPage> Page);
    void SignIn();
};
