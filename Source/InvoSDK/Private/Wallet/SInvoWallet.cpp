#include "Wallet/SInvoWallet.h"
#include "Wallet/InvoWalletUi.h"
#include "Wallet/SInvoWalletPages.h"
#include "InvoLibrary.h"
#include "InvoSubsystem.h"
#include "InvoWallet.h"
#include "Dom/JsonObject.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SOverlay.h"

using namespace InvoUi;

// =================================================================== context

FString FInvoWalletContext::BalanceText() const
{
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!LastBalance.IsValid() || !LastBalance->TryGetArrayField(TEXT("balances"), Rows))
    {
        return TEXT("-");
    }
    const FString Currency = Wallet.IsValid() ? Wallet->GetCurrencyName() : FString();
    if (Rows->IsEmpty())
    {
        return FString::Printf(TEXT("0 %s"), *Currency).TrimEnd();
    }
    const TSharedPtr<FJsonObject>* Row = nullptr;
    if (!(*Rows)[0]->TryGetObject(Row) || !Row)
    {
        return TEXT("-");
    }
    FString Name = JsonStr(*Row, TEXT("currency_name"));
    if (Name.IsEmpty())
    {
        Name = Currency;
    }
    return FString::Printf(TEXT("%s %s"), *Amount(JsonStr(*Row, TEXT("available_balance"))), *Name).TrimEnd();
}

void FInvoWalletContext::RefreshBalance()
{
    UInvoSubsystem* Sdk = Invo.Get();
    if (!Sdk || !Sdk->HasPlayerToken())
    {
        return;
    }
    TWeakPtr<FInvoWalletContext> Weak = AsWeak();
    Sdk->GetBalanceNative([Weak](const FInvoResult& Result)
    {
        TSharedPtr<FInvoWalletContext> Self = Weak.Pin();
        if (Self.IsValid() && Result.bOk)
        {
            Self->LastBalance = Result.Data.JsonObject;
        }
    });
}

void FInvoWalletContext::NotifyChanged()
{
    RefreshBalance();
    if (UInvoWalletSubsystem* W = Wallet.Get())
    {
        W->NotifyWalletChanged();
    }
}

// =================================================================== sheet

void SInvoWallet::Construct(const FArguments& InArgs)
{
    Ctx = MakeShared<FInvoWalletContext>();
    Ctx->Invo = InArgs._Invo;
    Ctx->Wallet = InArgs._Wallet;
    OnCloseRequested = InArgs._OnCloseRequested;
    TWeakPtr<SInvoWallet> WeakSelf = SharedThis(this);
    Ctx->ShowPanel = [WeakSelf](FName Panel)
    {
        if (TSharedPtr<SInvoWallet> Self = WeakSelf.Pin())
        {
            Self->ShowPanel(Panel);
        }
    };

    UInvoSubsystem* Invo = Ctx->Invo.Get();
    FString Mark;
#if !UE_BUILD_SHIPPING
    if (Invo && Invo->IsOfflineMock())
    {
        Mark = TEXT("OFFLINE DEMO");
    }
    else if (Invo && Invo->GetEnvironment() == EInvoEnvironment::Sandbox)
    {
        Mark = TEXT("SANDBOX");
    }
#endif

    ChildSlot
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBorder).BorderImage(&S().BackdropBrush)
            .OnMouseButtonDown_Lambda([](const FGeometry&, const FPointerEvent&) { return FReply::Handled(); })
        ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(FInvoWalletStyle::SheetWidth).HeightOverride(FInvoWalletStyle::SheetHeight)
            [
                SNew(SBorder).BorderImage(&S().SheetBrush).Padding(16)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12) [ BuildHeader() ]
                    + SVerticalBox::Slot().FillHeight(1)
                    [
                        SAssignNew(RootSwitcher, SWidgetSwitcher)
                        + SWidgetSwitcher::Slot() [ BuildSignIn() ]
                        + SWidgetSwitcher::Slot() [ SAssignNew(PageSwitcher, SWidgetSwitcher) ]
                    ]
                ]
            ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(16, 0, 0, 24)
        [
            Label(Mark, 15, S().Warning, true)
        ]
    ];

    TSharedRef<FInvoWalletContext> C = Ctx.ToSharedRef();
    AddPage("Store", SAssignNew(Store, SInvoStorePage, C));
    AddPage("Send", SAssignNew(Send, SInvoMovePage, C).bSend(true));
    AddPage("Transfer", SAssignNew(Transfer, SInvoMovePage, C).bSend(false));
    AddPage("Collect", SAssignNew(Collect, SInvoCollectPage, C));
    AddPage("Phone", SNew(SInvoPhonePage, C));
}

void SInvoWallet::AddPage(FName Id, TSharedRef<SInvoWalletPage> Page)
{
    PageSwitcher->AddSlot() [ Page ];
    Pages.Add(Id, Page);
    PageOrder.Add(Id);
}

TSharedRef<SWidget> SInvoWallet::BuildHeader()
{
    TSharedRef<SHorizontalBox> Tabs = SNew(SHorizontalBox);
    for (const TCHAR* Id : { TEXT("Store"), TEXT("Send"), TEXT("Transfer"), TEXT("Collect") })
    {
        const FName Panel(Id);
        TSharedPtr<SButton> Tab;
        Tabs->AddSlot().AutoWidth()
        [
            SAssignNew(Tab, SButton)
            .ButtonStyle(&S().Tab)
            .OnClicked_Lambda([this, Panel] { ShowPanel(Panel); return FReply::Handled(); })
            [
                SNew(STextBlock).Text(FText::FromString(Id)).Font(FInvoWalletStyle::Font(15, true))
                .ColorAndOpacity_Lambda([this, Panel] { return FSlateColor(Current == Panel ? S().TextPrimary : S().TextSecondary); })
            ]
        ];
        TabButtons.Add(Panel, Tab);
    }
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()
        [
            SNew(SBorder).BorderImage(&S().TabBarBrush).Padding(3) [ Tabs ]
        ]
        + SHorizontalBox::Slot().FillWidth(1) [ SNew(SSpacer) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, FInvoWalletStyle::Unit, 0)
        [
            PhoneButton([this] { ShowPanel("Phone"); })
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            IconButton(TEXT("×"), TEXT("Close"), [this] { OnCloseRequested.ExecuteIfBound(); })
        ];
}

TSharedRef<SWidget> SInvoWallet::BuildSignIn()
{
    bool bSandboxForm = false;
#if !UE_BUILD_SHIPPING
    if (UInvoSubsystem* Invo = Ctx->Invo.Get())
    {
        bSandboxForm = Invo->GetEnvironment() == EInvoEnvironment::Sandbox || Invo->IsOfflineMock();
    }
#endif
    TSharedRef<SVerticalBox> V = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16) [ Title(TEXT("Sign in")) ];
    if (!bSandboxForm)
    {
        V->AddSlot().AutoHeight() [ Body(TEXT("Sign in to the game to use your INVO wallet."), true) ];
        return V;
    }
    V->AddSlot().AutoHeight().Padding(0, 0, 0, 16) [ Caption(TEXT("Sandbox player for testing. Your game signs players in for real."), true) ];
    V->AddSlot().AutoHeight().Padding(0, 0, 0, 8) [ SAssignNew(SignInEmail, SEditableTextBox).Style(&S().FieldStyle).HintText(FText::FromString(TEXT("player@yourgame.com"))) ];
    V->AddSlot().AutoHeight().Padding(0, 0, 0, 8) [ SAssignNew(SignInPhone, SEditableTextBox).Style(&S().FieldStyle).HintText(FText::FromString(TEXT("+15555550111"))) ];
    V->AddSlot().AutoHeight().Padding(0, 0, 0, 8) [ SAssignNew(SignInError, STextBlock).Font(FInvoWalletStyle::Font(14)).ColorAndOpacity(S().Danger).AutoWrapText(true) ];
    V->AddSlot().AutoHeight() [ Primary(TEXT("Sign in"), [this] { SignIn(); }) ];
    return V;
}

void SInvoWallet::SignIn()
{
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo)
    {
        return;
    }
    SignInError->SetText(FText::GetEmpty());
    const FString Email = SignInEmail->GetText().ToString().TrimStartAndEnd();
    const FString Phone = SignInPhone->GetText().ToString().TrimStartAndEnd();
    TWeakPtr<SInvoWallet> Weak = SharedThis(this);
    Invo->LoginSandboxNative(Email, Phone, FString(), [Weak](const FInvoResult& Result)
    {
        TSharedPtr<SInvoWallet> Self = Weak.Pin();
        if (!Self.IsValid())
        {
            return;
        }
        if (Result.bOk)
        {
            Self->ShowPanel(Self->Current);
            return;
        }
        Self->SignInError->SetText(FText::FromString(Result.Error.Status == 404 ? TEXT("That player was not found in this sandbox game.") : SendCopy(Result.Error)));
    });
}

void SInvoWallet::ShowPanel(FName Panel)
{
    if (!Pages.Contains(Panel))
    {
        Panel = "Store";
    }
    Current = Panel;
    for (const TPair<FName, TSharedPtr<SButton>>& Tab : TabButtons)
    {
        Tab.Value->SetButtonStyle(Tab.Key == Panel ? &S().TabSelected : &S().Tab);
    }
    UInvoSubsystem* Invo = Ctx->Invo.Get();
    if (!Invo || !Invo->HasPlayerToken())
    {
        RootSwitcher->SetActiveWidgetIndex(0);
        return;
    }
    RootSwitcher->SetActiveWidgetIndex(1);
    PageSwitcher->SetActiveWidgetIndex(PageOrder.IndexOfByKey(Panel));
    Pages[Panel]->OnShown();
}

FReply SInvoWallet::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::F8)
    {
        OnCloseRequested.ExecuteIfBound();
        return FReply::Handled();
    }
    return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SInvoWallet::ShowDemoScreen(const FString& Screen)
{
#if !UE_BUILD_SHIPPING
    Ctx->bDemoHoldApproval = Screen.EndsWith(TEXT("-qr"));
    if (Screen == TEXT("store")) { ShowPanel("Store"); Store->ShowSub(false); }
    else if (Screen == TEXT("items")) { ShowPanel("Store"); Store->ShowSub(true); }
    else if (Screen == TEXT("items-confirm")) { ShowPanel("Store"); Store->ShowSub(true); Store->ConfirmItem(0); }
    else if (Screen == TEXT("send")) { ShowPanel("Send"); Send->Prefill(TEXT("+15555550112"), TEXT("100"), 0); }
    else if (Screen == TEXT("send-review")) { ShowPanel("Send"); Send->Prefill(TEXT("+15555550112"), TEXT("100"), 1); }
    else if (Screen == TEXT("send-qr")) { ShowPanel("Send"); Send->Prefill(TEXT("+15555550112"), TEXT("100"), 2); }
    else if (Screen == TEXT("transfer")) { ShowPanel("Transfer"); Transfer->Prefill(FString(), TEXT("50"), 0); }
    else if (Screen == TEXT("transfer-qr")) { ShowPanel("Transfer"); Transfer->Prefill(FString(), TEXT("50"), 2); }
    else if (Screen == TEXT("collect")) { ShowPanel("Collect"); }
    else if (Screen == TEXT("collect-qr")) { ShowPanel("Collect"); Collect->CollectFirst(); }
    else if (Screen == TEXT("phone")) { ShowPanel("Phone"); }
#endif
}
