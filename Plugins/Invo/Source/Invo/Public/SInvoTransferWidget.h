#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"

#include "Runtime/Slate/Public/Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

class SEditableTextBox;
class SSearchBox;

struct FGameInfo {
    FString GameName = "";
    int GameID{ 0 };
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGamesInfoReceived, const TArray<FGameInfo>&);
DECLARE_DELEGATE(FClosePopupDelegate);


class SInvoTransferWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoTransferWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetupWidget();
    void HandleHttpRequestCompleted(bool bWasSuccessful, const FString& ResponseContent);
    bool ValidateTransferResponseContent(const FString& ResponseContent);
    void FetchGameInFromAPI();

    void PopulateSearchBar(TArray<FGameInfo> GameInfos);
    int GetGameIDFromName(FString GameName, TArray<FGameInfo> GameInfos);

    FOnGamesInfoReceived OnGamesInfoReceived;

    


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
    TArray<TSharedPtr<TMap<FString, FString>>> GameNameList;
    //TSharedPtr<SComboBox<TMap<FString, FString>>> PriorityComboBox;

    //TSharedPtr<SSearchBox> SearchBox2;

    TArray<TSharedPtr<FGameInfo>> FilteredGames;
    TSharedPtr<SEditableTextBox> SearchBox;
    TSharedPtr<SListView<TSharedPtr<FGameInfo>>> GameListView;
    TArray<TSharedPtr<FGameInfo>> AllGames;
    TSharedPtr<FGameInfo> CurrentGame;


    // Event handlers
    FReply OnTransferClicked();
    FReply OnCloseClicked();
    FReply OnTogglePinMask();
    FReply OnSearchClicked();

    // Combo box related functions
    TSharedRef<SWidget> GeneratePriorityComboBoxWidget(TSharedPtr<FString> InItem);
    void OnPriorityChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    FText GetPriorityComboBoxText() const;
    FText GetPinMaskButtonText() const;

    void CloseTicketWidget() const;
    bool ValidateResponseContent(const FString& ResponseContent);

    bool isPinedMasked() const;

    // Function to create the search bar UI
    TSharedRef<SWidget> CreateSearchBar();

    // Pointer to the editable text box (to access the input text)

    void OnSearchTextChanged(const FText& NewText);

    void HandleTextChanged(const FText& NewText);
    TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FGameInfo> GameInfo, const TSharedRef<STableViewBase>& OwnerTable);

    void UpdateFilteredGames(const FText& InSearchText);

    void HandleGamesInfoReceived(const TArray<TSharedPtr<FGameInfo>>& GamesInfo);
    void OnGameSelected(TSharedPtr<FGameInfo> SelectedGame, ESelectInfo::Type SelectInfo);


};
