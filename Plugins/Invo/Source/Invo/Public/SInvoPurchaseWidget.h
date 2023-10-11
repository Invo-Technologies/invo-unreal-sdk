#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Runtime/Slate/Public/Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"


DECLARE_DELEGATE_OneParam(FOnGetSourceComplete, const FString&);

class SEditableTextBox;
class SWebBrowser;

class SInvoPurchaseWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoPurchaseWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetupWidget();
    void HandleHttpRequestCompleted(bool bWasSuccessful, const FString& ResponseContent);
    void HandlePurchaseCompleted();

    UPROPERTY(BlueprintAssignable, Category = "Invo")
    FOnGetSourceComplete OnGetSourceComplete;


private:
    // UI Elements
    TSharedPtr<SEditableTextBox> SubjectTextBox;
    TSharedPtr <SEditableTextBox> FromPlayerIDTextBox;
    TSharedPtr <SEditableTextBox> TargetPlayerIDTextBox;
    TSharedPtr <SEditableTextBox> DefaultCurrencyAmountTextBox;
    TSharedPtr <SEditableTextBox> DefaultCurrencyNameTextBox;
    TSharedPtr<SEditableTextBox> GameIDTextBox;
    TSharedPtr<SEditableTextBox> PinTextBox;
    bool bIsPinMasked = true;

    TSharedPtr<SMultiLineEditableTextBox> MessageBodyTextBox;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> PriorityComboBox;

    // Data
    TArray<TSharedPtr<FString>> PriorityOptions;

    TSharedPtr<SWebBrowser> WebBrowser;


    // Event handlers
    FReply OnPurchaseClicked();
    FReply OnCloseClicked();
    FReply OnTogglePinMask();

    // Combo box related functions
    TSharedRef<SWidget> GeneratePriorityComboBoxWidget(TSharedPtr<FString> InItem);
    void OnPriorityChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    FText GetPriorityComboBoxText() const;
    FText GetPinMaskButtonText() const;

    void CloseTicketWidget() const;
    bool ValidateResponseContent(const FString& ResponseContent);

    bool isPinedMasked() const;
    void OnBrowserUrlChanged(const FText& NewUrl);
    

};
