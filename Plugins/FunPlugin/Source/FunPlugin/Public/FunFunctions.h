// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FunFunctions.generated.h"


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


/**
 * 
 */
// NotBlueprintable, 
UCLASS(config = Game)
class FUNPLUGIN_API UFunFunctions : public UBlueprintFunctionLibrary
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


	UFunFunctions(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(config, EditAnywhere, Category = Settings)
	bool Active = false;

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



	UFUNCTION(BlueprintPure, Category = "Invo ", meta = (WorldContext = "WorldContextObject"))
	static bool InvoTestCall(const UObject* WorldContextObject, int32& OutMaxPacket);


	UFUNCTION(BlueprintCallable, Category = "Invo ", meta = (WorldContext = "WorldContextObject"))
	void InvoTestCallBeta(const UObject* WorldContextObjects);

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static FInvoAssetData GetInvoUserSettingsInput();

	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void GetInvoFacts();


	UFUNCTION(BlueprintCallable, Category = "Invo")
	static void OpenInvoWebPage();


private:

	static class UNetConnection* Internal_GetNetConnection(const UObject* WorldContextObject);


protected:
	// Called when the game starts or when spawned



};
