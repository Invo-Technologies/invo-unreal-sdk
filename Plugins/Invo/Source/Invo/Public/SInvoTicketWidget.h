#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"


class SInvoTicketWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoTicketWidget) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);

    /** Sets the parent window for this widget */
    void SetParentWindow(TSharedPtr<SWindow> InParentWindow) { ParentWindow = InParentWindow; }

private:

    TSharedPtr<SEditableTextBox> AddressBarTextBox;

    /** Pointer to the ticket title text box */
    TSharedPtr<SEditableTextBox> TicketTitleTextBox;

    /** Pointer to the description text box */
    TSharedPtr<SEditableTextBox> DescriptionTextBox;

    /** Pointer to the parent window */
    TSharedPtr<SWindow> ParentWindow;

    /** Handler for the close button click */
    FReply OnCloseClicked();

    /** Handler for the submit button click */
    FReply OnSubmitClicked();


    FReply OnBackClicked();


    FReply OnForwardClicked();


    void OnAddressBarTextCommitted(const FText& Text, ETextCommit::Type CommitType);
  
    FReply OnRefreshClicked();

};
