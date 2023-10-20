// Fill out your copyright notice in the Description page of Project Settings.
#include "SInvoTransferWidget.h"
#include "InvoFunctions.h"
#include "InvoHttpManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"

#include "Runtime/Online/HTTP/Public/Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/Misc/MessageDialog.h"
#include "Runtime/SlateCore/Public/Widgets/SWidget.h"



void SInvoTransferWidget::Construct(const FArguments& InArgs)
{
    // Populate PriorityOptions
    PriorityOptions.Add(MakeShared<FString>("Sonic"));
    PriorityOptions.Add(MakeShared<FString>("Counter Strike"));
    PriorityOptions.Add(MakeShared<FString>("Ria Ation Adventure"));
    PriorityOptions.Add(MakeShared<FString>("Call Of Duty"));

    ChildSlot
        [
            // Set the background color to match UE's UI
            SNew(SBorder)
                .BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f))
                .Padding(10.0f)
                [
                    SNew(SVerticalBox)
          
                        //+ SVerticalBox::Slot()
                        //.FillHeight(1.0f)
                        //.HAlign(HAlign_Left)
                        //.Padding(10.0f)
                        //[
                        //    SNew(SHorizontalBox)
                        //        // Group for "Game ID"
                        //        + SHorizontalBox::Slot()
                        //        .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width
                        //        
                        //        .Padding(5.0f)
                        //        [
                        //            SNew(SVerticalBox)
                        //                + SVerticalBox::Slot()
                        //                .AutoHeight()
                        //                [
                        //                    SNew(STextBlock)
                        //                        .Text(FText::FromString("Game Name:"))
                        //                ]
                        //                + SVerticalBox::Slot()
                        //                .AutoHeight()
                        //                [
                        //                    SAssignNew(GameIDTextBox, SEditableTextBox)
                        //                        .HintText(FText::FromString(""))
                        //                        .MinDesiredWidth(200.0f) // This sets the minimum width
                        //
                        //                ]
                        //        ]
                        //]

                        
                        + SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        .HAlign(HAlign_Left)
                        .Padding(10.0f)
                        [
                            SNew(SHorizontalBox)
                                // Group for "Game ID"
                                + SHorizontalBox::Slot()
                                .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width
                                .Padding(5.0f)
                                [
                                    SNew(SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Select Game:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(5.0f)
                                        [
                                            SAssignNew(PriorityComboBox, SComboBox<TSharedPtr<FString>>)
                                                .OptionsSource(&PriorityOptions)
                                                .OnGenerateWidget(this, &SInvoTransferWidget::GeneratePriorityComboBoxWidget)
                                                .OnSelectionChanged(this, &SInvoTransferWidget::OnPriorityChanged)
                                                
                                                .InitiallySelectedItem(PriorityOptions[0])
                                                [
                                                    SNew(STextBlock)
                                                        .Text(this, &SInvoTransferWidget::GetPriorityComboBoxText)
                                                        .MinDesiredWidth(180.0f) // This sets the minimum width

                                                ]
                                        ]
                                ]
                        ]

                    
                        +SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        .HAlign(HAlign_Left)
                        .Padding(10.0f)
                        [
                            SNew(SHorizontalBox)
                                // Group for "Game ID"
                                + SHorizontalBox::Slot()
                                .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width
                                .Padding(5.0f)
                                [
                                    SNew(SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Target Player ID:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SAssignNew(TargetPlayerIDTextBox, SEditableTextBox)
                                                .HintText(FText::FromString("Enter Target Player ID "))
                                                .MinDesiredWidth(200.0f) // This sets the minimum width


                                        ]
                                ]
                        ]


                        + SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        .HAlign(HAlign_Left)
                        .Padding(10.0f)
                        [
                            SNew(SHorizontalBox)
                                // Group for "Game ID"
                                + SHorizontalBox::Slot()
                                .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width

                                .Padding(5.0f)
                                [
                                    SNew(SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Default Currency Amount:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SAssignNew(DefaultCurrencyAmountTextBox, SEditableTextBox)
                                                .HintText(FText::FromString("Enter Currency Amount"))
                                                .MinDesiredWidth(200.0f) // This sets the minimum width

                                        ]
                                ]
                        ]

                        
                        + SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        .HAlign(HAlign_Left)
                        .Padding(10.0f)
                        [
                            SNew(SHorizontalBox)
                                // Group for "Game ID"
                                + SHorizontalBox::Slot()
                                .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width
                                .Padding(5.0f)
                                [
                                    SNew(SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Pin:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SAssignNew(PinTextBox, SEditableTextBox)
                                                .HintText(FText::FromString("Enter Pin"))
                                                .MinDesiredWidth(200.0f) // This sets the minimum width


                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(5.0f)
                                        [
                                            SNew(SButton)
                                                .Text(this, &SInvoTransferWidget::GetPinMaskButtonText)
                                                .OnClicked(this, &SInvoTransferWidget::OnTogglePinMask)

                                        ]

                                ]
                        ]
                     

                        // Submit and Close Buttons at the bottom
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
                                        .Text(FText::FromString("Transfer"))
                                        .OnClicked(this, &SInvoTransferWidget::OnTransferClicked)
                                ]

                                // Close Button
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.0f)
                                [
                                    SNew(SButton)
                                        .Text(FText::FromString("Close"))
                                        .OnClicked(this, &SInvoTransferWidget::OnCloseClicked)
                                ]
                        ]
                ]
        ];
}

// This function will be called when the HTTP request completes.
void SInvoTransferWidget::HandleHttpRequestCompleted(bool bWasSuccessful, const FString& ResponseContent)
{
    // Handle the response here.
    // For instance, display a message to the user based on bWasSuccessful.
}

// When setting up the widget (probably in its constructor or initialization function)
void SInvoTransferWidget::SetupWidget()
{
    // ... other setup code ...

    // Bind the callback to the delegate.
    //UInvoHttpManager::GetInstance()->OnHttpRequestCompleted.AddDynamic(this, &SInvoTransferWidget::HandleHttpRequestCompleted);
}


FReply SInvoTransferWidget::OnTransferClicked()
{
    if (!UInvoFunctions::CheckSecretsIni("PlayerID"))
    {
        FString UniqueIDStr;
        UInvoFunctions::GenerateUniquePlayerID(UniqueIDStr);
        FString Message = FString::Printf(TEXT("OnTransferClicked with ID %s"), *UniqueIDStr);
        GEngine->AddOnScreenDebugMessage(1, 3.0, FColor::Green, Message);
        UE_LOG(LogTemp, Warning, TEXT("This log message is from file %s on line %d"), TEXT(__FILE__), __LINE__);

        UInvoHttpManager::GetInstance()->CreatePlayerID(UniqueIDStr);
        CloseTicketWidget();


    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerID already exsist. %s on line %d"), TEXT(__FILE__), __LINE__);
        UInvoFunctions::InvoShowPurchaseWidget();
        CloseTicketWidget();



    }
 
    return FReply::Handled();
}


FReply SInvoTransferWidget::OnCloseClicked()
{
    // Handle closing logic here

    CloseTicketWidget();

    return FReply::Handled();
}

// Additional functions for the ComboBox
TSharedRef<SWidget> SInvoTransferWidget::GeneratePriorityComboBoxWidget(TSharedPtr<FString> InItem)
{
    return SNew(STextBlock).Text(FText::FromString(*InItem));
}

void SInvoTransferWidget::OnPriorityChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    // Handle priority change
}

FText SInvoTransferWidget::GetPriorityComboBoxText() const
{
    return FText::FromString(*PriorityComboBox->GetSelectedItem());
}

void SInvoTransferWidget::CloseTicketWidget() const
{
    // Find and close the parent window of this widget
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
    if (ParentWindow.IsValid())
    {
        ParentWindow->RequestDestroyWindow();
    }
}

bool SInvoTransferWidget::ValidateResponseContent(const FString& ResponseContent)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseContent);

    // 1. Check if it's valid JSON
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Response is not valid JSON."));
        return false;
    }

    // 2. Check for Expected Fields
    if (!JsonObject->HasField("success") || !JsonObject->HasField("message"))
    {
        UE_LOG(LogTemp, Error, TEXT("Mandatory fields are missing."));
        return false;
    }

    // 3. Validate Field Values
    // Example: Ensure "expectedField1" is a string and isn't empty
    FString expectedField1Value = JsonObject->GetStringField("success");
    if (expectedField1Value.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("'success' value shouldn't be empty."));
        return false;
    }

    // Add more validations as needed

    return true;  // If all checks pass
}


bool SInvoTransferWidget::isPinedMasked() const
{
    return bIsPinMasked;
}

FText SInvoTransferWidget::GetPinMaskButtonText() const
{
    return bIsPinMasked ? FText::FromString("Unmask") : FText::FromString("Mask");
}


FReply SInvoTransferWidget::OnTogglePinMask()
{
    bIsPinMasked = !bIsPinMasked;
    if (PinTextBox.IsValid())
    {
        PinTextBox->SetIsPassword(bIsPinMasked);
    }
    return FReply::Handled();
}