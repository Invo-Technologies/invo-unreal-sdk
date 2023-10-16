// @AlexKissiJr for OurInvo  CopyRight 2023 SDK Unreal Engine Uplugin.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvoFunctions.generated.h"

// Define the plugin version number
#define INVO_PLUGIN_VERSION "1.0.0"


class SWebBrowser;
class SWindow;
class FJsonObject;
class SInvoTicketWidget;
class SInvoTransferWidget;
class SInvoPurchaseWidget;
class SInvoTradeWidget;
// For CallBack Functions 
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInvoAPICallCompleted, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCurrencyAmountFetchedBP, const FString&, CurrencyAmount);
DECLARE_DYNAMIC_DELEGATE_OneParam(FHttpResponseReceivedDelegate, const FString&, ResponseData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHttpResponseReceived, const FString&, ResponseContent);
// Declare the delegate (if using a multi-cast delegate, use DECLARE_DYNAMIC_MULTICAST_DELEGATE)
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHttpResponseReceived, bool, bWasSuccessful, FString, ResponseContent);




UENUM(BlueprintType)
enum class ENetDebugStatConnectionState : uint8
{
	/** Connection is invalid, possibly uninitialized. */
	Invalid,

	/** Connection permanently closed. */
	Closed,

	/** Connection is awaiting connection. */
	Pending,

	/** Connection is open. */
	Open
};

USTRUCT(BlueprintType)
struct FNetDebugStatBytes
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly, Category = "Net Debug Stat Bytes")
		float InBytes;

	UPROPERTY(BlueprintReadOnly, Category = "Net Debug Stat Bytes")
		float OutBytes;

	FNetDebugStatBytes()
	{
		InBytes = OutBytes = -1.f;
	}

	FNetDebugStatBytes(const float& NewInBytes, const float& NewOutBytes)
	{
		InBytes = NewInBytes;
		OutBytes = NewOutBytes;
	}
};

USTRUCT(BlueprintType)
struct FNetDebugStatPackets
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly, Category = "Net Debug Stat Packets")
		int32 InPackets;

	UPROPERTY(BlueprintReadOnly, Category = "Net Debug Stat Packets")
		int32 OutPackets;

	FNetDebugStatPackets()
	{
		InPackets = OutPackets = INDEX_NONE;
	}

	FNetDebugStatPackets(const int32& NewInPackets, const int32& NewOutPackets)
	{
		InPackets = NewInPackets;
		OutPackets = NewOutPackets;
	}
};


UENUM(BlueprintType)
enum class EBabeNodeSetting : uint8
{
	Option1    UMETA(DisplayName = "INVO Babe Node"),
	Option2    UMETA(DisplayName = "INVO Babe Node Sandbox"),
	// Add more options as needed
};


UENUM(BlueprintType)
enum class ETokenPair : uint8
{
	USDC UMETA(DisplayName = "USDC"),
	INVO UMETA(DisplayName = "INVO"),
	USDT UMETA(DisplayName = "USDT"),
	BTC UMETA(DisplayName = "BTC"),
	ETH UMETA(DisplayName = "ETH")
};

USTRUCT(BlueprintType)
struct FInvoAssetData
{
	GENERATED_BODY()


		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FInvoAssetData")
		FString Asset_ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FInvoAssetData")
		ETokenPair TP;

	/**
	* There is an additional “Reference” variable that is the (String) name of their Default_Currency.
	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FInvoAssetData")
		FString RDC;
};

USTRUCT(BlueprintType)
struct FCurrencyData
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly, Category = "FCurrencyData")
		FString CurrencyID;

	UPROPERTY(BlueprintReadOnly, Category = "FCurrencyData")
		FString GameID;

	UPROPERTY(BlueprintReadOnly, Category = "FCurrencyData")
		FString UserID;

	UPROPERTY(BlueprintReadOnly, Category = "FCurrencyData")
		FString CurrencyName;

	UPROPERTY(BlueprintReadOnly, Category = "FCurrencyData")
		FString CurrencyAmount;

	// ... Add any other fields you need, based on your database schema ...
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FFetchCurrenciesCompleted, const TArray<FCurrencyData>&, Currencies);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTicketSubmissionComplete,TArray<FString>, ResponseContent);


USTRUCT(BlueprintType)
struct FInvoRegistrationInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString UserType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString ContactEmail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyContactNumber;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyContact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyAddress1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyCity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyZipcode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString CompanyState;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString Country;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString WebsiteURL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString TaxIdentificationNumber;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Developer Registration Info")
		FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Registration Info")
		FString GameName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Registration Info")
		FString GameDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Registration Info")
		FString Genre;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Registration Info")
		FString Platform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Registration Info")
		FString ReleaseDate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Registration Info")
		FString GameURL;
};


USTRUCT(BlueprintType)
struct FInvoCreditCardInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// Credit Card Info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Credit Card Info")
		FString CardholderName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Credit Card Info")
		FString CardNumber;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Credit Card Info")
		FString CardExpirationDate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Credit Card Info")
		FString CVVCode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Credit Card Info")
		FString BillingAddress;
};

/*
UENUM(BlueprintType)
enum class EPriority : uint8
{
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High")
};

USTRUCT(BlueprintType) // This allows the struct to be used in Blueprints
struct FTicketData
{
	GENERATED_BODY() // This is required for Unreal's reflection system

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ticket")
	FString TicketTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ticket")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ticket")
	TEnumAsByte<EPriority> Priority; // Assuming you have an enum called EPriority

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ticket")
	FString AttachmentPath; // Path to the attached file, if any
};


*/

/**
 *
 */
 // NotBlueprintable, 
UCLASS(config = Game)
class INVO_API UInvoFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* public static UNetDebugStatsFunctions::GetMaxPacket
	* Gets the maximum packet size. -1 if return value is false.
	* @param OutMaxPacket [int32&] Maximum packet size.
	* @return [bool] True if valid UNetConnection was found from PlayerController. False otherwise.
	**/
	UFUNCTION(BlueprintPure, Category = "Fun ", meta = (WorldContext = "WorldContextObject"))
	static bool GetMaxPacket(const UObject* WorldContextObject, int32& OutMaxPacket);


	// Get the plugin version number
	static FString GetInvoPluginVersion();


	UFUNCTION(BlueprintCallable, Category = "Invo")
		static void PrintSDKVersionOnScreen();

	UInvoFunctions(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = Settings)
		bool Active = false;

	/**
	* Applications ID or Account_ID
	* @param OutMaxPacket [int32&] Maximum packet size.
	**/
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString Account_ID;

	/**
	* Applications ID or Game_ID
	* @param OutMaxPacket [int32&] Maximum packet size.
	* @return [bool] True if valid UNetConnection was found from PlayerController. False otherwise.
	**/
	UPROPERTY(config, EditAnywhere, Category = Settings)
	FString Game_ID;


	/**
	* BabeNod
	* Choose the primary (IN) RPC: this is the INVO Babe Node responsible for RPC and API requests between Game Server and Origin Node.
	* INVO Babe Node = Default
	* INVO Babe Node = Sandbox
	**/
	UPROPERTY(config, EditAnywhere, Category = Settings)
		EBabeNodeSetting Node;

	/**
	* This is the name of the game's default resource that will be traded on the INVO exchange.
	* This field will require the Game Developer to instantiate an unsigned integer equivalent to the U.S Dollar
	* amount of how much their currency is evaluated at in the GAME_ID’s trading pair within their Pool_ID.
	**/
	UPROPERTY(config, EditAnywhere, Category = Settings)
		FString Default_Currency;

	/**
	* This will be a randomly generated unsigned integer.
	**/
	UPROPERTY(config, EditAnywhere, Category = Settings)
		int32 Pool_ID;

	UPROPERTY(config, EditAnywhere, Category = Settings)
		FInvoAssetData AssetData;

	UPROPERTY(config, EditAnywhere, Category = Settings)
		FString Function_ID;

	// Registration Info Fields
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registration Info")
		FInvoRegistrationInfo DeveloperRegistrationInfo;

	// Credit Card Info Field
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Devoloper Credit Card Info")
		FInvoCreditCardInfo CreditCardInfo;


	UFUNCTION(BlueprintPure, Category = "Invo ", meta = (WorldContext = "WorldContextObject"))
	static bool InvoTestCall(const UObject* WorldContextObject, int32& OutMaxPacket);


	UFUNCTION(BlueprintCallable, Category = "Invo ", meta = (WorldContext = "WorldContextObject"))
	void InvoTestCallBeta(const UObject* WorldContextObjects);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static FInvoAssetData GetInvoUserSettingsInput();

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void GetInvoFacts();

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void OpenInvoWebPage(UObject* WorldContextObject, FString Url);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void OpenInvoInitWebPage();

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void CloseInvoWebBrowser();

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void GetInvoFunctionOne(FOnInvoAPICallCompleted OnCompleted);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void GetInvoFunctionTwo(FOnInvoAPICallCompleted OnCompleted);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void GetInvoFunctionThree(FOnInvoAPICallCompleted OnCompleted);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoAPICallFunction(FOnInvoAPICallCompleted OnCompleted);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void GetInvoEthBlockNumberBP(FOnInvoAPICallCompleted OnBlockNumberReceived);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void RegisterInvoGameDevBP(FOnInvoAPICallCompleted OnRegisteredDatabaseReceived);

	// Blueprint function for transferring currency
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void TransferCurrencyBP(int64 SourceGameID, int64 SourcePlayerID, int64 TargetGameID, int64 TargetPlayerID, float Amount, FString CurrencyName, FOnInvoAPICallCompleted OnTransferCompleted);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoTransferCurrencyWebViewBP(FOnInvoAPICallCompleted OnTransferCompleted);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void FetchCurrenciesForUserBP(int64 GameID, int64 PlayerID, FFetchCurrenciesCompleted Completed);

	static void GetInvoCurrencyAmountForPlayer(int64 GameID, int64 PlayerID, TFunction<void(const FString&)> OnCurrencyAmountFetched);

	UFUNCTION(BlueprintCallable, Category = "Invo", meta = (DisplayName = "Get Currency Amount For Player in BP"))
	static void GetInvoCurrencyAmountForPlayerBP(int64 GameID, int64 PlayerID, const FOnCurrencyAmountFetchedBP& OnCurrencyAmountFetchedBP);

	static bool bIsTransferCompleted;

	// Binds the F1 key to show the ticket widget
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoBindTicketUIKey();

	// Displays the SInvoTicketWidget
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoShowTicketWidget();

	// Displays the SInvoTicketWidget
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoShowGDPRWidget();

	// Displays the SInvoTicketWidget
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoShowTransferWidget();

	// Displays the SInvoPurchaseWidget
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoShowPurchaseWidget();

	// Displays the SInvoPurchaseWidget
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void InvoShowTradeWidget();


	// Used to call for any UI Class
	static void MakeHttpRequest(const FString& Url, const FString& HttpMethod, const FString& Content, TFunction<void(const bool, const FString&)> Callback);

	UFUNCTION(BlueprintCallable, Category = "Invo", meta = (DisplayName = "Make HTTP Request"))
	static void MakeHttpRequestBP(const FString& Url, const FString& HttpMethod, const FString& Content, FOnHttpResponseReceived OnResponseReceived);


	static FString EncryptData(const FString& DataToEncrypt, const FString& KeyString);
	static FString DecryptData(const FString& DataToDecrypt, const FString& KeyString);


	static TArray<uint8> StringToBytes(const FString& String);
	static FString BytesToString(const TArray<uint8>& Bytes);


	static FString PadStringToAESBlockSize(const FString& Data);
	static FString UnpadStringFromAESBlockSize(const FString& Data);

	static void StringToBytes(const FString& InString, TArray<uint8>& OutBytes);
	static void BytesToString(const TArray<uint8>& InBytes, FString& OutString);
	static bool HexToBytes(const FString& HexString, TArray<uint8>& OutBytes);

	static TArray<uint8> PadData(const TArray<uint8>& Data);

	static TArray<uint8> UnpadData(const TArray<uint8>& Data);
	
	static void UpdateSecretsIni(FString KeyVarable, FString KeyCodeValue);

	static bool IsExpired(const FString& Value, const FString& Value2);

	static bool InitializeAESKey(const FString& HexKeyString);

	static void TestEncryptDecrypt(const FString& PlainText, const FString& KeyString, FString& EncryptedHexStringOut);

	static FString BytesToHex(const TArray<uint8>& Bytes);

	//static TArray<uint8> AuthCodePlainTextBytes;


	bool IsHexDigit(TCHAR Character)
	{
		return (Character >= TEXT('0') && Character <= TEXT('9')) ||
			(Character >= TEXT('a') && Character <= TEXT('f')) ||
			(Character >= TEXT('A') && Character <= TEXT('F'));
	}


	/**
	* Converts a JSON string to a Map (Key-Value pairs)
	* @param JSONString The JSON formatted string
	* @return Map of Key-Value pairs parsed from the JSON
	*/
	UFUNCTION(BlueprintCallable, Category = "Invo")
	static TMap<FString, FString> InvoConvertJSONStringToMap(const FString& JSONString);

private:

	static void MakeHttpRequest(const FString& Url, const FString& Method, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback);

	static class UNetConnection* Internal_GetNetConnection(const UObject* WorldContextObject);

	void ExecuteOnGameThread(UObject* WorldContextObject);

	static void HandleJavaScriptCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget);

	static void HandleJavaScriptTestCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget);

	static bool CloseWebBrowser(const FString& Message);

	static void InvoAPIJsonReturnCall(const FString& City, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback);

	static void InvoAPIJsonReturn(const FString& EndPoint, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback);

	static void SimulateAPICall(FOnInvoAPICallCompleted OnCompleted);

	UFUNCTION()
	static void HandleURLChange(const FString& NewUrl);

	UFUNCTION()
	static void OpenWebView(const FString& Url);

	static TSharedRef<SWebBrowser> WebBrowser;
	static TSharedRef<SWindow> Window;

	static void GetInvoEthBlockNumber(TFunction<void(const FString&)> OnBlockNumberReceived);

	static void RegisterInvoGameDev(TFunction<void(const FString&)> OnRegisteredDatabaseReceived);

	static void TransferCurrency(int64 SourceGameID, int64 SourcePlayerID, int64 TargetGameID, int64 TargetPlayerID, float Amount, FString CurrencyName, TFunction<void(const FString&)> OnTransferCompleted);
	
	static void FetchCurrenciesForUser(int64 GameID, int64 PlayerID, TFunction<void(const TArray<FCurrencyData>&)> OnCurrenciesFetched);

	static TSharedPtr<SInvoTicketWidget> InvoTicketWidget;

	static TSharedPtr<SInvoTransferWidget> InvoTransferWidget;

	static TSharedPtr<SInvoPurchaseWidget> InvoPurchaseWidget;

	static TSharedPtr<SInvoTradeWidget> InvoTradeWidget;

	// Helper Method for Extracting Code from HTML
	static FString ExtractCodeFromHTMLSource(const FString& HtmlSource);

	// Helper Method for Extracting Code from a URL
	static FString ExtractCodeFromUrl(const FString& Url);
	

	//static TSharedRef<FJsonObject> JsonObjectTest;

protected:
	// Called when the game starts or when spawned
};

