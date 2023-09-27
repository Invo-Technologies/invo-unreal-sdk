#include "SInvoTicketWidget.h"
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

// This function will be called when the HTTP request completes.
void SInvoTicketWidget::HandleHttpRequestCompleted(bool bWasSuccessful, const FString& ResponseContent)
{
    // Handle the response here.
    // For instance, display a message to the user based on bWasSuccessful.
}

// When setting up the widget (probably in its constructor or initialization function)
void SInvoTicketWidget::SetupWidget()
{
    // ... other setup code ...

    // Bind the callback to the delegate.
    UInvoHttpManager::GetInstance()->OnHttpRequestCompleted.AddDynamic(this, &SInvoTicketWidget::HandleHttpRequestCompleted);
}


FReply SInvoTicketWidget::OnSubmitClicked()
{
    // 1. Get the text from each of the UI fields.
    FString Subject = SubjectTextBox->GetText().ToString();
    FString Description = MessageBodyTextBox->GetText().ToString();
    FString Priority;
    TSharedPtr<FString> SelectedPriority = PriorityComboBox->GetSelectedItem();
    if (SelectedPriority.IsValid())
    {
        Priority = *SelectedPriority;
    }
    else
    {
        Priority = "";  // default or empty priority, adjust as needed
    }

    // 2. Create a JSON payload with this data.
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    // Settings from Invo SDK Feilds
    const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

    JsonObject->SetNumberField("user_id", 4);
    JsonObject->SetNumberField("player_id", 4);
    JsonObject->SetNumberField("game_id", Settings->Game_ID);
    JsonObject->SetStringField("subject", Subject);
    JsonObject->SetStringField("message_body", Description);
    JsonObject->SetStringField("priority", Priority);
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
    if (Subject.IsEmpty() || Description.IsEmpty())
    {
        // Show a Windows alert box
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Subject and Description cannot be empty.")));
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


    /*
    // Set the callback
    HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
        {
            if (bSuccess && Response.IsValid())
            {
                // Log the response's content as a string.
                UE_LOG(LogTemp, Warning, TEXT("HTTP Response: %s"), *Response->GetContentAsString());

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
                // Handle the failure case. 
                UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid response."));

            }
        });
    */

    // Finally, send the HTTP request.
    //HttpRequest->ProcessRequest();

    return FReply::Handled();
}


FReply SInvoTicketWidget::OnCloseClicked()
{
    // Handle closing logic here
   
    CloseTicketWidget();
        
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

void SInvoTicketWidget::CloseTicketWidget() const
{
    // Find and close the parent window of this widget
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
    if (ParentWindow.IsValid())
    {
        ParentWindow->RequestDestroyWindow();
    }
}

bool SInvoTicketWidget:: ValidateResponseContent(const FString& ResponseContent)
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
