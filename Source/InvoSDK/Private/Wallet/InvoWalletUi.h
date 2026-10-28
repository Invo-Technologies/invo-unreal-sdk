#pragma once

// Small builders for the wallet's Slate widgets, so every page reads like the Godot/Unity layouts.

#include "CoreMinimal.h"
#include "InvoTypes.h"
#include "Wallet/InvoWalletStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

class FJsonObject;

namespace InvoUi
{
    using FStyle = FInvoWalletStyle;
    inline const FStyle& S() { return FStyle::Get(); }

    TSharedRef<STextBlock> Label(const FString& Text, float Px, const FLinearColor& Color, bool bBold = false, bool bWrap = false,
                                 ETextJustify::Type Justify = ETextJustify::Left);
    inline TSharedRef<STextBlock> Title(const FString& Text) { return Label(Text, 26, S().TextPrimary, true); }
    inline TSharedRef<STextBlock> Heading(const FString& Text) { return Label(Text, 18, S().TextPrimary, true); }
    inline TSharedRef<STextBlock> Body(const FString& Text, bool bWrap = false) { return Label(Text, 16, S().TextPrimary, false, bWrap); }
    inline TSharedRef<STextBlock> BodyStrong(const FString& Text) { return Label(Text, 16, S().TextPrimary, true); }
    inline TSharedRef<STextBlock> Caption(const FString& Text, bool bWrap = false) { return Label(Text, 14, S().TextSecondary, false, bWrap); }
    inline TSharedRef<STextBlock> Muted(const FString& Text) { return Label(Text, 14, S().TextMuted); }
    inline TSharedRef<STextBlock> Danger(const FString& Text = FString()) { return Label(Text, 14, S().Danger, false, true); }
    TSharedRef<STextBlock> Overline(const FString& Text);

    TSharedRef<SButton> Button(const FString& Text, const FButtonStyle& Style, TFunction<void()> OnClick, const FLinearColor& TextColor,
                               bool bBold = true, float Px = 16);
    inline TSharedRef<SButton> Primary(const FString& Text, TFunction<void()> OnClick) { return Button(Text, S().Primary, MoveTemp(OnClick), S().OnAccent); }
    inline TSharedRef<SButton> Secondary(const FString& Text, TFunction<void()> OnClick) { return Button(Text, S().Secondary, MoveTemp(OnClick), S().TextPrimary); }
    inline TSharedRef<SButton> Link(const FString& Text, TFunction<void()> OnClick) { return Button(Text, S().LinkButton, MoveTemp(OnClick), S().Link, false, 14); }
    /** A square header button; Glyph is drawn as text ("x", "<"). */
    TSharedRef<SButton> IconButton(const FString& Glyph, const FString& Tooltip, TFunction<void()> OnClick);
    /** The phone outline icon button. */
    TSharedRef<SButton> PhoneButton(TFunction<void()> OnClick);
    /** Back arrow + heading row. */
    TSharedRef<SWidget> BackHeader(const FString& Text, TFunction<void()> OnBack);

    TSharedRef<SBorder> Card(const TSharedRef<SWidget>& Content, const FSlateBrush* Brush = nullptr, const FMargin& Padding = FMargin(12));
    TSharedRef<SWidget> Note(const FString& Text);
    TSharedRef<SEditableTextBox> Field(const FString& Hint, const FString& Text = FString());
    /** A row: muted label on the left, strong value on the right. */
    TSharedRef<SWidget> Row(const FString& Left, const FString& Right);
    /** A diamond "gem" tile used for packs and items. */
    TSharedRef<SWidget> Gem(float Size);

    // ---------------- formatting ----------------

    /** "••• 0111" (the last four digits), or "Not set". */
    FString MaskedPhone(const FString& Phone);
    /** "120", "4.5" from "120.00" / 4.5; non-numbers pass through. */
    FString Amount(const FString& Value);
    FString Usd(double Value);
    /** A field as text whether it is a string or a number ("7" for 7.0). Empty when absent or null. */
    FString JsonStr(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field);
    double JsonNum(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, double Default = 0);
    bool JsonBool(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field);

    // ---------------- player-facing copy (never the raw response: it can carry another player's data) ----------------

    bool IsNetworkError(const FInvoError& E);
    FString SendCopy(const FInvoError& E);
    FString TransferCopy(const FInvoError& E, const FString& Fallback = TEXT("That transfer could not be completed. Please try again."));
    FString PurchaseCopy(const FInvoError& E);
    FString StoreCopy(const FInvoError& E);
}
