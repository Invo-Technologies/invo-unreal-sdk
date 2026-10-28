#include "Wallet/InvoWalletStyle.h"
#include "Styling/CoreStyle.h"

namespace
{
    FLinearColor Hex(const TCHAR* Text) { return FLinearColor(FColor::FromHex(Text)); }

    FButtonStyle MakeButton(const FLinearColor& Fill, const FLinearColor& Hover, const FLinearColor& Outline, float Radius, const FMargin& Padding)
    {
        return FButtonStyle()
            .SetNormal(FSlateRoundedBoxBrush(Fill, Radius, Outline, Outline.A > 0 ? 1.f : 0.f))
            .SetHovered(FSlateRoundedBoxBrush(Hover, Radius, Outline, Outline.A > 0 ? 1.f : 0.f))
            .SetPressed(FSlateRoundedBoxBrush(Hover, Radius, Outline, Outline.A > 0 ? 1.f : 0.f))
            .SetDisabled(FSlateRoundedBoxBrush(Fill.CopyWithNewOpacity(Fill.A * 0.45f), Radius, Outline, Outline.A > 0 ? 1.f : 0.f))
            .SetNormalPadding(Padding)
            .SetPressedPadding(Padding);
    }

    FEditableTextBoxStyle MakeField(const FInvoWalletStyle& S, float FontPx, bool bBold, const FMargin& Padding, bool bOutlined)
    {
        const float Outline = bOutlined ? 1.f : 0.f;
        return FEditableTextBoxStyle()
            .SetBackgroundImageNormal(FSlateRoundedBoxBrush(bOutlined ? S.Field : FLinearColor::Transparent, 12.f, S.Border, Outline))
            .SetBackgroundImageHovered(FSlateRoundedBoxBrush(bOutlined ? S.Field : FLinearColor::Transparent, 12.f, S.Border, Outline))
            .SetBackgroundImageFocused(FSlateRoundedBoxBrush(bOutlined ? S.Field : FLinearColor::Transparent, 12.f, S.Accent, Outline))
            .SetBackgroundImageReadOnly(FSlateRoundedBoxBrush(bOutlined ? S.Field : FLinearColor::Transparent, 12.f, S.Border, Outline))
            .SetTextStyle(FTextBlockStyle().SetFont(FInvoWalletStyle::Font(FontPx, bBold)).SetColorAndOpacity(S.TextPrimary))
            .SetForegroundColor(S.TextPrimary)
            .SetFocusedForegroundColor(S.TextPrimary)
            .SetReadOnlyForegroundColor(S.TextSecondary)
            .SetPadding(Padding);
    }
}

FSlateFontInfo FInvoWalletStyle::Font(float Px, bool bBold)
{
    return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", FMath::RoundToInt(Px * 0.75f));
}

FInvoWalletStyle::FInvoWalletStyle()
    : Backdrop(FLinearColor(FColor(0x05, 0x07, 0x0B, 200)))
    , Surface(Hex(TEXT("141922")))
    , SurfaceRaised(Hex(TEXT("1B2130")))
    , Field(Hex(TEXT("0F131A")))
    , Border(Hex(TEXT("283041")))
    , Accent(Hex(TEXT("3B6FE0")))
    , AccentSoft(Hex(TEXT("1A2336")))
    , OnAccent(FLinearColor::White)
    , TextPrimary(Hex(TEXT("EEF1F6")))
    , TextSecondary(Hex(TEXT("A3ACBA")))
    , TextMuted(Hex(TEXT("7D8797")))
    , Link(Hex(TEXT("8FB3FF")))
    , Success(Hex(TEXT("6FD3A8")))
    , SuccessSoft(Hex(TEXT("133A2C")))
    , Warning(Hex(TEXT("F2A93B")))
    , Danger(Hex(TEXT("F07068")))
    , BackdropBrush(Backdrop)
    , SheetBrush(Surface, 20.f, Border, 1.f)
    , CardBrush(Field, 12.f, Border, 1.f)
    , CardSelectedBrush(AccentSoft, 12.f, Accent, 1.f)
    , RaisedBrush(SurfaceRaised, 12.f, Border, 1.f)
    , TabBarBrush(Field, 12.f, FLinearColor::Transparent, 0.f)
    , NoteBrush(Field, 12.f, FLinearColor::Transparent, 0.f)
    , QrFrameBrush(FLinearColor::White, 16.f, FLinearColor::Transparent, 0.f)
    , PillBrush(SurfaceRaised, 999.f, Border, 1.f)
    , BadgeBrush(SurfaceRaised, 8.f, Border, 1.f)
    , SuccessCircleBrush(SuccessSoft, 999.f, Success, 2.f)
    , PhoneOutlineBrush(FLinearColor::Transparent, 3.f, TextPrimary, 1.5f)
    , StrikeBrush(TextMuted)
{
    const FMargin ButtonPad(Unit * 2, Unit + 2);
    Primary = MakeButton(Accent, Accent * 1.12f, FLinearColor::Transparent, 12.f, ButtonPad);
    Secondary = MakeButton(SurfaceRaised, Border, Border, 12.f, ButtonPad);
    Tab = MakeButton(FLinearColor::Transparent, SurfaceRaised * 0.8f, FLinearColor::Transparent, 9.f, FMargin(Unit + 4, 6));
    TabSelected = MakeButton(SurfaceRaised, SurfaceRaised, FLinearColor::Transparent, 9.f, FMargin(Unit + 4, 6));
    LinkButton = MakeButton(FLinearColor::Transparent, FLinearColor::Transparent, FLinearColor::Transparent, 0.f, FMargin(2, 0));
    Chip = MakeButton(Field, SurfaceRaised, Border, 12.f, FMargin(Unit * 2, Unit + 2));
    ChipSelected = MakeButton(AccentSoft, AccentSoft, Accent, 12.f, FMargin(Unit * 2, Unit + 2));
    IconButton = MakeButton(Field, SurfaceRaised, Border, 10.f, FMargin(0));

    FieldStyle = MakeField(*this, 16.f, false, FMargin(Unit + 4, Unit + 2), true);
    AmountStyle = MakeField(*this, 44.f, true, FMargin(Unit, Unit), false);

    ScrollBar = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
    const FSlateRoundedBoxBrush Thumb(Border, 4.f);
    ScrollBar.SetNormalThumbImage(Thumb).SetHoveredThumbImage(Thumb).SetDraggedThumbImage(Thumb)
        .SetVerticalBackgroundImage(FSlateNoResource()).SetHorizontalBackgroundImage(FSlateNoResource())
        .SetVerticalTopSlotImage(FSlateNoResource()).SetVerticalBottomSlotImage(FSlateNoResource())
        .SetThickness(6.f);
}
