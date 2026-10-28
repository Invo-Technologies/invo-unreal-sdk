#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateTypes.h"

/**
 * The INVO wallet look, shared with the Unity and Godot SDKs: the dark palette, 8 px spacing unit,
 * corner radii and type scale (Display 44, Title 26, Heading 18, Body 16, Caption 14, Overline 13 px).
 * Sizes are pixels at the 1920x1080 reference; the game viewport's DPI scaling does the rest.
 */
struct FInvoWalletStyle
{
    static constexpr float Unit = 8.f;
    static constexpr float SheetWidth = 560.f;
    static constexpr float SheetHeight = 780.f;

    FLinearColor Backdrop, Surface, SurfaceRaised, Field, Border, Accent, AccentSoft, OnAccent;
    FLinearColor TextPrimary, TextSecondary, TextMuted, Link, Success, SuccessSoft, Warning, Danger;

    FSlateColorBrush BackdropBrush;
    FSlateRoundedBoxBrush SheetBrush, CardBrush, CardSelectedBrush, RaisedBrush, TabBarBrush, NoteBrush, QrFrameBrush,
        PillBrush, BadgeBrush, SuccessCircleBrush, PhoneOutlineBrush;
    FSlateColorBrush StrikeBrush;

    FButtonStyle Primary, Secondary, Tab, TabSelected, LinkButton, Chip, ChipSelected, IconButton;
    FEditableTextBoxStyle FieldStyle, AmountStyle;
    FScrollBarStyle ScrollBar;

    static const FInvoWalletStyle& Get()
    {
        static const FInvoWalletStyle Style;
        return Style;
    }

    /** Inter-like sizes in pixels; Slate font sizes are points at 96 DPI, so 1 px = 0.75 pt. */
    static FSlateFontInfo Font(float Px, bool bBold = false);

private:
    FInvoWalletStyle();
};
