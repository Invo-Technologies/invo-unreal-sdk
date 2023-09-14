#include "SInvoTicketWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"

void SInvoTicketWidget::Construct(const FArguments& InArgs)
{
    // Populate PriorityOptions
    PriorityOptions.Add(MakeShared<FString>("Low"));
    PriorityOptions.Add(MakeShared<FString>("Medium"));
    PriorityOptions.Add(MakeShared<FString>("High"));
    PriorityOptions.Add(MakeShared<FString>("Urgent"));

    ChildSlot
        [
            // Set the background color to match UE's UI
            SNew(SBorder)
                .BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f))
                .Padding(10.0f)
                [
                    SNew(SVerticalBox)

                        // Subject Field
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.0f)
                        [
                            SAssignNew(SubjectTextBox, SEditableTextBox)
                                .HintText(FText::FromString("Enter Ticket Subject"))
                        ]

                        // Priority Dropdown
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.0f)
                        [
                            SAssignNew(PriorityComboBox, SComboBox<TSharedPtr<FString>>)
                                .OptionsSource(&PriorityOptions)
                                .OnGenerateWidget(this, &SInvoTicketWidget::GeneratePriorityComboBoxWidget)
                                .OnSelectionChanged(this, &SInvoTicketWidget::OnPriorityChanged)
                                .InitiallySelectedItem(PriorityOptions[0])
                                [
                                    SNew(STextBlock)
                                        .Text(this, &SInvoTicketWidget::GetPriorityComboBoxText)
                                ]
                        ]

                        // Message Body Field
                        + SVerticalBox::Slot()
                        .MaxHeight(300.0f)
                        .Padding(5.0f)
                        [
                            SAssignNew(MessageBodyTextBox, SMultiLineEditableTextBox)
                                .HintText(FText::FromString("Enter Detailed Message"))
                        ]


                        // Submit and Close Buttons at the bottom
                        + SVerticalBox::Slot()
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

// Additional functions for the ComboBox
TSharedRef<SWidget> SInvoTicketWidget::GeneratePriorityComboBoxWidget(TSharedPtr<FString> InItem)
{
    return SNew(STextBlock).Text(FText::FromString(*InItem));
}

void SInvoTicketWidget::OnPriorityChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    // Handle priority change
}

FText SInvoTicketWidget::GetPriorityComboBoxText() const
{
    return FText::FromString(*PriorityComboBox->GetSelectedItem());
}
