// Fill out your copyright notice in the Description page of Project Settings.
#include "SInvoPurchaseWidget.h"
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

#include "Misc/Paths.h"
#include "Misc/FeedbackContext.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformProcess.h"
#include "Widgets/Layout/SBox.h"
//#include "WebBrowser/Public/SWebBrowser.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/WebBrowser/Public/WebBrowserModule.h"
#include "Widgets/SWeakWidget.h"
#include "Runtime/WebBrowser/Public/SWebBrowser.h"
#include "Runtime/WebBrowser/Public/WebBrowserModule.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/World.h"



void SInvoPurchaseWidget::Construct(const FArguments& InArgs)
{
  
    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(0.8f) // Change this as per your needs
                [
                    SAssignNew(WebBrowser, SWebBrowser)
                        .InitialURL(TEXT("http://localhost:5173/")) // Set your desired URL here
                        .OnUrlChanged(this, &SInvoPurchaseWidget::OnBrowserUrlChanged) // <-- Add this line

                ]
                // Submit and Close Buttons at the bottom
                + SVerticalBox::Slot()
                .AutoHeight()
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
                                .Text(FText::FromString("Purchase"))
                                .OnClicked(this, &SInvoPurchaseWidget::OnPurchaseClicked)
                        ]

                        // Close Button
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(5.0f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString("Close"))
                                .OnClicked(this, &SInvoPurchaseWidget::OnCloseClicked)
                        ]
                ]
        ];
}

// This function will be called when the HTTP request completes.
void SInvoPurchaseWidget::HandleHttpRequestCompleted(bool bWasSuccessful, const FString& ResponseContent)
{
    // Handle the response here.
    // For instance, display a message to the user based on bWasSuccessful.
}

// When setting up the widget (probably in its constructor or initialization function)
void SInvoPurchaseWidget::SetupWidget()
{
    // ... other setup code ...

    // Bind the callback to the delegate.
    //UInvoHttpManager::GetInstance()->OnHttpRequestCompleted.AddDynamic(this, &SInvoPurchaseWidget::HandleHttpRequestCompleted);
}


FReply SInvoPurchaseWidget::OnPurchaseClicked()
{
  
    const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

    TMap<FString, FString> FormData;
    
    if (Settings->Player_ID.IsEmpty())
    {
       // UInvoHttpManager::GetInstance()->CreatePlayerID();
    }
  

    FormData.Add(TEXT("player_id"), Settings->Player_ID);
    FormData.Add(TEXT("game_id"), Settings->Game_ID);



    /*
    JsonObject->SetStringField("player_id", "2");
    JsonObject->SetStringField("game_id", "17");
    JsonObject->SetStringField("subject", Subject);
    JsonObject->SetStringField("message_body", Description);
    JsonObject->SetStringField("priority", Priority);
    JsonObject->SetStringField("status", "inbox");
    */

    //TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
    //FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // 3. Directly make the HTTP request without using UInvoFunctions.
    FString Endpoint = "https://api.dev.ourinvo.com/v1/external/purchase"; // Replace with your actual server address
    FString HttpMethod = "POST";

    //4. Headers 
    TMap<FString, FString> Headers;

    // Create HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetURL(Endpoint);
    HttpRequest->SetVerb(HttpMethod);
    HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

    FString Payload;
    for (const auto& Pair : FormData)
    {
        if (!Payload.IsEmpty())
        {
            Payload.Append(TEXT("&"));
        }
        Payload.Append(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value));
    }

    HttpRequest->SetContentAsString(Payload);


    for (const auto& Header : Headers)
    {
        UE_LOG(LogTemp, Warning, TEXT("Headers %s"), *Header.Value);

    }
    UE_LOG(LogTemp, Warning, TEXT("Payload is  %s"), *Payload);

    // Make the HTTP Request
    UInvoHttpManager::GetInstance()->MakeHttpRequest(Endpoint, HttpMethod, Headers, FormData,
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
                UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid response with response %s"), *ResponseContent);

                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to get a valid response")));

            }
        });
   
    return FReply::Handled();
}


FReply SInvoPurchaseWidget::OnCloseClicked()
{
    // Handle closing logic here

    CloseTicketWidget();

    return FReply::Handled();
}

// Additional functions for the ComboBox
TSharedRef<SWidget> SInvoPurchaseWidget::GeneratePriorityComboBoxWidget(TSharedPtr<FString> InItem)
{
    return SNew(STextBlock).Text(FText::FromString(*InItem));
}

void SInvoPurchaseWidget::OnPriorityChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    // Handle priority change
}

FText SInvoPurchaseWidget::GetPriorityComboBoxText() const
{
    return FText::FromString(*PriorityComboBox->GetSelectedItem());
}

void SInvoPurchaseWidget::CloseTicketWidget() const
{
    // Find and close the parent window of this widget
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
    if (ParentWindow.IsValid())
    {
        ParentWindow->RequestDestroyWindow();
    }
}

bool SInvoPurchaseWidget::ValidateResponseContent(const FString& ResponseContent)
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


bool SInvoPurchaseWidget::isPinedMasked() const
{
    return bIsPinMasked;
}

FText SInvoPurchaseWidget::GetPinMaskButtonText() const
{
    return bIsPinMasked ? FText::FromString("Unmask") : FText::FromString("Mask");
}


FReply SInvoPurchaseWidget::OnTogglePinMask()
{
    bIsPinMasked = !bIsPinMasked;
    if (PinTextBox.IsValid())
    {
        PinTextBox->SetIsPassword(bIsPinMasked);
    }
    return FReply::Handled();
}

void SInvoPurchaseWidget::HandlePurchaseCompleted()
{
    // Code to handle post-purchase actions. For example, show a confirmation message.
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invo Purchase Complelted..")));

}


void SInvoPurchaseWidget::OnBrowserUrlChanged(const FText& NewUrl)
{
    FString CurrentUrl = NewUrl.ToString();

    // Check if the current URL matches the completion URL
    if (CurrentUrl.Contains(TEXT("https://www.ourinvo.com/?buttonClicked=true")))
    {
        // Purchase has been completed. Handle accordingly.
        HandlePurchaseCompleted();
    }
    else if (CurrentUrl.Contains(TEXT("closeButton")))
    {
        CloseTicketWidget();
    }
}

