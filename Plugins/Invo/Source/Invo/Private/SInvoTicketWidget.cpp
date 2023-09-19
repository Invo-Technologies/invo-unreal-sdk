#include "SInvoTicketWidget.h"
#include "InvoFunctions.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"

#include "Runtime/Online/HTTP/Public/Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"


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

    JsonObject->SetNumberField("user_id", 4);
    JsonObject->SetNumberField("player_id", 4);
    JsonObject->SetNumberField("game_id", 6);

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

    // Create HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

    HttpRequest->SetURL(Endpoint);
    HttpRequest->SetVerb(HttpMethod);
    HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(Payload);

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

    // Finally, send the HTTP request.
    HttpRequest->ProcessRequest();

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
