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

// For CallBack Functions 
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInvoAPICallCompleted, bool, bSuccess);


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
	static void OpenInvoWebPage(UObject* WorldContextObject);
	
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

private:

	static void MakeHttpRequest(const FString& Url, const FString& Method, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback);

	static class UNetConnection* Internal_GetNetConnection(const UObject* WorldContextObject);

	void ExecuteOnGameThread(UObject* WorldContextObject);

	static void HandleJavaScriptCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget);

	static void HandleJavaScriptTestCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget);

	static bool CloseWebBrowser(const FString& Message);

	static void InvoAPIJsonReturnCall(const FString& City, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback);

	static void SimulateAPICall(FOnInvoAPICallCompleted OnCompleted);

	UFUNCTION()
	static void HandleURLChange(const FString& NewUrl);

	static TSharedRef<SWebBrowser> WebBrowser;
	static TSharedRef<SWindow> Window;

	static void GetInvoEthBlockNumber(TFunction<void(const FString&)> OnBlockNumberReceived);

	static void RegisterInvoGameDev(TFunction<void(const FString&)> OnRegisteredDatabaseReceived);

	/**
	* This is for postgressSQL connections.
	**/


	//static TSharedRef<FJsonObject> JsonObjectTest;

protected:
	// Called when the game starts or when spawned
};

