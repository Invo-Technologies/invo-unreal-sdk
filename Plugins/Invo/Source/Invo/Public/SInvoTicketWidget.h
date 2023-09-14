#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

class SInvoTicketWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoTicketWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // UI Elements
    TSharedPtr<SEditableTextBox> SubjectTextBox;
    TSharedPtr<SMultiLineEditableTextBox> MessageBodyTextBox;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> PriorityComboBox;

    // Data
    TArray<TSharedPtr<FString>> PriorityOptions;

    // Event handlers
    FReply OnSubmitClicked();
    FReply OnCloseClicked();

    // Combo box related functions
    TSharedRef<SWidget> GeneratePriorityComboBoxWidget(TSharedPtr<FString> InItem);
    void OnPriorityChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    FText GetPriorityComboBoxText() const;
};
