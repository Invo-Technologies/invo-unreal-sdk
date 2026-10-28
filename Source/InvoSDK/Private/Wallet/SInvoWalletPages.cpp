#include "Wallet/SInvoWalletPages.h"
#include "Wallet/InvoWalletUi.h"
#include "InvoLibrary.h"
#include "InvoQrCode.h"
#include "InvoSubsystem.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/SecureHash.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

using namespace InvoUi;

namespace
{
    constexpr int32 MaxTransientFailures = 5;
    constexpr float Gap = FInvoWalletStyle::Unit * 2;

    FString HoldMessage(const FString& Reason)
    {
        if (Reason == TEXT("GUARDIAN_APPROVAL_PENDING"))
        {
            return TEXT("Approved. It now needs the guardian on file to approve as well; we will finish when they do.");
        }
        if (Reason == TEXT("RISK_HOLD"))
        {
            return TEXT("Approved. Invo is reviewing it before it goes through; nothing else is needed from you.");
        }
        return TEXT("Approved. One more check is running; we will finish as soon as it clears.");
    }

    FString BeginProblem(const FInvoError& E)
    {
        if (E.Code == TEXT("DEVICE_APPROVAL_ALREADY_PENDING")) { return TEXT("This was already approved on a phone and is finishing. Wait a few minutes, then try again."); }
        if (E.Code == TEXT("PASSKEY_RECOVERY_COOLDOWN")) { return TEXT("Your INVO passkey was recently recovered, so sending is paused for a while. Try again later."); }
        if (E.Code == TEXT("DEVICE_APPROVAL_NOT_CONFIGURED") || E.Code == TEXT("TENANT_NOT_MIGRATED")) { return TEXT("Phone approval is not enabled for this game yet. Contact Invo to enable it."); }
        if (E.Code == TEXT("player_not_found")) { return TEXT("Your Invo account was not found in this game."); }
        if (IsNetworkError(E)) { return TEXT("No connection. Check your network and try again."); }
        return TEXT("We could not start the approval. Please try again.");
    }

    bool IsTransientPollFailure(const FInvoError& E)
    {
        return IsNetworkError(E) || UInvoLibrary::IsRateLimited(E) || E.Status >= 500;
    }

    FString Initials(const FString& Name)
    {
        TArray<FString> Words;
        Name.ParseIntoArrayWS(Words);
        FString Out;
        for (const FString& W : Words)
        {
            if (Out.Len() < 2 && !W.IsEmpty())
            {
                Out.AppendChar(FChar::ToUpper(W[0]));
            }
        }
        return Out.IsEmpty() ? TEXT("?") : Out;
    }

    TArray<TSharedPtr<FJsonObject>> ObjectArray(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
    {
        TArray<TSharedPtr<FJsonObject>> Out;
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (Object.IsValid() && Object->TryGetArrayField(Field, Values))
        {
            for (const TSharedPtr<FJsonValue>& V : *Values)
            {
                const TSharedPtr<FJsonObject>* O = nullptr;
                if (V.IsValid() && V->TryGetObject(O) && O)
                {
                    Out.Add(*O);
                }
            }
        }
        return Out;
    }

    /** A one-shot delay on a widget (the timer dies with it). */
    void After(const TSharedRef<SWidget>& Widget, float Seconds, TFunction<void()> Then)
    {
        Widget->RegisterActiveTimer(Seconds, FWidgetActiveTimerDelegate::CreateLambda([Then = MoveTemp(Then)](double, float)
        {
            Then();
            return EActiveTimerReturnType::Stop;
        }));
    }
}

// =================================================================== approval

void SInvoApprovalView::Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx)
{
    Ctx = InCtx;
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().FillHeight(1) [ SNew(SSpacer) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(SummaryText, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary)
            .Justification(ETextJustify::Center).AutoWrapText(true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            Label(TEXT("Scan with your phone to approve"), 16, S().TextPrimary, true, false, ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, Gap)
        [
            SAssignNew(QrFrame, SBorder).BorderImage(&S().QrFrameBrush).Padding(12)
            [
                SNew(SBox).WidthOverride(200).HeightOverride(200)
                [
                    SNew(SImage).Image(&QrBrush)
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(CodeLine, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextMuted).Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(MatchBox, SVerticalBox).Visibility(EVisibility::Collapsed)
            + SVerticalBox::Slot().AutoHeight() [ Label(TEXT("Set up INVO on this phone?"), 18, S().TextPrimary, true, false, ETextJustify::Center) ]
            + SVerticalBox::Slot().AutoHeight()
            [
                SAssignNew(MatchCode, STextBlock).Font(FInvoWalletStyle::Font(44, true)).ColorAndOpacity(S().TextPrimary).Justification(ETextJustify::Center)
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, FInvoWalletStyle::Unit)
            [
                Label(TEXT("Say Yes only if the phone you just opened shows this code."), 14, S().TextSecondary, false, true, ETextJustify::Center)
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, FInvoWalletStyle::Unit, 0) [ Primary(TEXT("Yes, it matches"), [this] { AnswerMatch(true); }) ]
                + SHorizontalBox::Slot().AutoWidth() [ Secondary(TEXT("No, it's different"), [this] { AnswerMatch(false); }) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, Gap)
        [
            SNew(SBorder).BorderImage(&S().PillBrush).Padding(FMargin(14, 6))
            [
                SAssignNew(Status, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary)
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, FInvoWalletStyle::Unit, 0)
            [
                SAssignNew(OpenAgain, SBox).Visibility(EVisibility::Collapsed)
                [
                    Secondary(TEXT("Open again"), [this]
                    {
                        if (ApproveUrl.StartsWith(TEXT("https://")))
                        {
                            FPlatformProcess::LaunchURL(*ApproveUrl, nullptr, nullptr);
                        }
                    })
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth() [ Secondary(TEXT("Cancel"), [this] { Cancel(); }) ]
        ]
        + SVerticalBox::Slot().FillHeight(1) [ SNew(SSpacer) ]
    ];
}

void SInvoApprovalView::Start(const FString& InTransactionId, const FString& InFlow, const FString& Summary)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    StopTimer();
    const int32 ThisRun = ++Run;
    TransactionId = InTransactionId;
    Flow = InFlow;
    DeviceCode.Reset();
    ApproveUrl.Reset();
    QrTexture.Reset();
    QrBrush = FSlateBrush();
    QrBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
    SummaryText->SetText(FText::FromString(Summary));
    CodeLine->SetText(FText::GetEmpty());
    MatchBox->SetVisibility(EVisibility::Collapsed);
    OpenAgain->SetVisibility(EVisibility::Collapsed);
    Status->SetText(FText::FromString(TEXT("Getting a code…")));

    TWeakPtr<SInvoApprovalView> Weak = SharedThis(this);
    Invo->BeginPhoneApprovalNative(TransactionId, Flow, UInvoSubsystem::DefaultApprovalChannel(), [Weak, ThisRun](const FInvoResult& Result)
    {
        TSharedPtr<SInvoApprovalView> Self = Weak.Pin();
        if (!Self.IsValid() || ThisRun != Self->Run)
        {
            return;
        }
        if (!Result.bOk)
        {
            Self->Fail(BeginProblem(Result.Error));
            return;
        }
        const TSharedPtr<FJsonObject> Grant = Result.Data.JsonObject;
        Self->DeviceCode = JsonStr(Grant, TEXT("device_code"));
        Self->ApproveUrl = JsonStr(Grant, TEXT("verification_uri_complete"));
        if (UTexture2D* Qr = UInvoQrCodeLibrary::MakeQrCodeTexture(Self->ApproveUrl, 8, 2))
        {
            Self->QrTexture.Reset(Qr);
            Self->QrBrush.SetResourceObject(Qr);
            Self->QrBrush.ImageSize = FVector2D(200, 200);
            Self->QrBrush.DrawAs = ESlateBrushDrawType::Image;
        }
        FString Uri = JsonStr(Grant, TEXT("verification_uri"));
        Uri.RemoveFromStart(TEXT("https://"));
        Self->CodeLine->SetText(FText::FromString(FString::Printf(TEXT("Or go to %s and enter %s"), *Uri, *JsonStr(Grant, TEXT("user_code")))));
        if (JsonStr(Grant, TEXT("channel")) == TEXT("app_browser"))
        {
            Self->OpenAgain->SetVisibility(EVisibility::Visible);
            if (Self->ApproveUrl.StartsWith(TEXT("https://")))
            {
                FPlatformProcess::LaunchURL(*Self->ApproveUrl, nullptr, nullptr);
            }
        }
        Self->Deadline = FPlatformTime::Seconds() + JsonNum(Grant, TEXT("expires_in"), 600);
        Self->Interval = FMath::Max(2, (int32)JsonNum(Grant, TEXT("interval"), 5));
        Self->Waited = 0;
        Self->Transient = 0;
        Self->bPolling = false;
        Self->Status->SetText(FText::FromString(FString::Printf(TEXT("Waiting for your phone · code expires in %s"), *Self->Clock())));
        if (!Self->Ctx->bDemoHoldApproval)
        {
            Self->Timer = Self->RegisterActiveTimer(1.f, FWidgetActiveTimerDelegate::CreateSP(Self.ToSharedRef(), &SInvoApprovalView::TickPoll));
        }
    });
}

EActiveTimerReturnType SInvoApprovalView::TickPoll(double, float)
{
    if (FPlatformTime::Seconds() >= Deadline)
    {
        Timer.Reset();
        Fail(TEXT("The QR code expired before it was approved. Show a new one to try again."));
        return EActiveTimerReturnType::Stop;
    }
    Status->SetText(FText::FromString(FString::Printf(TEXT("Waiting for your phone · code expires in %s"), *Clock())));
    if (!bPolling && ++Waited >= Interval)
    {
        Poll();
    }
    return EActiveTimerReturnType::Continue;
}

void SInvoApprovalView::Poll()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    bPolling = true;
    const int32 ThisRun = Run;
    TWeakPtr<SInvoApprovalView> Weak = SharedThis(this);
    Invo->PollPhoneApprovalNative(DeviceCode, [Weak, ThisRun](const FInvoResult& Result)
    {
        TSharedPtr<SInvoApprovalView> Self = Weak.Pin();
        if (!Self.IsValid() || ThisRun != Self->Run)
        {
            return;
        }
        Self->bPolling = false;
        Self->Waited = 0;
        if (!Result.bOk)
        {
            if (IsTransientPollFailure(Result.Error) && Self->Transient < MaxTransientFailures)
            {
                ++Self->Transient;
                return;
            }
            Self->Fail(TEXT("We could not confirm the approval. Check your history before trying again."));
            return;
        }
        Self->Transient = 0;
        const TSharedPtr<FJsonObject> P = Result.Data.JsonObject;
        const FString PollStatus = JsonStr(P, TEXT("status"));
        Self->Interval = UInvoSubsystem::NextPollInterval(Self->Interval, PollStatus, (int32)JsonNum(P, TEXT("interval"), 0));
        if (PollStatus == TEXT("approved"))
        {
            Self->StopTimer();
            Self->Status->SetText(FText::FromString(TEXT("Approved. Finishing…")));
            Self->Settle();
        }
        else if (PollStatus == TEXT("access_denied"))
        {
            Self->Fail(TEXT("The approval was declined, so nothing was sent. Try again if that was a mistake."));
        }
        else if (PollStatus == TEXT("expired_token"))
        {
            Self->Fail(TEXT("The QR code expired before it was approved. Show a new one to try again."));
        }
        else
        {
            const TSharedPtr<FJsonObject>* Enrollment = nullptr;
            FString Code;
            if (P.IsValid() && P->TryGetObjectField(TEXT("enrollment"), Enrollment) && Enrollment)
            {
                Code = JsonStr(*Enrollment, TEXT("match_code"));
            }
            Self->MatchCode->SetText(FText::FromString(Code));
            Self->MatchBox->SetVisibility(Code.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
        }
    });
}

void SInvoApprovalView::Settle()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    const int32 ThisRun = Run;
    TWeakPtr<SInvoApprovalView> Weak = SharedThis(this);
    Invo->SettlePhoneApprovalNative(TransactionId, Flow, DeviceCode, [Weak, ThisRun](const FInvoResult& Result)
    {
        TSharedPtr<SInvoApprovalView> Self = Weak.Pin();
        if (!Self.IsValid() || ThisRun != Self->Run)
        {
            return;
        }
        if (!Result.bOk)
        {
            const FString Ref = Result.Error.RequestId.IsEmpty() ? FString() : FString::Printf(TEXT(" (Ref %s)"), *Result.Error.RequestId);
            Self->Fail(IsNetworkError(Result.Error)
                ? FString(TEXT("No connection. If you already approved on your phone, check your history before trying again."))
                : FString(TEXT("We could not confirm the approval. Check your history before trying again.")) + Ref);
            return;
        }
        const TSharedPtr<FJsonObject> R = Result.Data.JsonObject;
        const FString Hold = JsonStr(R, TEXT("hold_reason"));
        if (!Hold.IsEmpty())
        {
            if (Self->OnFinished) { Self->OnFinished(R, HoldMessage(Hold)); }
        }
        else if (JsonStr(R, TEXT("status")) == TEXT("not_pending") && !JsonBool(R, TEXT("already_settled")))
        {
            Self->Fail(TEXT("We could not confirm this. Check your history before trying again."));
        }
        else if (Self->OnFinished)
        {
            Self->OnFinished(R, FString());
        }
    });
}

void SInvoApprovalView::AnswerMatch(bool bApprove)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    MatchBox->SetVisibility(EVisibility::Collapsed);
    TWeakPtr<SInvoApprovalView> Weak = SharedThis(this);
    Invo->ConfirmPhoneEnrollmentNative(DeviceCode, bApprove, [Weak](const FInvoResult& Result)
    {
        TSharedPtr<SInvoApprovalView> Self = Weak.Pin();
        const bool bMoot = Result.Error.Code == TEXT("DEVICE_APPROVAL_NO_ENROLLMENT_PENDING") || Result.Error.Code == TEXT("DEVICE_APPROVAL_ENROLLMENT_ALREADY_DECIDED");
        if (Self.IsValid() && !Result.bOk && !bMoot)
        {
            Self->Status->SetText(FText::FromString(TEXT("We could not send your answer. The next check will show where things stand.")));
        }
    });
}

void SInvoApprovalView::Cancel()
{
    ++Run;
    StopTimer();
    if (OnFailed)
    {
        OnFailed(FString());
    }
}

void SInvoApprovalView::Fail(const FString& Message)
{
    ++Run;
    StopTimer();
    if (OnFailed)
    {
        OnFailed(Message);
    }
}

void SInvoApprovalView::StopTimer()
{
    if (Timer.IsValid())
    {
        UnRegisterActiveTimer(Timer.ToSharedRef());
        Timer.Reset();
    }
}

FString SInvoApprovalView::Clock() const
{
    const int32 Left = FMath::Max(0, (int32)(Deadline - FPlatformTime::Seconds()));
    return FString::Printf(TEXT("%d:%02d"), Left / 60, Left % 60);
}

// =================================================================== store

void SInvoStorePage::Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx)
{
    Ctx = InCtx;
    TSharedPtr<FInvoWalletContext> C = Ctx;
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center) [ Title(TEXT("Store")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary)
                .Text_Lambda([C] { return FText::FromString(TEXT("Balance ") + C->BalanceText()); })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SNew(SBorder).BorderImage(&S().TabBarBrush).Padding(3)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1)
                [
                    SAssignNew(TopUpTab, SButton).ButtonStyle(&S().TabSelected).HAlign(HAlign_Center)
                    .OnClicked_Lambda([this] { ShowSub(false); return FReply::Handled(); })
                    [ Label(TEXT("Top up"), 15, S().TextPrimary, true) ]
                ]
                + SHorizontalBox::Slot().FillWidth(1)
                [
                    SAssignNew(ItemsTab, SButton).ButtonStyle(&S().Tab).HAlign(HAlign_Center)
                    .OnClicked_Lambda([this] { ShowSub(true); return FReply::Handled(); })
                    [ Label(TEXT("Items"), 15, S().TextPrimary, true) ]
                ]
            ]
        ]
        + SVerticalBox::Slot().FillHeight(1)
        [
            SNew(SScrollBox).ScrollBarStyle(&S().ScrollBar)
            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SAssignNew(Message, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary)
                    .AutoWrapText(true).Justification(ETextJustify::Center)
                ]
                + SVerticalBox::Slot().AutoHeight() [ SAssignNew(TopUp, SVerticalBox) ]
                + SVerticalBox::Slot().AutoHeight() [ SAssignNew(ItemsPage, SVerticalBox).Visibility(EVisibility::Collapsed) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, FInvoWalletStyle::Unit, 0, 0)
        [
            SAssignNew(Confirm, SBorder).BorderImage(&S().RaisedBrush).Padding(16).Visibility(EVisibility::Collapsed)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, FInvoWalletStyle::Unit, 0, 0)
        [
            Note(TEXT("Paying opens INVO's secure checkout in your browser. Your balance updates when the payment clears."))
        ]
    ];
}

void SInvoStorePage::OnShown()
{
    Ctx->RefreshBalance();
    if (!bLoaded)
    {
        Load();
    }
}

void SInvoStorePage::ShowSub(bool bInItems)
{
    bItems = bInItems;
    TopUpTab->SetButtonStyle(bItems ? &S().Tab : &S().TabSelected);
    ItemsTab->SetButtonStyle(bItems ? &S().TabSelected : &S().Tab);
    TopUp->SetVisibility(bItems ? EVisibility::Collapsed : EVisibility::Visible);
    ItemsPage->SetVisibility(bItems ? EVisibility::Visible : EVisibility::Collapsed);
    Message->SetText(FText::GetEmpty());
    if (bItems)
    {
        BuildItems();
    }
}

void SInvoStorePage::Load()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    TopUp->ClearChildren();
    Message->SetText(FText::FromString(TEXT("Loading packs...")));
    TWeakPtr<SInvoStorePage> Weak = SharedThis(this);
    Invo->GetGameItemsNative([Weak](const FInvoResult& Result)
    {
        TSharedPtr<SInvoStorePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (!Result.bOk)
        {
            Self->Message->SetText(FText::FromString(StoreCopy(Result.Error)));
            return;
        }
        Self->bLoaded = true;
        TSharedPtr<FJsonObject> Featured;
        TArray<TSharedPtr<FJsonObject>> Weekly, Daily;
        for (const TSharedPtr<FJsonObject>& Item : ObjectArray(Result.Data.JsonObject, TEXT("items")))
        {
            const FString Category = JsonStr(Item, TEXT("item_category"));
            if (!Featured.IsValid() && JsonBool(Item, TEXT("is_featured")))
            {
                Featured = Item;
            }
            else if (Category == TEXT("daily"))
            {
                Daily.Add(Item);
            }
            else if (Category == TEXT("weekly"))
            {
                Weekly.Add(Item);
            }
        }
        if (!Self->bItems)
        {
            Self->Message->SetText(Featured.IsValid() || Weekly.Num() || Daily.Num() ? FText::GetEmpty() : FText::FromString(TEXT("The store is empty right now.")));
        }
        if (Featured.IsValid())
        {
            Self->TopUp->AddSlot().AutoHeight().Padding(0, FInvoWalletStyle::Unit, 0, 6) [ Overline(TEXT("Featured")) ];
            Self->TopUp->AddSlot().AutoHeight() [ Self->HeroCard(Featured) ];
        }
        const TPair<const TCHAR*, TArray<TSharedPtr<FJsonObject>>*> Sections[] = { { TEXT("Packs"), &Weekly }, { TEXT("Daily deals"), &Daily } };
        for (const auto& Section : Sections)
        {
            if (Section.Value->IsEmpty())
            {
                continue;
            }
            Self->TopUp->AddSlot().AutoHeight().Padding(0, Gap, 0, 6) [ Overline(Section.Key) ];
            TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(4));
            for (int32 I = 0; I < Section.Value->Num(); ++I)
            {
                Grid->AddSlot(I % 2, I / 2) [ Self->PackCard((*Section.Value)[I]) ];
            }
            Self->TopUp->AddSlot().AutoHeight() [ Grid ];
        }
    });
}

TSharedRef<SWidget> SInvoStorePage::Price(const TSharedPtr<FJsonObject>& Item)
{
    const double Now = JsonNum(Item, TEXT("price_usd"));
    const double Was = JsonNum(Item, TEXT("original_price_usd"));
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom) [ Label(Usd(Now), 16, S().TextPrimary, true) ];
    if (Was > Now)
    {
        Row->AddSlot().AutoWidth().VAlign(VAlign_Bottom).Padding(6, 0, 0, 1)
        [
            SNew(STextBlock).Text(FText::FromString(Usd(Was))).Font(FInvoWalletStyle::Font(13)).ColorAndOpacity(S().TextMuted)
            .StrikeBrush(&S().StrikeBrush)
        ];
    }
    return Row;
}

TSharedRef<SWidget> SInvoStorePage::HeroCard(const TSharedPtr<FJsonObject>& Item)
{
    const double Discount = JsonNum(Item, TEXT("discount_percentage"), 0);
    FString Name = JsonStr(Item, TEXT("display_name"));
    if (Name.IsEmpty()) { Name = JsonStr(Item, TEXT("item_name")); }
    return Card(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, Gap, 0) [ Gem(72) ]
        + SHorizontalBox::Slot().FillWidth(1)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                Label(Discount > 0 ? FString::Printf(TEXT("-%d%%"), (int32)Discount) : FString(), 13, S().Warning, true)
            ]
            + SVerticalBox::Slot().AutoHeight() [ Heading(Name) ]
            + SVerticalBox::Slot().AutoHeight() [ Caption(JsonStr(Item, TEXT("display_subtitle"))) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center) [ Price(Item) ]
                + SHorizontalBox::Slot().AutoWidth() [ Primary(TEXT("Buy"), [this, Item] { BuyPack(Item); }) ]
            ]
        ],
        &S().CardSelectedBrush, FMargin(12));
}

TSharedRef<SWidget> SInvoStorePage::PackCard(const TSharedPtr<FJsonObject>& Item)
{
    FString Name = JsonStr(Item, TEXT("display_name"));
    if (Name.IsEmpty()) { Name = JsonStr(Item, TEXT("item_name")); }
    return Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 6) [ Gem(48) ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ BodyStrong(Name) ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ Muted(JsonStr(Item, TEXT("display_subtitle"))) ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4) [ Price(Item) ]
        + SVerticalBox::Slot().AutoHeight() [ Primary(TEXT("Buy"), [this, Item] { BuyPack(Item); }) ]);
}

void SInvoStorePage::BuyPack(const TSharedPtr<FJsonObject>& Item)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    FString Email, Phone, Name;
    Invo->GetActivePlayer(Email, Phone, Name);
    if (Email.IsEmpty())
    {
        Message->SetText(FText::FromString(TEXT("Sign in before making a purchase.")));
        return;
    }
    Message->SetText(FText::GetEmpty());
    TWeakPtr<SInvoStorePage> Weak = SharedThis(this);
    Invo->OpenCheckoutNative(UInvoLibrary::FormatMoney(JsonNum(Item, TEXT("price_usd"))), [Weak](const FInvoResult& Result)
    {
        TSharedPtr<SInvoStorePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (Result.bOk)
        {
            Self->Message->SetText(FText::FromString(TEXT("Checkout opened in your browser. Your balance updates when the payment clears.")));
            return;
        }
        Self->Message->SetText(FText::FromString(Result.Error.Code == TEXT("NO_CHECKOUT_ENDPOINT")
            ? TEXT("Real-money purchases need a server endpoint. See the README.")
            : IsNetworkError(Result.Error) ? TEXT("No connection. Check your network and try again.") : TEXT("Checkout could not be started. Please try again.")));
    });
}

TArray<FInvoWalletItem> SInvoStorePage::Catalog() const
{
    UInvoWalletSubsystem* Wallet = Ctx->Wallet.Get();
    TArray<FInvoWalletItem> Items = Wallet ? Wallet->GetItemCatalog() : TArray<FInvoWalletItem>();
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (Items.IsEmpty() && Invo && Invo->IsOfflineMock())
    {
        // Demo items for the Items tab while the offline mock is on (the Godot wallet's MOCK_CATALOG).
        const TCHAR* Demo[][3] = {
            { TEXT("sword_01"), TEXT("Ember Sword"), TEXT("250") },
            { TEXT("shield_01"), TEXT("Oak Shield"), TEXT("120") },
            { TEXT("potion_05"), TEXT("Potion x5"), TEXT("40") },
            { TEXT("skin_gold"), TEXT("Gold Skin"), TEXT("900") },
        };
        for (const auto& D : Demo)
        {
            FInvoWalletItem& Item = Items.AddDefaulted_GetRef();
            Item.ItemId = D[0];
            Item.Name = D[1];
            Item.Price = D[2];
        }
    }
    return Items;
}

void SInvoStorePage::BuildItems()
{
    ItemsPage->ClearChildren();
    const TArray<FInvoWalletItem> Items = Catalog();
    if (Items.IsEmpty())
    {
        Message->SetText(FText::FromString(TEXT("No items are available right now.")));
        return;
    }
    const FString Currency = Ctx->Wallet.IsValid() ? Ctx->Wallet->GetCurrencyName() : FString();
    TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(4));
    for (int32 I = 0; I < Items.Num(); ++I)
    {
        Grid->AddSlot(I % 2, I / 2)
        [
            Card(
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 6) [ Gem(48) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) [ BodyStrong(Items[I].Name) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 2, 0, 6)
                [
                    Caption(FString::Printf(TEXT("%s %s"), *Amount(Items[I].Price), *Currency))
                ]
                + SVerticalBox::Slot().AutoHeight() [ Primary(TEXT("Buy"), [this, I] { ConfirmItem(I); }) ])
        ];
    }
    ItemsPage->AddSlot().AutoHeight().Padding(0, FInvoWalletStyle::Unit, 0, 0) [ Grid ];
}

void SInvoStorePage::ConfirmItem(int32 Index)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    const TArray<FInvoWalletItem> Items = Catalog();
    if (!Invo || !Items.IsValidIndex(Index))
    {
        return;
    }
    FString Email, Phone, Name;
    Invo->GetActivePlayer(Email, Phone, Name);
    if (Email.IsEmpty())
    {
        Message->SetText(FText::FromString(TEXT("Sign in before making a purchase.")));
        return;
    }
    const FInvoWalletItem Item = Items[Index];
    const FString Currency = Ctx->Wallet.IsValid() ? Ctx->Wallet->GetCurrencyName() : FString();
    const FString GameName = Ctx->Wallet.IsValid() ? Ctx->Wallet->GetGameName() : FString();
    // One client_request_id per purchase intent, reused on retry: a retry must not charge twice.
    const FString RequestId = UInvoLibrary::NewClientRequestId();
    TSharedRef<STextBlock> Warn = Danger();
    TSharedRef<STextBlock> BuyLabel = Label(TEXT("Buy"), 16, S().OnAccent, true);
    TSharedRef<SButton> Buy = SNew(SButton).ButtonStyle(&S().Primary).HAlign(HAlign_Center) [ BuyLabel ];
    TWeakPtr<SButton> WeakBuy = Buy;
    Buy->SetOnClicked(FOnClicked::CreateLambda([this, Item, RequestId, WeakBuy, BuyLabel, Warn]
    {
        if (TSharedPtr<SButton> Button = WeakBuy.Pin())
        {
            Purchase(Item, RequestId, Button.ToSharedRef(), BuyLabel, Warn);
        }
        return FReply::Handled();
    }));
    Confirm->SetContent(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, FInvoWalletStyle::Unit) [ Heading(TEXT("Confirm purchase")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ Row(TEXT("Item"), Item.Name) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ Row(TEXT("Price"), FString::Printf(TEXT("%s %s"), *Amount(Item.Price), *Currency)) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ Row(TEXT("Player"), Name.IsEmpty() ? Email : Name) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 2) [ Row(TEXT("Game"), GameName) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0) [ Caption(TEXT("Paid from your in-game balance. This can't be undone.")) ]
        + SVerticalBox::Slot().AutoHeight() [ Warn ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, FInvoWalletStyle::Unit, 0, 0)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, FInvoWalletStyle::Unit, 0)
            [
                Secondary(TEXT("Cancel"), [this] { Confirm->SetVisibility(EVisibility::Collapsed); })
            ]
            + SHorizontalBox::Slot().FillWidth(1) [ Buy ]
        ]);
    Confirm->SetVisibility(EVisibility::Visible);
}

void SInvoStorePage::Purchase(const FInvoWalletItem& Item, const FString& RequestId, TSharedRef<SButton> Buy, TSharedRef<STextBlock> BuyLabel, TSharedRef<STextBlock> Warn)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    if (!UInvoLibrary::IsValidMoney(Item.Price, UInvoLibrary::MaxItemPrice))
    {
        Warn->SetText(FText::FromString(TEXT("This item's price is not set up correctly.")));
        return;
    }
    Buy->SetEnabled(false);
    BuyLabel->SetText(FText::FromString(TEXT("Processing...")));
    Warn->SetText(FText::GetEmpty());
    TWeakPtr<SInvoStorePage> Weak = SharedThis(this);
    Invo->PurchaseItemNative(RequestId, Item.ItemId, Item.Name, 1, Item.Price, Item.Price, [Weak, Buy, BuyLabel, Warn](const FInvoResult& Result)
    {
        TSharedPtr<SInvoStorePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (Result.bOk || UInvoLibrary::IsDuplicateRequest(Result.Error))
        {
            BuyLabel->SetText(FText::FromString(TEXT("Purchase Complete")));
            if (!Result.bOk)
            {
                Warn->SetText(FText::FromString(TEXT("This purchase already went through.")));
            }
            Self->Ctx->NotifyChanged();
            After(Self.ToSharedRef(), 1.2f, [Weak]
            {
                if (TSharedPtr<SInvoStorePage> Page = Weak.Pin())
                {
                    Page->Confirm->SetVisibility(EVisibility::Collapsed);
                }
            });
            return;
        }
        Warn->SetText(FText::FromString(PurchaseCopy(Result.Error)));
        BuyLabel->SetText(FText::FromString(TEXT("Try Again")));
        Buy->SetEnabled(true);
    });
}

// =================================================================== send / transfer

void SInvoMovePage::Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx)
{
    Ctx = InCtx;
    bSend = InArgs._bSend;
    ChildSlot
    [
        SAssignNew(Steps, SWidgetSwitcher)
        + SWidgetSwitcher::Slot() [ BuildDetails() ]
        + SWidgetSwitcher::Slot() [ BuildReview() ]
        + SWidgetSwitcher::Slot() [ BuildApprove() ]
        + SWidgetSwitcher::Slot() [ BuildDone() ]
        + SWidgetSwitcher::Slot() [ BuildPicker() ]
    ];
    Go(0);
}

void SInvoMovePage::Go(int32 Step)
{
    Steps->SetActiveWidgetIndex(Step);
}

FString SInvoMovePage::PlayerPhone() const
{
    FString Email, Phone, Name;
    if (UInvoSubsystem* Invo = Ctx->Invo.Get())
    {
        Invo->GetActivePlayer(Email, Phone, Name);
    }
    return Phone;
}

TSharedRef<SWidget> SInvoMovePage::BuildDetails()
{
    TSharedPtr<FInvoWalletContext> C = Ctx;
    const bool bS = bSend;
    TSharedRef<SVerticalBox> V = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight() [ Title(bSend ? TEXT("Send") : TEXT("Transfer to another game")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, Gap)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary)
                .Text_Lambda([this, bS] { return FText::FromString(FString::Printf(TEXT("%s %s"), bS ? TEXT("From your phone") : TEXT("Approved on your phone"), *MaskedPhone(PlayerPhone()))); })
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0, 0, 0)
            [
                Link(TEXT("Change"), [C] { if (C->ShowPanel) { C->ShowPanel("Phone"); } })
            ]
        ];
    if (bSend)
    {
        V->AddSlot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(RecentBox, SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6) [ Overline(TEXT("Recent")) ]
            + SVerticalBox::Slot().AutoHeight() [ SAssignNew(Recent, SHorizontalBox) ]
        ];
        V->AddSlot().AutoHeight().Padding(0, 0, 0, 4) [ Caption(TEXT("Their phone number")) ];
        V->AddSlot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(PhoneField, SEditableTextBox).Style(&S().FieldStyle).HintText(FText::FromString(TEXT("+15555550123")))
            .OnTextChanged_Lambda([this](const FText&) { FillRecent(); })
        ];
    }
    V->AddSlot().AutoHeight().Padding(0, 0, 0, Gap)
    [
        Card(
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 12, 0)
            [
                SNew(SBorder).BorderImage(&S().BadgeBrush).Padding(FMargin(8, 4))
                [
                    SAssignNew(GameBadge, STextBlock).Text(FText::FromString(TEXT("?"))).Font(FInvoWalletStyle::Font(16, true)).ColorAndOpacity(S().TextPrimary)
                ]
            ]
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SAssignNew(GameTitle, STextBlock).Text(FText::FromString(TEXT("Choose a game"))).Font(FInvoWalletStyle::Font(16, true)).ColorAndOpacity(S().TextPrimary)
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SAssignNew(GameSub, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextMuted)
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) [ Link(TEXT("Change game"), [this] { OpenPicker(); }) ])
    ];
    V->AddSlot().AutoHeight()
    [
        SAssignNew(AmountField, SEditableTextBox).Style(&S().AmountStyle).HintText(FText::FromString(TEXT("0"))).Justification(ETextJustify::Center)
    ];
    V->AddSlot().AutoHeight().Padding(0, 6, 0, 0)
    [
        SNew(STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary).Justification(ETextJustify::Center)
        .Text_Lambda([this, C, bS]
        {
            FString Hint = TEXT("Balance ") + C->BalanceText();
            if (Game.IsValid())
            {
                Hint += FString::Printf(TEXT(" · %s %s"), bS ? TEXT("They receive") : TEXT("You receive"), *JsonStr(Game, TEXT("currency_name")));
            }
            return FText::FromString(Hint);
        })
    ];
    V->AddSlot().AutoHeight().Padding(0, Gap, 0, 0)
    [
        SAssignNew(Error1, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().Danger).AutoWrapText(true).Justification(ETextJustify::Center)
    ];
    V->AddSlot().FillHeight(1) [ SNew(SSpacer) ];
    V->AddSlot().AutoHeight()
    [
        SNew(SButton).ButtonStyle(&S().Primary).HAlign(HAlign_Center)
        .IsEnabled_Lambda([this] { return !bBusy; })
        .OnClicked_Lambda([this] { OnContinue(); return FReply::Handled(); })
        [
            SAssignNew(ContinueLabel, STextBlock).Text(FText::FromString(TEXT("Continue"))).Font(FInvoWalletStyle::Font(16, true)).ColorAndOpacity(S().OnAccent)
        ]
    ];
    return V;
}

void SInvoMovePage::OnShown()
{
    if (Steps->GetActiveWidgetIndex() == 0)
    {
        if (bSend)
        {
            FillRecent();
        }
        Ctx->RefreshBalance();
        if (Destinations.IsEmpty())
        {
            LoadDestinations();
        }
    }
}

TArray<FString> SInvoMovePage::LoadRecent() const
{
    FString Email, Phone, Name;
    if (UInvoSubsystem* Invo = Ctx->Invo.Get())
    {
        Invo->GetActivePlayer(Email, Phone, Name);
    }
    TArray<FString> Out;
    if (GConfig && !Email.IsEmpty())
    {
        FString Joined;
        GConfig->GetString(TEXT("InvoWallet.Recent"), *FMD5::HashAnsiString(*Email), Joined, GGameUserSettingsIni);
        Joined.ParseIntoArray(Out, TEXT(","));
    }
    return Out;
}

void SInvoMovePage::SaveRecent(const FString& Phone) const
{
    FString Email, Mine, Name;
    if (UInvoSubsystem* Invo = Ctx->Invo.Get())
    {
        Invo->GetActivePlayer(Email, Mine, Name);
    }
    if (!GConfig || Email.IsEmpty())
    {
        return;
    }
    TArray<FString> List = LoadRecent();
    List.Remove(Phone);
    List.Insert(Phone, 0);
    List.SetNum(FMath::Min(List.Num(), 3));
    GConfig->SetString(TEXT("InvoWallet.Recent"), *FMD5::HashAnsiString(*Email), *FString::Join(List, TEXT(",")), GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void SInvoMovePage::FillRecent()
{
    if (!Recent.IsValid())
    {
        return;
    }
    Recent->ClearChildren();
    const TArray<FString> List = LoadRecent();
    RecentBox->SetVisibility(List.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
    const FString Typed = PhoneField->GetText().ToString().TrimStartAndEnd();
    for (const FString& Phone : List)
    {
        const bool bSelected = Phone == Typed;
        Recent->AddSlot().FillWidth(1).Padding(0, 0, 6, 0)
        [
            Button(MaskedPhone(Phone), bSelected ? S().ChipSelected : S().Chip, [this, Phone]
            {
                PhoneField->SetText(FText::FromString(Phone));
                FillRecent();
            }, S().TextPrimary, false, 15)
        ];
    }
}

void SInvoMovePage::LoadDestinations()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    TWeakPtr<SInvoMovePage> Weak = SharedThis(this);
    Invo->GetDestinationsNative(bSend ? TEXT("send") : TEXT("transfer"), [Weak](const FInvoResult& Result)
    {
        TSharedPtr<SInvoMovePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (!Result.bOk)
        {
            Self->Error1->SetText(FText::FromString(Self->bSend ? SendCopy(Result.Error) : TransferCopy(Result.Error)));
            return;
        }
        Self->Destinations.Reset();
        UInvoSubsystem* Sdk = Self->Ctx->Invo.Get();
        if (Self->bSend && Sdk && !Sdk->GetGameId().IsEmpty())
        {
            TSharedRef<FJsonObject> Own = MakeShared<FJsonObject>();
            Own->SetStringField(TEXT("game_id"), Sdk->GetGameId());
            Own->SetStringField(TEXT("game_name"), TEXT("Another player in this game"));
            Own->SetStringField(TEXT("currency_name"), Self->Ctx->Wallet.IsValid() ? Self->Ctx->Wallet->GetCurrencyName() : FString());
            Self->Destinations.Add(Own);
        }
        Self->Destinations.Append(ObjectArray(Result.Data.JsonObject, TEXT("available_games")));
        if (!Self->Game.IsValid() && Self->Destinations.Num())
        {
            Self->PickGame(Self->Destinations[0]);
        }
        if (Self->AutoAdvance > 0)
        {
            Self->OnContinue();
        }
    });
}

void SInvoMovePage::PickGame(const TSharedPtr<FJsonObject>& InGame)
{
    Game = InGame;
    const FString Name = JsonStr(Game, TEXT("game_name"));
    GameTitle->SetText(FText::FromString(Name));
    GameSub->SetText(FText::FromString(FString::Printf(TEXT("%s %s"), bSend ? TEXT("They receive") : TEXT("You receive"), *JsonStr(Game, TEXT("currency_name")))));
    GameBadge->SetText(FText::FromString(Initials(Name)));
    Go(0);
}

TSharedRef<SWidget> SInvoMovePage::BuildPicker()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap) [ BackHeader(TEXT("Choose a game"), [this] { Go(0); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(SearchField, SEditableTextBox).Style(&S().FieldStyle).HintText(FText::FromString(TEXT("Search games")))
            .OnTextChanged_Lambda([this](const FText&) { FillPicker(); })
        ]
        + SVerticalBox::Slot().FillHeight(1)
        [
            SNew(SScrollBox).ScrollBarStyle(&S().ScrollBar)
            + SScrollBox::Slot() [ SAssignNew(PickerList, SVerticalBox) ]
        ];
}

void SInvoMovePage::OpenPicker()
{
    Go(4);
    SearchField->SetText(FText::GetEmpty());
    FillPicker();
}

void SInvoMovePage::FillPicker()
{
    PickerList->ClearChildren();
    const FString Query = SearchField->GetText().ToString().TrimStartAndEnd();
    bool bAny = false;
    for (const TSharedPtr<FJsonObject>& G : Destinations)
    {
        const FString Name = JsonStr(G, TEXT("game_name"));
        if (!Query.IsEmpty() && !Name.Contains(Query))
        {
            continue;
        }
        if (!bAny)
        {
            PickerList->AddSlot().AutoHeight().Padding(0, 0, 0, 6) [ Overline(TEXT("Games on INVO")) ];
            bAny = true;
        }
        const bool bSelected = Game.IsValid() && JsonStr(Game, TEXT("game_id")) == JsonStr(G, TEXT("game_id"));
        PickerList->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
        [
            SNew(SButton).ButtonStyle(bSelected ? &S().ChipSelected : &S().Chip).HAlign(HAlign_Left)
            .OnClicked_Lambda([this, G] { PickGame(G); return FReply::Handled(); })
            [
                Label(FString::Printf(TEXT("%s  ·  %s"), *Name, *JsonStr(G, TEXT("currency_name"))), 15, S().TextPrimary)
            ]
        ];
    }
    if (!bAny)
    {
        PickerList->AddSlot().AutoHeight() [ Caption(FString::Printf(TEXT("No games match \"%s\"."), *Query)) ];
    }
}

void SInvoMovePage::OnContinue()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo || bBusy)
    {
        return;
    }
    Error1->SetText(FText::GetEmpty());
    FString Email, Mine, Name;
    Invo->GetActivePlayer(Email, Mine, Name);
    const FString AmountText = AmountField->GetText().ToString().TrimStartAndEnd();
    const FString Phone = bSend ? PhoneField->GetText().ToString().TrimStartAndEnd() : Mine;
    auto Problem = [this](const FString& Text) { Error1->SetText(FText::FromString(Text)); AutoAdvance = 0; };
    if (Email.IsEmpty()) { Problem(TEXT("Sign in first.")); return; }
    if (!Game.IsValid()) { Problem(TEXT("Choose a game first.")); return; }
    if (bSend && !UInvoLibrary::IsE164Phone(Phone)) { Problem(TEXT("Enter their phone number with the country code, like +15555550123.")); return; }
    if (!UInvoLibrary::IsValidMoney(AmountText, UInvoLibrary::MaxItemPrice)) { Problem(TEXT("Enter an amount.")); return; }
    const double Value = FCString::Atod(*AmountText);
    const FString Lo = JsonStr(Game, TEXT("minimum_transfer"));
    const FString Hi = JsonStr(Game, TEXT("maximum_transfer"));
    if (Lo.IsNumeric() && Value < FCString::Atod(*Lo))
    {
        Problem(FString::Printf(TEXT("The smallest amount for %s is %s."), *JsonStr(Game, TEXT("game_name")), *Amount(Lo)));
        return;
    }
    if (Hi.IsNumeric() && FCString::Atod(*Hi) > 0 && Value > FCString::Atod(*Hi))
    {
        Problem(FString::Printf(TEXT("The largest amount for %s is %s."), *JsonStr(Game, TEXT("game_name")), *Amount(Hi)));
        return;
    }
    if (!Invo->IsCommerceReady())
    {
        Problem(TEXT("This build is not connected to its game server."));
        return;
    }
    if (RequestId.IsEmpty())
    {
        RequestId = UInvoLibrary::NewClientRequestId();
    }
    bBusy = true;
    ContinueLabel->SetText(FText::FromString(TEXT("Checking…")));
    TWeakPtr<SInvoMovePage> Weak = SharedThis(this);
    auto OnDone = [Weak, AmountText, Phone](const FInvoResult& Result)
    {
        TSharedPtr<SInvoMovePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        Self->bBusy = false;
        Self->ContinueLabel->SetText(FText::FromString(TEXT("Continue")));
        if (!Result.bOk)
        {
            Self->Error1->SetText(FText::FromString(Self->bSend ? SendCopy(Result.Error) : TransferCopy(Result.Error)));
            Self->AutoAdvance = 0;
            return;
        }
        Self->Txn = Result.Data.JsonObject;
        Self->TxnAmount = AmountText;
        Self->TxnPhone = Phone;
        if (Self->bSend)
        {
            Self->SaveRecent(Phone);
        }
        Self->ShowReview();
    };
    const FString GameId = JsonStr(Game, TEXT("game_id"));
    if (bSend)
    {
        Invo->InitiateSendNative(RequestId, Phone, GameId, AmountText, OnDone);
    }
    else
    {
        Invo->InitiateTransferNative(RequestId, GameId, AmountText, OnDone);
    }
}

TSharedRef<SWidget> SInvoMovePage::BuildReview()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap) [ BackHeader(TEXT("Review"), [this] { Go(0); }) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            Label(bSend ? TEXT("You're sending") : TEXT("You're moving"), 14, S().TextSecondary, false, false, ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, Gap)
        [
            SAssignNew(ReviewAmount, STextBlock).Font(FInvoWalletStyle::Font(44, true)).ColorAndOpacity(S().TextPrimary).Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap) [ Card(SAssignNew(ReviewRows, SVerticalBox)) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            Caption(bSend ? TEXT("Check the number. Once you approve on your phone, a send can't be undone.")
                          : TEXT("You approve on your phone. A transfer can't be undone once approved."), true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, FInvoWalletStyle::Unit, 0, 0)
        [
            SAssignNew(Error2, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().Danger).AutoWrapText(true)
        ]
        + SVerticalBox::Slot().FillHeight(1) [ SNew(SSpacer) ]
        + SVerticalBox::Slot().AutoHeight() [ Primary(TEXT("Approve on my phone"), [this] { StartApproval(); }) ];
}

void SInvoMovePage::ShowReview()
{
    Go(1);
    Error2->SetText(FText::GetEmpty());
    const FString Currency = Ctx->Wallet.IsValid() ? Ctx->Wallet->GetCurrencyName() : FString();
    ReviewAmount->SetText(FText::FromString(FString::Printf(TEXT("%s %s"), *Amount(TxnAmount), *Currency)));
    ReviewRows->ClearChildren();
    TArray<TPair<FString, FString>> Rows = { { TEXT("From"), MaskedPhone(PlayerPhone()) } };
    if (bSend)
    {
        Rows.Add({ TEXT("To"), MaskedPhone(TxnPhone) });
    }
    Rows.Add({ TEXT("Game"), JsonStr(Game, TEXT("game_name")) });
    for (const TCHAR* Key : { TEXT("fee_amount"), TEXT("fee"), TEXT("total_fee") })
    {
        const FString Fee = JsonStr(Txn, Key);
        if (!Fee.IsEmpty())
        {
            Rows.Add({ TEXT("Fee"), FString::Printf(TEXT("%s %s"), *Amount(Fee), *Currency) });
            break;
        }
    }
    for (const TCHAR* Key : { TEXT("net_amount"), TEXT("amount_received"), TEXT("receiver_amount") })
    {
        const FString Net = JsonStr(Txn, Key);
        if (!Net.IsEmpty())
        {
            Rows.Add({ bSend ? TEXT("They receive") : TEXT("You receive"), FString::Printf(TEXT("%s %s"), *Amount(Net), *JsonStr(Game, TEXT("currency_name"))) });
            break;
        }
    }
    for (const TPair<FString, FString>& R : Rows)
    {
        ReviewRows->AddSlot().AutoHeight().Padding(0, 3) [ Row(R.Key, R.Value) ];
    }
    if (AutoAdvance > 1)
    {
        AutoAdvance = 0;
        StartApproval();
    }
    else
    {
        AutoAdvance = 0;
    }
}

TSharedRef<SWidget> SInvoMovePage::BuildApprove()
{
    SAssignNew(Approval, SInvoApprovalView, Ctx.ToSharedRef());
    TWeakPtr<SInvoMovePage> Weak = SharedThis(this);
    Approval->OnFinished = [Weak](TSharedPtr<FJsonObject> Result, const FString& Message)
    {
        if (TSharedPtr<SInvoMovePage> Self = Weak.Pin())
        {
            Self->OnApproved(Result, Message);
        }
    };
    Approval->OnFailed = [Weak](const FString& Message)
    {
        if (TSharedPtr<SInvoMovePage> Self = Weak.Pin())
        {
            Self->ShowReview();
            Self->Error2->SetText(FText::FromString(Message));
        }
    };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight() [ BackHeader(TEXT("Approve on your phone"), [this] { Approval->Cancel(); }) ]
        + SVerticalBox::Slot().FillHeight(1) [ Approval.ToSharedRef() ];
}

void SInvoMovePage::StartApproval()
{
    Go(2);
    const FString Currency = Ctx->Wallet.IsValid() ? Ctx->Wallet->GetCurrencyName() : FString();
    const FString Summary = bSend
        ? FString::Printf(TEXT("Sending %s %s to %s in %s"), *Amount(TxnAmount), *Currency, *MaskedPhone(TxnPhone), *JsonStr(Game, TEXT("game_name")))
        : FString::Printf(TEXT("Moving %s %s to %s"), *Amount(TxnAmount), *Currency, *JsonStr(Game, TEXT("game_name")));
    Approval->Start(JsonStr(Txn, TEXT("transaction_id")), bSend ? TEXT("send") : TEXT("transfer"), Summary);
}

TSharedRef<SWidget> SInvoMovePage::BuildDone()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().FillHeight(1) [ SNew(SSpacer) ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, Gap)
        [
            SNew(SBox).WidthOverride(56).HeightOverride(56)
            [
                SNew(SBorder).BorderImage(&S().SuccessCircleBrush).HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    Label(TEXT("✓"), 26, S().Success, true)
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SAssignNew(DoneTitle, STextBlock).Font(FInvoWalletStyle::Font(26, true)).ColorAndOpacity(S().TextPrimary).Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, FInvoWalletStyle::Unit)
        [
            SAssignNew(DoneText, STextBlock).Font(FInvoWalletStyle::Font(16)).ColorAndOpacity(S().TextPrimary).AutoWrapText(true).Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
        [
            SAssignNew(DoneCode, STextBlock).Font(FInvoWalletStyle::Font(44, true)).ColorAndOpacity(S().TextPrimary).Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, FInvoWalletStyle::Unit, 0)
            [
                SAssignNew(CopyButton, SBox)
                [
                    Secondary(TEXT("Copy code"), [this] { FPlatformApplicationMisc::ClipboardCopy(*DoneCode->GetText().ToString()); })
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                Primary(TEXT("Done"), [this]
                {
                    AmountField->SetText(FText::GetEmpty());
                    Go(0);
                    OnShown();
                })
            ]
        ]
        + SVerticalBox::Slot().FillHeight(1) [ SNew(SSpacer) ];
}

void SInvoMovePage::OnApproved(TSharedPtr<FJsonObject> Result, const FString& Message)
{
    RequestId.Reset();
    Go(3);
    const FString Code = JsonStr(Result, TEXT("claim_code"));
    const FString Currency = Ctx->Wallet.IsValid() ? Ctx->Wallet->GetCurrencyName() : FString();
    const FString GameName = JsonStr(Game, TEXT("game_name"));
    if (bSend)
    {
        DoneTitle->SetText(FText::FromString(TEXT("Sent")));
        DoneText->SetText(FText::FromString(!Message.IsEmpty() ? Message
            : FString::Printf(TEXT("%s %s is waiting for %s in %s."), *Amount(TxnAmount), *Currency, *MaskedPhone(TxnPhone), *GameName)));
    }
    else
    {
        DoneTitle->SetText(FText::FromString(Message.IsEmpty() ? TEXT("Transfer complete") : TEXT("Transfer approved")));
        DoneText->SetText(FText::FromString(!Message.IsEmpty() ? Message : FString::Printf(TEXT("Collect it in %s."), *GameName)));
    }
    DoneCode->SetText(FText::FromString(Code));
    DoneCode->SetVisibility(Code.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
    CopyButton->SetVisibility(Code.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
    Ctx->NotifyChanged();
}

void SInvoMovePage::Prefill(const FString& Phone, const FString& InAmount, int32 AutoAdvanceSteps)
{
    Go(0);
    if (PhoneField.IsValid())
    {
        PhoneField->SetText(FText::FromString(Phone));
    }
    AmountField->SetText(FText::FromString(InAmount));
    AutoAdvance = AutoAdvanceSteps;
    if (AutoAdvance > 0 && !Destinations.IsEmpty())
    {
        OnContinue();
    }
}

// =================================================================== collect

void SInvoCollectPage::Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx)
{
    Ctx = InCtx;
    SAssignNew(Approval, SInvoApprovalView, InCtx);
    TWeakPtr<SInvoCollectPage> Weak = SharedThis(this);
    Approval->OnFinished = [Weak](TSharedPtr<FJsonObject>, const FString& Text)
    {
        if (TSharedPtr<SInvoCollectPage> Self = Weak.Pin())
        {
            Self->Switcher->SetActiveWidgetIndex(0);
            Self->Ctx->NotifyChanged();
            Self->Refresh([Weak, Text]
            {
                if (TSharedPtr<SInvoCollectPage> Page = Weak.Pin())
                {
                    Page->Message->SetText(FText::FromString(Text.IsEmpty() ? TEXT("Added to your balance.") : Text));
                }
            });
        }
    };
    Approval->OnFailed = [Weak](const FString& Text)
    {
        if (TSharedPtr<SInvoCollectPage> Self = Weak.Pin())
        {
            Self->Switcher->SetActiveWidgetIndex(0);
            Self->Refresh([Weak, Text]
            {
                TSharedPtr<SInvoCollectPage> Page = Weak.Pin();
                if (Page.IsValid() && !Text.IsEmpty())
                {
                    Page->Message->SetText(FText::FromString(Text));
                }
            });
        }
    };
    ChildSlot
    [
        SAssignNew(Switcher, SWidgetSwitcher)
        + SWidgetSwitcher::Slot()
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, FInvoWalletStyle::Unit) [ Title(TEXT("Collect")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
            [
                SAssignNew(Message, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary).AutoWrapText(true)
            ]
            + SVerticalBox::Slot().FillHeight(1)
            [
                SNew(SScrollBox).ScrollBarStyle(&S().ScrollBar)
                + SScrollBox::Slot() [ SAssignNew(List, SVerticalBox) ]
            ]
        ]
        + SWidgetSwitcher::Slot()
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight() [ BackHeader(TEXT("Approve on your phone"), [this] { Approval->Cancel(); }) ]
            + SVerticalBox::Slot().FillHeight(1) [ Approval.ToSharedRef() ]
        ]
    ];
}

void SInvoCollectPage::OnShown()
{
    if (Switcher->GetActiveWidgetIndex() == 0)
    {
        Refresh();
    }
}

void SInvoCollectPage::Refresh(TFunction<void()> Then)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    const int32 Run = ++RefreshRun;
    if (!Invo || !Invo->HasPlayerToken())
    {
        Items.Reset();
        ShowItems();
        return;
    }
    TWeakPtr<SInvoCollectPage> Weak = SharedThis(this);
    Invo->GetPendingCollectNative([Weak, Run, Then = MoveTemp(Then)](const FInvoResult& Result)
    {
        TSharedPtr<SInvoCollectPage> Self = Weak.Pin();
        // Only the newest refresh draws; an older one finishing late would duplicate rows.
        if (!Self.IsValid() || Run != Self->RefreshRun)
        {
            return;
        }
        if (!Result.bOk)
        {
            Self->Message->SetText(FText::FromString(TransferCopy(Result.Error, TEXT("Your pending items could not be loaded. Please try again."))));
            return;
        }
        Self->Items = ObjectArray(Result.Data.JsonObject, TEXT("pending"));
        Self->ShowItems();
        if (Then)
        {
            Then();
        }
    });
}

void SInvoCollectPage::ShowItems()
{
    List->ClearChildren();
    if (Items.IsEmpty())
    {
        Message->SetText(FText::FromString(TEXT("Nothing is waiting for you right now.")));
        List->AddSlot().AutoHeight()
        [
            Note(TEXT("Expecting something that isn't here? If you don't play this game yet, INVO emailed you a link to collect it."))
        ];
        return;
    }
    Message->SetText(FText::FromString(TEXT("Approve on your phone to add it to your balance.")));
    for (const TSharedPtr<FJsonObject>& Item : Items)
    {
        const FString From = JsonStr(Item, TEXT("counterparty_game"));
        TSharedRef<SWidget> Action = JsonBool(Item, TEXT("held"))
            ? StaticCastSharedRef<SWidget>(Label(TEXT("On hold"), 14, S().Warning, true))
            : StaticCastSharedRef<SWidget>(Primary(TEXT("Collect"), [this, Item] { CollectItem(Item); }));
        List->AddSlot().AutoHeight().Padding(0, 0, 0, FInvoWalletStyle::Unit)
        [
            Card(
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        BodyStrong(FString::Printf(TEXT("%s %s"), *Amount(JsonStr(Item, TEXT("amount"))), *JsonStr(Item, TEXT("currency"))))
                    ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Muted(From.IsEmpty() ? TEXT("From another player") : TEXT("From ") + From)
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) [ Action ])
        ];
    }
}

void SInvoCollectPage::CollectItem(const TSharedPtr<FJsonObject>& Item)
{
    Switcher->SetActiveWidgetIndex(1);
    const FString Flow = JsonStr(Item, TEXT("flow")) == TEXT("transfer") ? TEXT("transfer_receipt") : TEXT("send_receipt");
    Approval->Start(JsonStr(Item, TEXT("transfer_id")), Flow,
        FString::Printf(TEXT("Collecting %s %s"), *Amount(JsonStr(Item, TEXT("amount"))), *JsonStr(Item, TEXT("currency"))));
}

void SInvoCollectPage::CollectFirst()
{
    TWeakPtr<SInvoCollectPage> Weak = SharedThis(this);
    Refresh([Weak]
    {
        TSharedPtr<SInvoCollectPage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        for (const TSharedPtr<FJsonObject>& Item : Self->Items)
        {
            if (!JsonBool(Item, TEXT("held")))
            {
                Self->CollectItem(Item);
                return;
            }
        }
    });
}

// =================================================================== phone

void SInvoPhonePage::Construct(const FArguments& InArgs, TSharedRef<FInvoWalletContext> InCtx)
{
    Ctx = InCtx;
    TSharedPtr<FInvoWalletContext> C = Ctx;
    auto Back = [C] { if (C->ShowPanel) { C->ShowPanel("Send"); } };
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap) [ BackHeader(TEXT("Your phone number"), Back) ]
        + SVerticalBox::Slot().FillHeight(1)
        [
            SAssignNew(Switcher, SWidgetSwitcher)
            + SWidgetSwitcher::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
                [
                    Card(
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight() [ Muted(TEXT("Current")) ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SAssignNew(Current, STextBlock).Font(FInvoWalletStyle::Font(18, true)).ColorAndOpacity(S().TextPrimary)
                            ]
                        ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) [ Caption(TEXT("Approves your sends")) ])
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4) [ Caption(TEXT("New number")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, FInvoWalletStyle::Unit)
                [
                    SAssignNew(NewPhone, SEditableTextBox).Style(&S().FieldStyle).HintText(FText::FromString(TEXT("+15555550123")))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, FInvoWalletStyle::Unit)
                [
                    SAssignNew(Error, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().Danger).AutoWrapText(true)
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
                [
                    Note(TEXT("To keep your account safe, INVO may email you a link to confirm the change. No text message is sent."))
                ]
                + SVerticalBox::Slot().AutoHeight() [ Primary(TEXT("Update number"), [this] { OnUpdate(); }) ]
            ]
            + SWidgetSwitcher::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap) [ Heading(TEXT("Check your email")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
                [
                    SAssignNew(WaitText, STextBlock).Font(FInvoWalletStyle::Font(16)).ColorAndOpacity(S().TextPrimary).AutoWrapText(true)
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
                [
                    SAssignNew(WaitStatus, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().TextSecondary).AutoWrapText(true)
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, FInvoWalletStyle::Unit, 0)
                    [
                        Secondary(TEXT("Resend email"), [this] { StopPolling(); RequestChange(); })
                    ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        // INVO has no cancel route: an unconfirmed request simply expires.
                        Secondary(TEXT("Cancel change"), [this]
                        {
                            StopPolling();
                            Switcher->SetActiveWidgetIndex(0);
                            Error->SetText(FText::FromString(TEXT("Change not made. The emailed link stops working when it expires.")));
                        })
                    ]
                ]
            ]
            + SWidgetSwitcher::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, Gap)
                [
                    SAssignNew(DoneText, STextBlock).Font(FInvoWalletStyle::Font(16)).ColorAndOpacity(S().TextPrimary).AutoWrapText(true)
                ]
                + SVerticalBox::Slot().AutoHeight() [ Primary(TEXT("Done"), Back) ]
            ]
        ]
    ];
}

void SInvoPhonePage::OnShown()
{
    FString Email, Phone, Name;
    if (UInvoSubsystem* Invo = Ctx->Invo.Get())
    {
        Invo->GetActivePlayer(Email, Phone, Name);
    }
    Current->SetText(FText::FromString(MaskedPhone(Phone)));
    if (Switcher->GetActiveWidgetIndex() == 1)
    {
        return;
    }
    NewPhone->SetText(FText::GetEmpty());
    Error->SetText(FText::GetEmpty());
    Switcher->SetActiveWidgetIndex(0);
}

void SInvoPhonePage::OnUpdate()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    const FString Phone = NewPhone->GetText().ToString().TrimStartAndEnd();
    Error->SetText(FText::GetEmpty());
    if (!UInvoLibrary::IsE164Phone(Phone))
    {
        Error->SetText(FText::FromString(TEXT("That doesn't look like a valid phone number. Include the country code.")));
        return;
    }
    TWeakPtr<SInvoPhonePage> Weak = SharedThis(this);
    Invo->ChangePlayerPhoneNative(Phone, [Weak, Phone](const FInvoResult& Result)
    {
        TSharedPtr<SInvoPhonePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (Result.bOk)
        {
            Self->Finish(JsonStr(Result.Data.JsonObject, TEXT("status")) == TEXT("unchanged")
                ? FString(TEXT("That's already your number."))
                : FString::Printf(TEXT("Done. Your sends are now approved on %s."), *MaskedPhone(Phone)));
            return;
        }
        if (UInvoLibrary::IsPhoneChangeRequiresVerification(Result.Error))
        {
            Self->RequestedPhone = Phone;
            Self->RequestChange();
            return;
        }
        Self->Error->SetText(FText::FromString(Describe(Result.Error)));
    });
}

void SInvoPhonePage::RequestChange()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    TWeakPtr<SInvoPhonePage> Weak = SharedThis(this);
    Invo->CreatePhoneChangeRequestNative(RequestedPhone, [Weak](const FInvoResult& Result)
    {
        TSharedPtr<SInvoPhonePage> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (!Result.bOk)
        {
            Self->Switcher->SetActiveWidgetIndex(0);
            Self->Error->SetText(FText::FromString(Describe(Result.Error)));
            return;
        }
        Self->RequestId = JsonStr(Result.Data.JsonObject, TEXT("change_request_id"));
        Self->WaitText->SetText(FText::FromString(FString::Printf(
            TEXT("We sent a link to your email. Open it and confirm on INVO's page to switch to %s."), *MaskedPhone(Self->RequestedPhone))));
        Self->WaitStatus->SetText(FText::FromString(TEXT("Waiting for you to confirm")));
        Self->Switcher->SetActiveWidgetIndex(1);
        Self->StopPolling();
        Self->Timer = Self->RegisterActiveTimer(5.f, FWidgetActiveTimerDelegate::CreateSP(Self.ToSharedRef(), &SInvoPhonePage::TickPoll));
    });
}

EActiveTimerReturnType SInvoPhonePage::TickPoll(double, float)
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return EActiveTimerReturnType::Stop;
    }
    TWeakPtr<SInvoPhonePage> Weak = SharedThis(this);
    const FString Id = RequestId;
    Invo->GetPhoneChangeRequestNative(Id, [Weak, Id](const FInvoResult& Result)
    {
        TSharedPtr<SInvoPhonePage> Self = Weak.Pin();
        if (!Self.IsValid() || Id != Self->RequestId || !Result.bOk || !Self->Timer.IsValid())
        {
            return;
        }
        const FString Status = JsonStr(Result.Data.JsonObject, TEXT("status"));
        if (Status == TEXT("confirmed"))
        {
            if (UInvoSubsystem* Sdk = Self->Ctx->Invo.Get())
            {
                FString Email, Phone, Name;
                Sdk->GetActivePlayer(Email, Phone, Name);
                Sdk->SetActivePlayer(Email, Self->RequestedPhone, Name);
            }
            Self->Finish(FString::Printf(TEXT("Confirmed. Your sends are now approved on %s."), *MaskedPhone(Self->RequestedPhone)));
        }
        else if (Status != TEXT("pending") && Status != TEXT("pending_confirmation"))
        {
            Self->StopPolling();
            Self->WaitStatus->SetText(FText::FromString(
                Status == TEXT("expired") ? TEXT("The link expired before it was confirmed. Nothing changed.")
                : Status == TEXT("cancelled") ? TEXT("A newer request replaced this one.")
                : Status == TEXT("denied") ? TEXT("The change was declined. Your number is unchanged.")
                : TEXT("Change not made. The emailed link stops working when it expires.")));
        }
    });
    return EActiveTimerReturnType::Continue;
}

void SInvoPhonePage::StopPolling()
{
    if (Timer.IsValid())
    {
        UnRegisterActiveTimer(Timer.ToSharedRef());
        Timer.Reset();
    }
}

void SInvoPhonePage::Finish(const FString& Text)
{
    StopPolling();
    DoneText->SetText(FText::FromString(Text));
    FString Email, Phone, Name;
    if (UInvoSubsystem* Invo = Ctx->Invo.Get())
    {
        Invo->GetActivePlayer(Email, Phone, Name);
    }
    Current->SetText(FText::FromString(MaskedPhone(Phone)));
    Switcher->SetActiveWidgetIndex(2);
}

FString SInvoPhonePage::Describe(const FInvoError& E)
{
    if (E.Code == TEXT("INVALID_PHONE") || E.Code == TEXT("INVALID_INPUT")) { return TEXT("That doesn't look like a valid phone number. Include the country code."); }
    if (E.Code == TEXT("PHONE_SHARE_APPROVAL_REQUIRED")) { return TEXT("That number belongs to another INVO account."); }
    if (E.Code == TEXT("PENDING_TRANSFERS_TO_OLD_PHONE")) { return TEXT("Collect what's waiting for your current number first, then change it."); }
    if (E.Code == TEXT("PENDING_TRANSFERS_TO_NEW_PHONE")) { return TEXT("Currency is waiting to be collected on that number. Collect it first."); }
    if (E.Code == TEXT("PHONE_UPDATE_UNAVAILABLE")) { return TEXT("Changing numbers is unavailable right now. Try again later."); }
    if (E.Code == TEXT("player_not_found")) { return TEXT("Make a send or transfer first; then you can change your number."); }
    if (IsNetworkError(E)) { return TEXT("No connection. Check your network and try again."); }
    return TEXT("We could not change your number. Try again in a moment.");
}
