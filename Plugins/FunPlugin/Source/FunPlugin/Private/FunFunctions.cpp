// Alex Kissi Jr for OurInvo  CopyRight 2023 SDK Unreal Engine Uplugin.


#include "FunFunctions.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetConnection.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Runtime/Core/Public/Misc/Paths.h" // web brouser
#include "Runtime/Core/Public/HAL/PlatformProcess.h" // web browser
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"

#include "Serialization/JsonSerializer.h"

#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"

// Web braoser //
#include "Misc/Paths.h"
#include "Misc/FeedbackContext.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformProcess.h"
// web braoser 

#define GET_CONNECTION	UNetConnection* PlayerNetConnection = UFunFunctions::Internal_GetNetConnection(WorldContextObject)

// Called when the game starts or when spawned

bool UFunFunctions::GetMaxPacket(const UObject* WorldContextObject, int32& OutMaxPacket)
{
	if (GET_CONNECTION)
	{
		OutMaxPacket = PlayerNetConnection->MaxPacket;
		return true;
	}

	OutMaxPacket = INDEX_NONE;
	return false;
}


bool UFunFunctions::InvoTestCall(const UObject* WorldContextObject, int32& OutMaxPacket)
{

	// Print the Asset_ID
	//UE_LOG(LogTemp, Warning, TEXT("Asset_ID: %s"), AssetData.);
	return false;
}


void UFunFunctions::InvoTestCallBeta(const UObject* WorldContextObject)
{
	UE_LOG(LogTemp, Warning, TEXT("Asset_ID: %s"), *AssetData.Asset_ID);
	UE_LOG(LogTemp, Warning, TEXT("RDC: %s"), *AssetData.RDC);

	//for (const auto& TokenPair : AssetData.TP)
	//{
	//	//FString TokenPairString = ENUM_TO_STRING(ETokenPair, TokenPair); // Assume there's a function for this
	//	//UE_LOG(LogTemp, Warning, TEXT("Token Pair: %s"), *TokenPairString);
	//	
	//}

}


class UNetConnection* UFunFunctions::Internal_GetNetConnection(const UObject* WorldContextObject)
{
	const APlayerController* MyController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (MyController)
	{
		UNetConnection* MyNetConnection = MyController->GetNetConnection();
		return MyNetConnection;
	}

	return nullptr;
}

UFunFunctions::UFunFunctions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	

}


FInvoAssetData UFunFunctions::GetInvoUserSettingsInput()
{
	
	const UFunFunctions* Settings = GetDefault<UFunFunctions>();
	UE_LOG(LogTemp, Warning, TEXT("Testing Game_ID fisrt %s"), *Settings->Game_ID)


	if (Settings)
	{
		FInvoAssetData AssetData = Settings->AssetData;
		UE_LOG(LogTemp, Warning, TEXT("Testing Game_ID %s"), *Settings->Game_ID)

		return AssetData;
		// Access the properties of AssetData...
	}


	return Settings->AssetData;

}

void UFunFunctions::GetInvoFacts()
{
	const UFunFunctions* Settings = GetDefault<UFunFunctions>();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL("https://cat-fact.herokuapp.com/facts/");
	Request->SetVerb("GET");
	FString Asset_ID = Settings->AssetData.Asset_ID.Replace(TEXT(" "), TEXT("%20"));
	FString Url = FString::Printf(TEXT("https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"), *Asset_ID, *Settings->Game_ID);
	Request->SetURL(Url);
	Request->SetVerb("GET");
	//Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));

	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
		if (bWasSuccessful && Response.IsValid()) {
			FString ResponseString = Response->GetContentAsString();


			// Create a Json reader
			TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseString);

			// create a json array reader
			TSharedRef<TJsonReader<>> JsonArrayReader = TJsonReaderFactory<>::Create(ResponseString);


			TArray<TSharedPtr<FJsonValue>> Values;
			if (FJsonSerializer::Deserialize(JsonArrayReader, Values))  // Deserialize the json data given Reader and the actual object to deserialize.
			{
				for (auto& Value : Values)
				{
					TSharedPtr<FJsonObject> obj = Value->AsObject();

					if (obj.IsValid())
					{
						FString Fact = obj->GetStringField("text");
						UE_LOG(LogTemp, Warning, TEXT("Fact: %s"), *Fact);
						// Do something with the fact
					}
				}
			}

			// Use the Json reader to create a Json object
			TSharedPtr<FJsonObject> JsonObject;
			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
			{
				// Now you can access the values in the Json object like this:
				FString Fact = JsonObject->GetStringField("facts");
				UE_LOG(LogTemp, Warning, TEXT("Fact: %s"), *Fact);
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Error occurred"));
		}


		if (bWasSuccessful)
		{
			FString ResponseString = Response->GetContentAsString();

			UE_LOG(LogTemp, Warning, TEXT("FullString: %s"), *ResponseString);

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				// Assuming the weather data is an array.
				TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");
				for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
				{
					// Get the weather object
					TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();

					if (WeatherObject.IsValid())
					{
						FString Description = WeatherObject->GetStringField("description");
						UE_LOG(LogTemp, Warning, TEXT("Weather description: %s"), *Description);


					}
				}
			}
		}

		});

	Request->ProcessRequest();

}

void UFunFunctions::OpenInvoWebPage()
{
	FString URL = TEXT("https://www.google.com");
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}