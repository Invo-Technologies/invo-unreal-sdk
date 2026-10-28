#include "Wallet/InvoWalletUi.h"
#include "InvoLibrary.h"
#include "Dom/JsonObject.h"

namespace InvoUi
{
    TSharedRef<STextBlock> Label(const FString& Text, float Px, const FLinearColor& Color, bool bBold, bool bWrap, ETextJustify::Type Justify)
    {
        return SNew(STextBlock)
            .Text(FText::FromString(Text))
            .Font(FStyle::Font(Px, bBold))
            .ColorAndOpacity(Color)
            .AutoWrapText(bWrap)
            .Justification(Justify);
    }

    TSharedRef<STextBlock> Overline(const FString& Text)
    {
        return Label(Text.ToUpper(), 13, S().TextMuted, true);
    }

    TSharedRef<SButton> Button(const FString& Text, const FButtonStyle& Style, TFunction<void()> OnClick, const FLinearColor& TextColor, bool bBold, float Px)
    {
        return SNew(SButton)
            .ButtonStyle(&Style)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .OnClicked_Lambda([OnClick = MoveTemp(OnClick)]()
            {
                if (OnClick)
                {
                    OnClick();
                }
                return FReply::Handled();
            })
            [
                SNew(STextBlock)
                .Text(FText::FromString(Text))
                .Font(FStyle::Font(Px, bBold))
                .ColorAndOpacity(TextColor)
            ];
    }

    TSharedRef<SButton> IconButton(const FString& Glyph, const FString& Tooltip, TFunction<void()> OnClick)
    {
        return SNew(SButton)
            .ButtonStyle(&S().IconButton)
            .ToolTipText(FText::FromString(Tooltip))
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .OnClicked_Lambda([OnClick = MoveTemp(OnClick)]()
            {
                OnClick();
                return FReply::Handled();
            })
            [
                SNew(SBox).WidthOverride(32).HeightOverride(32).HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    Label(Glyph, 18, S().TextPrimary, true)
                ]
            ];
    }

    TSharedRef<SButton> PhoneButton(TFunction<void()> OnClick)
    {
        return SNew(SButton)
            .ButtonStyle(&S().IconButton)
            .ToolTipText(FText::FromString(TEXT("Your phone number")))
            .OnClicked_Lambda([OnClick = MoveTemp(OnClick)]()
            {
                OnClick();
                return FReply::Handled();
            })
            [
                SNew(SBox).WidthOverride(32).HeightOverride(32).HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(11).HeightOverride(17)
                    [
                        SNew(SBorder).BorderImage(&S().PhoneOutlineBrush).Padding(0).VAlign(VAlign_Bottom).HAlign(HAlign_Center)
                        [
                            SNew(SBox).WidthOverride(3).HeightOverride(3).Padding(FMargin(0, 0, 0, 2))
                        ]
                    ]
                ]
            ];
    }

    TSharedRef<SWidget> BackHeader(const FString& Text, TFunction<void()> OnBack)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, FStyle::Unit, 0)
            [
                IconButton(TEXT("<"), TEXT("Back"), MoveTemp(OnBack))
            ]
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
            [
                Heading(Text)
            ];
    }

    TSharedRef<SBorder> Card(const TSharedRef<SWidget>& Content, const FSlateBrush* Brush, const FMargin& Padding)
    {
        return SNew(SBorder)
            .BorderImage(Brush ? Brush : &S().CardBrush)
            .Padding(Padding)
            [
                Content
            ];
    }

    TSharedRef<SWidget> Note(const FString& Text)
    {
        return SNew(SBorder)
            .BorderImage(&S().NoteBrush)
            .Padding(FMargin(12, 10))
            [
                Caption(Text, true)
            ];
    }

    TSharedRef<SEditableTextBox> Field(const FString& Hint, const FString& Text)
    {
        return SNew(SEditableTextBox)
            .Style(&S().FieldStyle)
            .HintText(FText::FromString(Hint))
            .Text(FText::FromString(Text));
    }

    TSharedRef<SWidget> Row(const FString& Left, const FString& Right)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center) [ Muted(Left) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) [ BodyStrong(Right) ];
    }

    TSharedRef<SWidget> Gem(float Size)
    {
        static const FSlateRoundedBoxBrush Tile(FLinearColor(FColor(0x4A, 0x1E, 0x33)), 8.f, FLinearColor(FColor(0x8C, 0x2A, 0x55)), 1.f);
        static const FSlateRoundedBoxBrush Stone(FLinearColor(FColor(0xE8, 0x3E, 0x8C)), 3.f, FLinearColor(FColor(0xFF, 0x9C, 0xC6)), 2.f);
        const float Inner = Size * 0.42f;
        return SNew(SBox).WidthOverride(Size).HeightOverride(Size)
            [
                SNew(SBorder).BorderImage(&Tile).HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(Inner).HeightOverride(Inner)
                    .RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(45.f))))
                    .RenderTransformPivot(FVector2D(0.5f, 0.5f))
                    [
                        SNew(SBorder).BorderImage(&Stone)
                    ]
                ]
            ];
    }

    FString MaskedPhone(const FString& Phone)
    {
        const FString Digits = Phone.TrimStartAndEnd();
        if (Digits.IsEmpty())
        {
            return TEXT("Not set");
        }
        return FString(TEXT("••• ")) + Digits.Right(4);
    }

    FString Amount(const FString& Value)
    {
        const FString Trimmed = Value.TrimStartAndEnd();
        return Trimmed.IsNumeric() ? UInvoLibrary::FormatMoney(FCString::Atod(*Trimmed)) : Trimmed;
    }

    FString Usd(double Value)
    {
        return FString::Printf(TEXT("$%.2f"), Value);
    }

    FString JsonStr(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
    {
        if (!Object.IsValid())
        {
            return FString();
        }
        const TSharedPtr<FJsonValue> V = Object->TryGetField(Field);
        if (!V.IsValid() || V->IsNull())
        {
            return FString();
        }
        if (V->Type == EJson::Number)
        {
            return UInvoLibrary::FormatMoney(V->AsNumber());
        }
        if (V->Type == EJson::Boolean)
        {
            return V->AsBool() ? TEXT("true") : TEXT("false");
        }
        return V->AsString();
    }

    double JsonNum(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, double Default)
    {
        const FString Text = JsonStr(Object, Field);
        return Text.IsNumeric() ? FCString::Atod(*Text) : Default;
    }

    bool JsonBool(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
    {
        bool bValue = false;
        return Object.IsValid() && Object->TryGetBoolField(Field, bValue) && bValue;
    }

    // ---------------------------------------------------------------- copy

    bool IsNetworkError(const FInvoError& E)
    {
        return E.Status == 0 && E.Code.IsEmpty();
    }

    static int32 Seconds(const FInvoError& E, int32 Fallback)
    {
        const float S = UInvoLibrary::GetRetryAfterSeconds(E);
        return S >= 0 ? FMath::CeilToInt(S) : Fallback;
    }

    static FString Plural(int32 N)
    {
        return FString::Printf(TEXT("%d second%s"), N, N == 1 ? TEXT("") : TEXT("s"));
    }

    static FString Ref(const FInvoError& E, const TCHAR* Word)
    {
        return E.RequestId.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s %s)"), Word, *E.RequestId);
    }

    static bool IsRateLimited(const FInvoError& E)
    {
        return UInvoLibrary::IsRateLimited(E) || E.Code == TEXT("rate_limited");
    }

    FString SendCopy(const FInvoError& E)
    {
        if (IsNetworkError(E)) { return TEXT("No connection. Check your network and try again."); }
        if (IsRateLimited(E)) { return FString::Printf(TEXT("Too many attempts. Try again in %s."), *Plural(Seconds(E, 60))); }
        if (UInvoLibrary::IsPhoneShareApprovalRequired(E))
        {
            return TEXT("This phone number is shared with another account, so it needs approval before currency can move. The account holder must approve the number in the Invo app, then try again.");
        }
        if (UInvoLibrary::IsDuplicateRequest(E)) { return TEXT("That request was already processed. Nothing was charged twice."); }
        if (E.Code == TEXT("TENANT_NOT_MIGRATED")) { return TEXT("Phone approval is not enabled for this game yet. Contact Invo to enable it."); }
        switch (E.Status)
        {
        case 400: return TEXT("Some details were not accepted. Check the phone numbers and amount.");
        case 401:
        case 403: return TEXT("This game is not allowed to complete that request.");
        case 404: return TEXT("We could not find that send. It may have expired or been claimed already.");
        case 410: return TEXT("That approval is no longer valid. Start the send again.");
        case 503: return TEXT("Invo is briefly unavailable. Please try again in a moment.");
        default: break;
        }
        if (E.Status >= 500) { return TEXT("Invo had a problem completing that. Please try again.") + Ref(E, TEXT("Ref")); }
        return TEXT("That request could not be completed. Please try again.");
    }

    FString TransferCopy(const FInvoError& E, const FString& Fallback)
    {
        FString Message = Fallback;
        if (IsNetworkError(E)) { Message = TEXT("No connection to Invo. Check your internet and try again."); }
        else if (IsRateLimited(E)) { Message = FString::Printf(TEXT("Too many attempts. Try again in %s."), *Plural(Seconds(E, 30))); }
        else if (E.Code == TEXT("GUARDIAN_REQUIRED")) { Message = TEXT("This account needs a parent or guardian linked in the Invo app before it can transfer currency."); }
        else if (UInvoLibrary::IsPhoneShareApprovalRequired(E)) { Message = TEXT("That number is shared with another Invo account and has to be approved with a one-time code first."); }
        else if (E.Code == TEXT("TENANT_NOT_MIGRATED")) { Message = TEXT("Phone approval is not enabled for this game yet. Contact Invo to enable it."); }
        else if (E.Status == 403) { Message = TEXT("Invo declined this transfer for this account."); }
        else if (UInvoLibrary::IsDuplicateRequest(E)) { Message = TEXT("This transfer was already submitted. Check your balance before trying again."); }
        else if (E.Status >= 500) { Message = TEXT("Invo is having trouble right now. Try again in a few minutes."); }
        return Message + Ref(E, TEXT("ref"));
    }

    FString PurchaseCopy(const FInvoError& E)
    {
        if (IsNetworkError(E)) { return TEXT("No connection. Check your network and try again."); }
        if (IsRateLimited(E)) { return TEXT("Too many attempts. Please wait before trying again."); }
        if (UInvoLibrary::IsDuplicateRequest(E)) { return TEXT("This purchase already went through."); }
        if (E.Status == 401 || E.Status == 403) { return TEXT("This game is not authorised to complete the purchase."); }
        if (E.Status >= 500) { return TEXT("The store is having trouble. Please try again shortly."); }
        if (UInvoLibrary::IsInsufficientBalance(E)) { return TEXT("You do not have enough balance for this item."); }
        return TEXT("That purchase could not be completed.");
    }

    FString StoreCopy(const FInvoError& E)
    {
        return IsNetworkError(E) ? TEXT("No connection. Check your network and try again.") : TEXT("The store could not be loaded. Please try again.");
    }
}
