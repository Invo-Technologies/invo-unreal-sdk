#include "SInvoTicketWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Application/SlateApplication.h"

void SInvoTicketWidget::Construct(const FArguments& InArgs)
{
    ChildSlot
        [
            // Set the background color to match UE's UI
            SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))  // Use a solid white box as the brush
                .BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f))
                .Padding(10.0f)
                [
                    SNew(SVerticalBox)

                        // ... [You can add other UI components here as needed]

                        // Buttons at the bottom, centered
                        +SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        .VAlign(VAlign_Bottom)
                        .HAlign(HAlign_Center)
                        .Padding(10.0f)
                        [
                            SNew(SHorizontalBox)

                                // Submit Button
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.0f)
                                [
                                    SNew(SButton)
                                        .Text(FText::FromString("Submit"))
                                        .OnClicked(this, &SInvoTicketWidget::OnSubmitClicked)
                                ]

                                // Close Button
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.0f)
                                [
                                    SNew(SButton)
                                        .Text(FText::FromString("Close"))
                                        .OnClicked(this, &SInvoTicketWidget::OnCloseClicked)
                                ]
                        ]
                ]
        ];
}

FReply SInvoTicketWidget::OnSubmitClicked()
{
    // Handle submission logic here
    return FReply::Handled();
}

FReply SInvoTicketWidget::OnCloseClicked()
{
    // Handle closing logic here
    return FReply::Handled();
}
