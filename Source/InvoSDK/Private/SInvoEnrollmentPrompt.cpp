#include "SInvoEnrollmentPrompt.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SInvoEnrollmentPrompt::Construct(const FArguments& InArgs)
{
    OnPrimary = InArgs._OnPrimary;
    OnSecondary = InArgs._OnSecondary;

    const bool bTwoButtons = !InArgs._PrimaryLabel.IsEmpty();
    const FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);
    const FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);

    TSharedRef<SHorizontalBox> Buttons = SNew(SHorizontalBox);
    if (bTwoButtons)
    {
        Buttons->AddSlot()
            .FillWidth(1.0f)
            .Padding(0.0f, 0.0f, 8.0f, 0.0f)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .ContentPadding(FMargin(12.0f, 10.0f))
                .OnClicked(this, &SInvoEnrollmentPrompt::HandlePrimary)
                [
                    SNew(STextBlock).Text(InArgs._PrimaryLabel).Font(ButtonFont)
                ]
            ];
    }
    Buttons->AddSlot()
        .FillWidth(1.0f)
        [
            SNew(SButton)
            .HAlign(HAlign_Center)
            .ContentPadding(FMargin(12.0f, 10.0f))
            .OnClicked(this, &SInvoEnrollmentPrompt::HandleSecondary)
            [
                SNew(STextBlock).Text(InArgs._SecondaryLabel).Font(ButtonFont)
            ]
        ];

    ChildSlot
    [
        SNew(SOverlay)
        // Dim everything behind the prompt and swallow taps on it, so the buttons are the
        // only thing that can be pressed.
        + SOverlay::Slot()
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f))
            .OnMouseButtonDown_Lambda([](const FGeometry&, const FPointerEvent&) { return FReply::Handled(); })
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .WidthOverride(460.0f)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.98f))
                .Padding(FMargin(24.0f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(InArgs._Message)
                        .Font(BodyFont)
                        .ColorAndOpacity(FLinearColor::White)
                        .AutoWrapText(true)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 20.0f, 0.0f, 0.0f)
                    [
                        Buttons
                    ]
                ]
            ]
        ]
    ];
}

FReply SInvoEnrollmentPrompt::HandlePrimary()
{
    OnPrimary.ExecuteIfBound();
    return FReply::Handled();
}

FReply SInvoEnrollmentPrompt::HandleSecondary()
{
    OnSecondary.ExecuteIfBound();
    return FReply::Handled();
}
