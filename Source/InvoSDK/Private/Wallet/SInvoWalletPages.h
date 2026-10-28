#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/StrongObjectPtr.h"
#include "Styling/SlateBrush.h"
#include "InvoTypes.h"
#include "InvoWallet.h"
#include "Wallet/SInvoWallet.h"

class FActiveTimerHandle;
class FJsonObject;
class SBorder;
class SButton;
class SEditableTextBox;
class SHorizontalBox;
class SImage;
class STextBlock;
class SUniformGridPanel;
class SVerticalBox;
class SWidgetSwitcher;
class UTexture2D;

/** A wallet page: told when it becomes visible so it can refresh. */
class SInvoWalletPage : public SCompoundWidget
{
public:
    virtual void OnShown() {}

protected:
    TSharedPtr<FInvoWalletContext> Ctx;
};

/**
 * "Approve on your phone": begins a device grant, shows its QR code and user code, polls until the
 * phone answers (answering the first-time match-code prompt on the way), then settles. The device
 * code is single use, so nothing here is retried.
 */
class SInvoApprovalView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoApprovalView) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx);

    /** Flow: transfer | send | send_receipt | transfer_receipt. Summary is the line above the QR. */
    void Start(const FString& TransactionId, const FString& Flow, const FString& Summary);
    /** Stops polling. Nothing has moved until the phone approves, so this is safe. */
    void Cancel();

    /** The money step finished: the settle result and what to tell the player ("" = default copy). */
    TFunction<void(TSharedPtr<FJsonObject>, const FString&)> OnFinished;
    /** Nothing moved (declined, expired, cancelled = ""). */
    TFunction<void(const FString&)> OnFailed;

private:
    TSharedPtr<FInvoWalletContext> Ctx;
    int32 Run = 0;
    FString TransactionId, Flow, DeviceCode, ApproveUrl;
    double Deadline = 0;
    int32 Interval = 5;
    int32 Waited = 0;
    int32 Transient = 0;
    bool bPolling = false;
    TStrongObjectPtr<UTexture2D> QrTexture;
    FSlateBrush QrBrush;
    TSharedPtr<FActiveTimerHandle> Timer;

    TSharedPtr<STextBlock> SummaryText, CodeLine, Status, MatchCode;
    TSharedPtr<SWidget> MatchBox, OpenAgain, QrFrame;

    EActiveTimerReturnType TickPoll(double CurrentTime, float DeltaTime);
    void Poll();
    void Settle();
    void Fail(const FString& Message);
    void StopTimer();
    void AnswerMatch(bool bApprove);
    FString Clock() const;
};

/** Store: "Top up" (packs paid in INVO's hosted checkout) and "Items" (paid from the in-game balance). */
class SInvoStorePage : public SInvoWalletPage
{
public:
    SLATE_BEGIN_ARGS(SInvoStorePage) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx);
    virtual void OnShown() override;

    void ShowSub(bool bItems);
    void ConfirmItem(int32 Index);

private:
    bool bLoaded = false;
    bool bItems = false;
    TSharedPtr<SButton> TopUpTab, ItemsTab;
    TSharedPtr<STextBlock> Message;
    TSharedPtr<SVerticalBox> TopUp;
    TSharedPtr<SVerticalBox> ItemsPage;
    TSharedPtr<SBorder> Confirm;

    void Load();
    void BuildItems();
    TArray<FInvoWalletItem> Catalog() const;
    TSharedRef<SWidget> HeroCard(const TSharedPtr<FJsonObject>& Item);
    TSharedRef<SWidget> PackCard(const TSharedPtr<FJsonObject>& Item);
    TSharedRef<SWidget> Price(const TSharedPtr<FJsonObject>& Item);
    void BuyPack(const TSharedPtr<FJsonObject>& Item);
    void Purchase(const FInvoWalletItem& Item, const FString& RequestId, TSharedRef<SButton> Buy, TSharedRef<STextBlock> BuyLabel, TSharedRef<STextBlock> Warn);
};

/** The four-step Send and Transfer screens: details, review, approve on your phone (QR), done. */
class SInvoMovePage : public SInvoWalletPage
{
public:
    SLATE_BEGIN_ARGS(SInvoMovePage) {}
        SLATE_ARGUMENT(bool, bSend)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx);
    virtual void OnShown() override;

    /** Demo helpers: fill the form, and how many steps to advance on their own (1 = review, 2 = approve). */
    void Prefill(const FString& Phone, const FString& Amount, int32 AutoAdvanceSteps);

private:
    bool bSend = true;
    TArray<TSharedPtr<FJsonObject>> Destinations;
    TSharedPtr<FJsonObject> Game;
    TSharedPtr<FJsonObject> Txn;
    FString TxnAmount, TxnPhone;
    FString RequestId;
    int32 AutoAdvance = 0;
    bool bBusy = false;

    TSharedPtr<SWidgetSwitcher> Steps;
    TSharedPtr<SHorizontalBox> Recent;
    TSharedPtr<SWidget> RecentBox;
    TSharedPtr<SEditableTextBox> PhoneField, AmountField, SearchField;
    TSharedPtr<STextBlock> GameTitle, GameSub, GameBadge, Error1, Error2, ContinueLabel, ReviewAmount, DoneTitle, DoneText, DoneCode;
    TSharedPtr<SVerticalBox> PickerList, ReviewRows;
    TSharedPtr<SWidget> CopyButton;
    TSharedPtr<SInvoApprovalView> Approval;

    void Go(int32 Step);
    TSharedRef<SWidget> BuildDetails();
    TSharedRef<SWidget> BuildPicker();
    TSharedRef<SWidget> BuildReview();
    TSharedRef<SWidget> BuildApprove();
    TSharedRef<SWidget> BuildDone();
    void FillRecent();
    void LoadDestinations();
    void PickGame(const TSharedPtr<FJsonObject>& InGame);
    void OpenPicker();
    void FillPicker();
    void OnContinue();
    void ShowReview();
    void StartApproval();
    void OnApproved(TSharedPtr<FJsonObject> Result, const FString& Message);
    FString PlayerPhone() const;
    TArray<FString> LoadRecent() const;
    void SaveRecent(const FString& Phone) const;
};

/** Collect: sends and transfers waiting for this player, collected with phone approval. */
class SInvoCollectPage : public SInvoWalletPage
{
public:
    SLATE_BEGIN_ARGS(SInvoCollectPage) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx);
    virtual void OnShown() override;

    void Refresh(TFunction<void()> Then = nullptr);
    /** Demo helper: collect the first item that is not on hold, once the list is loaded. */
    void CollectFirst();

private:
    int32 RefreshRun = 0;
    TArray<TSharedPtr<FJsonObject>> Items;
    TSharedPtr<SWidgetSwitcher> Switcher;
    TSharedPtr<STextBlock> Message;
    TSharedPtr<SVerticalBox> List;
    TSharedPtr<SInvoApprovalView> Approval;

    void ShowItems();
    void CollectItem(const TSharedPtr<FJsonObject>& Item);
};

/** The number that approves the player's sends, and changing it (a secured account confirms by email). */
class SInvoPhonePage : public SInvoWalletPage
{
public:
    SLATE_BEGIN_ARGS(SInvoPhonePage) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx);
    virtual void OnShown() override;

private:
    FString RequestId, RequestedPhone;
    TSharedPtr<FActiveTimerHandle> Timer;
    TSharedPtr<SWidgetSwitcher> Switcher;
    TSharedPtr<STextBlock> Current, Error, WaitText, WaitStatus, DoneText;
    TSharedPtr<SEditableTextBox> NewPhone;

    void OnUpdate();
    void RequestChange();
    EActiveTimerReturnType TickPoll(double CurrentTime, float DeltaTime);
    void StopPolling();
    void Finish(const FString& Text);
    static FString Describe(const FInvoError& E);
};
