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
                                                .Text(FText::FromString("Target Game ID:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SAssignNew(GameIDTextBox, SEditableTextBox)
                                                .HintText(FText::FromString("Enter Game ID"))
                                                .MinDesiredWidth(200.0f) // This sets the minimum width

                                        ]
                                ]
                        ]

                        
                       //+ SVerticalBox::Slot()
                       //.FillHeight(1.0f)
                       //.HAlign(HAlign_Left)
                       //.Padding(10.0f)
                       //[
                       //     //Sew(SHorizontalBox)
                       //     //  // Group for "Game ID"
                       //     //  + SHorizontalBox::Slot()
                       //     //  .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width
                       //     //  .Padding(5.0f)
                       //     //  [
                       //     //      SNew(SVerticalBox)
                       //     //          + SVerticalBox::Slot()
                       //     //          .AutoHeight()
                       //     //          [
                       //     //              SNew(STextBlock)
                       //     //                  .Text(FText::FromString("From Player ID (Optional):"))
                       //     //          ]
                       //     //          + SVerticalBox::Slot()
                       //     //          .AutoHeight()
                       //     //          [
                       //     //              SAssignNew(FromPlayerIDTextBox, SEditableTextBox)
                       //     //                  .HintText(FText::FromString("Enter Player ID "))
                       //     //                  .MinDesiredWidth(200.0f) // This sets the minimum width
                       //     //
                       //     //
                       //     //          ]
                       //     //  ]
                       //]   //



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
                                                .Text(FText::FromString("Default Currency Name:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SAssignNew(DefaultCurrencyNameTextBox, SEditableTextBox)
                                                .HintText(FText::FromString("Enter Currency Name"))
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
    UInvoHttpManager::GetInstance()->OnHttpRequestCompleted.AddDynamic(this, &SInvoTransferWidget::HandleHttpRequestCompleted);
}


FReply SInvoTransferWidget::OnTransferClicked()
{
    // 1. Get the text from each of the UI fields.
    FString GameID = GameIDTextBox->GetText().ToString();
    FString TargetPlayerID = TargetPlayerIDTextBox->GetText().ToString();
    FString Priority;
 

    // 2. Create a JSON payload with this data.
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    // Settings from Invo SDK Feilds
    const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

    JsonObject->SetNumberField("user_id", 4);
    JsonObject->SetNumberField("player_id", 4);
    //JsonObject->SetNumberField("game_id", Settings->Game_ID);
    JsonObject->SetStringField("subject", GameID);
    //JsonObject->SetStringField("message_body", Description);
    JsonObject->SetStringField("status", "inbox");


    FString Payload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // 3. Directly make the HTTP request without using UInvoFunctions.
    FString Endpoint = "http://127.0.0.1:3030/create_ticket"; // Replace with your actual server address
    FString HttpMethod = "POST";

    //4. Headers 
    TMap<FString, FString> Headers;

    // Create HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetURL(Endpoint);
    HttpRequest->SetVerb(HttpMethod);
    HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(Payload);


    // Alert for empty fields
    if (GameID.IsEmpty() ||TargetPlayerID.IsEmpty())
    {
        // Show a Windows alert box
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Game ID and Target Player ID cannot be empty.")));
        return FReply::Unhandled();  // Do not proceed further
    }


    // ... (Your existing code to gather data and prepare the payload)


    // Make the HTTP Request
    UInvoHttpManager::GetInstance()->MakeHttpRequest(Endpoint, HttpMethod, Headers, Payload,
        [this](const bool bSuccess, const FString& ResponseContent)
        {
            if (ValidateResponseContent(ResponseContent))
            {
                // Handle the valid response
                // Log the response's content as a string.
                FString StringbSuccess = bSuccess ? "True" : "False";
                UE_LOG(LogTemp, Warning, TEXT("HTTP Response: %s and is bSucess %s"), *ResponseContent, *StringbSuccess);
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Ticket Submited Succeefully.")));

                UWorld* World = GWorld->GetWorld();

                CloseTicketWidget();

                // Restore player input and cursor mode
                APlayerController* PlayerController = World->GetFirstPlayerController();

                if (PlayerController)
                {
                    // Set the input mode back to the game
                    FInputModeGameOnly InputMode;
                    PlayerController->SetInputMode(InputMode);

                    // Lock the mouse cursor to the center of the screen
                    PlayerController->bShowMouseCursor = false;
                    PlayerController->bEnableClickEvents = false;
                    PlayerController->bEnableMouseOverEvents = false;
                }
            }
            else
            {
                // Handle the invalid response
                UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid response."));
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to get a valid response")));

            }
        });




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