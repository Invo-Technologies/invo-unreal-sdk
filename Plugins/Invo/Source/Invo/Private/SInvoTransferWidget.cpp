// Fill out your copyright notice in the Description page of Project Settings.
#include "SInvoTransferWidget.h"
#include "InvoFunctions.h"
#include "InvoHttpManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
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
#include "Widgets/DeclarativeSyntaxSupport.h"



void SInvoTransferWidget::Construct(const FArguments& InArgs)
{
    //OnFunCurrencyAmountFetchedDBPDelegate = InArgs._OnCurrencyAmountFetchedBP;
    //InvoFunctionsInstance = NewObject<UInvoFunctions>();
    OnCurrencyAmountFetchedBPDelegate = InArgs._OnCurrencyAmountFetchedBP2;

    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Tap the Load Game Button First to Load Game")));

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
                                        .Padding(2.0f)
                                        [
                                            SNew(SButton)
                                                .Text(FText::FromString(TEXT("Load Games")))
                                                .OnClicked(this, &SInvoTransferWidget::OnSearchClicked)
                                        ]

                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(2.0f)
                                        [
                                            SAssignNew(SearchBox, SSearchBox)
                                                .HintText(FText::FromString(TEXT("Search Game")))
                                                .OnTextChanged(this, &SInvoTransferWidget::OnSearchTextChanged)
                                        ]
                                       
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(2.0f)
                                        [
                                            SAssignNew(GameListView, SListView<TSharedPtr<FGameInfo>>)
                                                .ItemHeight(5)
                                                .ListItemsSource(&FilteredGames) // The TArray that holds the filtered games
                                                .OnGenerateRow(this, &SInvoTransferWidget::OnGenerateRowForList)
                                                .OnSelectionChanged(this, &SInvoTransferWidget::OnGameSelected) // Bind the selection event

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
                                                .IsPassword(true)


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
    if (UInvoFunctions::CheckSecretsIni("PlayerID"))
    {
       
        const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
        TMap<FString, FString> FormData;
        
        FString PlayerID = UInvoFunctions::GetSecretsIniKeyValue("PlayerID");
        FormData.Add(TEXT("from_player_id"), PlayerID);
        FormData.Add(TEXT("from_game_id"), Settings->Game_ID);
        FormData.Add(TEXT("to_player_id"), TargetPlayerIDTextBox->GetText().ToString());
        FormData.Add(TEXT("to_game_id"), FString::FromInt(CurrentGame->GameID));
        FormData.Add(TEXT("amount"), DefaultCurrencyAmountTextBox->GetText().ToString());
        FormData.Add(TEXT("s_key"), PinTextBox->GetText().ToString());
        

        // 3. Directly make the HTTP request without using UInvoFunctions.
        FString Endpoint = "https://api.dev.ourinvo.com/v1/external/player/transfer"; // Replace with your actual server address
        FString HttpMethod = "POST";

        //4. Headers 
        TMap<FString, FString> Headers;

        // Create HTTP Request
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

        HttpRequest->SetURL(Endpoint);
        HttpRequest->SetVerb(HttpMethod);
        HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
        //HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
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


        // Alert for empty fields
        if (TargetPlayerIDTextBox->GetText().IsEmpty() || PinTextBox->GetText().IsEmpty())
        {
            // Show a Windows alert box
            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("TargetPlayerID or Pin cannot be empty.")));
            return FReply::Unhandled();  // Do not proceed further
        }


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
                    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Transfer Succeefully Completed.")));

                    UWorld* World = GWorld->GetWorld();


                    TSharedPtr<FJsonObject> OutDataObject;
                    TArray<TSharedPtr<FJsonValue>> OutDataArray;
                    FString OutMessage;
                    bool OutResults;
                    UInvoHttpManager::GetInstance()->ParseJSON(ResponseContent, OutDataObject, OutDataArray, OutMessage, OutResults);
                    FString CurrencyAmount = OutDataObject->GetStringField("current_balance");


                    // Check if the delegate is bound to avoid runtime errors
                    if (OnCurrencyAmountFetchedBPDelegate.IsBound())
                    {
                        // Execute the delegate with the currency amount
                        OnCurrencyAmountFetchedBPDelegate.Execute(CurrencyAmount);
                    }
                    else
                    {
                        // You might want to log or handle the case where nothing is bound to the delegate
                        UE_LOG(LogTemp, Warning, TEXT("No function bound to OnCurrencyAmountFetchedBP delegate."));
                    }

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

        //CloseTicketWidget();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerID already exsist. %s on line %d"), TEXT(__FILE__), __LINE__);
        UInvoFunctions::InvoShowPurchaseWidget();

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
    if (!JsonObject->HasField("message") || !JsonObject->HasField("result"))
    {
        UE_LOG(LogTemp, Error, TEXT("Mandatory fields are missing."));
        return false;
    }

    // 3. Validate Field Values
    // Example: Ensure "expectedField1" is a string and isn't empty
    FString expectedField1Value = JsonObject->GetStringField("message");
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


void SInvoTransferWidget::FetchGameInFromAPI()
{
     
    TArray<TSharedPtr<FGameInfo>> Results;


    UWorld* World = GWorld->GetWorld();
    const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();


    if (World)
    {

        if (!Settings->Player_ID.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Player %s is already registered"), *Settings->Player_ID);
            //return;
        }

        // Settings from Invo SDK Feilds

        TMap<FString, FString> FormData;

        // 3. Directly make the HTTP request without using UInvoFunctions.
        FString Endpoint = "https://api.dev.ourinvo.com/v1/external/game/allGames"; // Replace with your actual server address
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
            [&](const bool bSuccess, const FString& ResponseContent)
            {

                if (ValidateTransferResponseContent(ResponseContent))
                {
                    // Handle the valid response
                    // Log the response's content as a string.
                    FString StringbSuccess = bSuccess ? "True" : "False";
                   // UE_LOG(LogTemp, Warning, TEXT("HTTP Response: %s and is bSucess %s"), *ResponseContent, *StringbSuccess);

                    TSharedPtr<FJsonObject> OutDataObject;
                    TArray<TSharedPtr<FJsonValue>> OutDataArray;
                    FString OutMessage;
                    bool OutResults;
                    AllGames.Empty();

                    UInvoHttpManager::GetInstance()->ParseJSON(ResponseContent, OutDataObject, OutDataArray, OutMessage, OutResults);

                    if (!OutDataArray.IsEmpty())
                    
                       for (TSharedPtr<FJsonValue> Value : OutDataArray)
                       {
                           TSharedPtr<FJsonObject> ParsedJsonObject = Value->AsObject();
                           if (ParsedJsonObject.IsValid())
                           {
                               FGameInfo GameInfo;

                               // Assuming that your JSON object has these fields. Adjust as necessary.
                               GameInfo.GameName = ParsedJsonObject->GetStringField("game_name");
                               GameInfo.GameID = ParsedJsonObject->GetIntegerField("game_id");
                               // ... (populate other properties of GameInfo as necessary)
                               UE_LOG(LogTemp, Warning, TEXT(" Parsed Json Array response %s"), *GameInfo.GameName);
                               if (GameInfo.GameID && !GameInfo.GameName.IsEmpty())
                               {
                                  
                                    FilteredGames.Add(MakeShared<FGameInfo>(GameInfo));
                                    AllGames.Add(MakeShared<FGameInfo>(GameInfo));


                               }
       
                           }
                       }
                   // OutGameInfo = FilteredGames;
                   HandleGamesInfoReceived(FilteredGames);  // Call the handler with the parsed game info


                }
                else
                {
                    // Handle the invalid response
                    UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid response with response %s"), *ResponseContent);

                    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to get a valid response")));

                }
            });

    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No world"));
    }

   
    GameListView->SetVisibility(EVisibility::Visible);
}


// Function to populate the search bar or dropdown
void SInvoTransferWidget:: PopulateSearchBar(TArray<FGameInfo> GameInfos)
{
    for (auto& GameInfo : GameInfos)
    {
        // Add GameInfo.GameName to the search bar or dropdown
    }
}


// Function to get game ID based on game name
int SInvoTransferWidget:: GetGameIDFromName(FString GameName, TArray<FGameInfo> GameInfos)
{
    for (auto& GameInfo : GameInfos)
    {
        if (GameInfo.GameName == GameName)
        {
            return GameInfo.GameID;
        }
    }
    return -1; // Return -1 or any indicator for "not found"
}


bool SInvoTransferWidget::ValidateTransferResponseContent(const FString& ResponseContent)
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
    if (!JsonObject->HasField("result") || !JsonObject->HasField("message"))
    {
        UE_LOG(LogTemp, Error, TEXT("Mandatory fields are missing."));
        return false;
    }

    // 3. Validate Field Values
    // Example: Ensure "expectedField1" is a string and isn't empty
    FString expectedField1Value = JsonObject->GetStringField("message");
    if (expectedField1Value.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("'success' value shouldn't be empty."));
        // return false;
    }

    // Add more validations as needed

    return true;  // If all checks pass
}

// Function to handle the search button click
FReply SInvoTransferWidget::OnSearchClicked()
{
    // Get the search term from the text box
    FString SearchTerm = SearchBox->GetText().ToString();

    // Execute the search (you need to implement the FetchGameInfoFromAPI function to return a TArray of games)
    TArray<TSharedPtr <FGameInfo>> SearchResult;
    //TArray<TSharedPtr <FGameInfo>> SearchResult = FetchGameInFromAPI();

    // Clear the old results
    FilteredGames.Empty();
    //PriorityOptions.Empty();
    FetchGameInFromAPI();

    
    // Populate the FilteredGames array with the search results
    //for (auto& GameInfo : SearchResult)
    //{
    //    FilteredGames.Add(GameInfo);
    //    
    //}
    //
    //// Request the list view to update
    GameListView->RequestListRefresh();

    return FReply::Handled();
}



// Function to create the search bar UI
TSharedRef<SWidget> SInvoTransferWidget::CreateSearchBar()
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2.0f)
        [
            SAssignNew(SearchBox, SEditableTextBox)
                .HintText(FText::FromString(TEXT("Search Game")))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2.0f)
        [
            SNew(SButton)
                .Text(FText::FromString(TEXT("Search")))
                .OnClicked(this, &SInvoTransferWidget::OnSearchClicked)
        ];
}

void SInvoTransferWidget::OnSearchTextChanged(const FText& NewText)
{
    
    FilteredGames.Empty();


    FString SearchText = NewText.ToString();

        // Call the API to fetch games based on the search text    

    for (const TSharedPtr<FGameInfo>& GameInfo : AllGames)
    {
        if (GameInfo->GameName.Contains(SearchText, ESearchCase::IgnoreCase, ESearchDir::FromStart))
        {
            FilteredGames.Add(GameInfo);
        }
    }

    // Refresh the game list view to show the filtered results
    GameListView->RequestListRefresh();
    // Here, you can retrieve the data based on SearchText and update the suggestions
    // For example, querying a list of game names and updating a suggestion box

   
}
void SInvoTransferWidget::HandleTextChanged(const FText& NewText)
{
    //FilteredGames.Empty();
    //
    //FString Query = NewText.ToString();
    //TArray<TSharedPtr < FGameInfo>> Results = FetchGameInFromAPI();
    //for (const auto& Game : Results)
    //{
    //    if (Game->GameName.Contains(Query))
    //    {
    //        FilteredGames.Add(Game);
    //    }
    //}
    //
    //GameListView->RequestListRefresh();
}



TSharedRef<ITableRow> SInvoTransferWidget::OnGenerateRowForList(TSharedPtr<FGameInfo> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    // Return a new row widget for the game list view
    return SNew(STableRow<TSharedPtr<FGameInfo>>, OwnerTable)
        [
            // You might want to create a custom widget here to display game info
            SNew(STextBlock).Text(FText::FromString(InItem->GameName))
        ];
}

void SInvoTransferWidget::UpdateFilteredGames(const FText& InSearchText)
{
    // Clear the previous filtered games
    FilteredGames.Empty();

    // If there is no search text, just use all games
    if (InSearchText.IsEmpty())
    {
        
        FetchGameInFromAPI();
        return;
        //FilteredGames = GameList;
    }
    else
    {
        // Filter the games based on the search text
        for (const TSharedPtr<FGameInfo>& GameInfo : FilteredGames)
        {
            if (GameInfo->GameName.Contains(InSearchText.ToString()))
            {
                FilteredGames.Add(GameInfo);
            }
        }
    }

    // Refresh the list view to display the filtered games
    GameListView->RequestListRefresh();
}



void SInvoTransferWidget::HandleGamesInfoReceived(const TArray<TSharedPtr<FGameInfo>>& GamesInfo)
{
    FilteredGames.Empty();  // Clear the previous games info
    UE_LOG(LogTemp, Warning, TEXT("HandleGamesInfoReceived"))

    // Your implementation here, for example:
    for (const TSharedPtr<FGameInfo>& GameInfo : GamesInfo)
    {
        if (GameInfo.IsValid())
        {
            // Access game info using GameInfo->GameName, GameInfo->GameID, etc.
            // Add game info to your widget or perform other actions
            FilteredGames.Add(GameInfo);  // Adding to the FilteredGames array
            PriorityOptions.Add(MakeShared<FString>(GameInfo->GameName));

            UE_LOG(LogTemp, Warning, TEXT("FilteredGames"))


        }
    }

    // If you have a ListView or similar, you might want to refresh it here
    if (GameListView.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("GameListView"))

        GameListView->RequestListRefresh();
    }
}

void SInvoTransferWidget::OnGameSelected(TSharedPtr<FGameInfo> SelectedGame, ESelectInfo::Type SelectInfo)
{
    if (SelectedGame.IsValid())
    {
        // Set the selected game as the current game or perform any other action.
        CurrentGame = SelectedGame;
        UE_LOG(LogTemp, Warning, TEXT("Current game is selected is %s"), *CurrentGame->GameName);
        SearchBox->SetText(FText::FromString(CurrentGame->GameName));
        GameListView->SetVisibility(EVisibility::Collapsed);


        // You can close the pop-up here if you have a reference to it.
        // If it's a separate widget, you might need a delegate or event to communicate between widgets.
    }
}