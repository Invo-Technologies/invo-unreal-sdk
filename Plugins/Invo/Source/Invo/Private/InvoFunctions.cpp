// Alex Kissi Jr for OurInvo  CopyRight 2023 SDK Unreal Engine Uplugin.
#include "InvoFunctions.h"
//#include "WebBrowserWidget.h"

#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/OnlineReplStructs.h"
#include "Engine/NetConnection.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/WorldSettings.h"
#include "Runtime/Core/Public/Misc/MessageDialog.h"

#include "Misc/AES.h"
#include "Misc/Base64.h"

#include "InvoHttpManager.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"
//#include "ThirdParty/SQLite/include/sqlite3.h"


// UI Widgets 
#include "SInvoGDPRWidget.h"
#include "SInvoTicketWidget.h"
#include "SInvoTransferWidget.h"
#include "SInvoPurchaseWidget.h"
#include "SInvoTradeWidget.h"
#include "SKeyInputDialog.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Runtime/Core/Public/Misc/Paths.h" // web brouser
#include "Runtime/Core/Public/HAL/PlatformProcess.h" // web browser
#include "Http.h"
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
#include "Widgets/Layout/SBox.h"
//#include "WebBrowser/Public/SWebBrowser.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/WebBrowser/Public/WebBrowserModule.h"
#include "Widgets/SWeakWidget.h"
#include "Runtime/WebBrowser/Public/SWebBrowser.h"
#include "Runtime/WebBrowser/Public/WebBrowserModule.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/World.h"


// web braoser 
#include "Components/WidgetComponent.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanel.h"
#include "Runtime/SlateCore/Public/Widgets/SWindow.h"

#include "Blueprint/UserWidget.h"

// Threads
#include "Runtime/Core/Public/Async/TaskGraphInterfaces.h"
#include "Async/TaskGraphInterfaces.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Runtime/SlateCore/Public/Widgets/SWindow.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Engine/Public/TimerManager.h"



//#include "Engine/Plugins/Runtime/Database/DatabaseSupport/Public/DatabaseSupport.h"

// Postgres
//#include "Engine/Plugins/Runtime/Database/DatabaseSupport/SourceDatabaseSupport/Public/Database.h"

//#include "DatabaseSupport/Public/DatabaseSupport.h"

//#include "Plugins/Runtime/Database/DatabaseSupport/Source/DatabaseSupport/Public/Database.h"

//#include "Plugins/Runtime/Database/DatabaseSupport/Source/DatabaseSupport/Public/DatabaseSupport.h"


namespace InvoPrivate
{
	static TArray<uint8> AuthCodePlainTextBytes;
	static FAES::FAESKey Key;
}


// Initialize the static shared references
//TSharedRef<SWebBrowser> UInvoFunctions::WebBrowser = SNew(SWebBrowser);
//TSharedRef<SWindow> UInvoFunctions::Window = SNew(SWindow);

//Ticket Support
//TSharedPtr<SInvoTicketWidget> UInvoFunctions::InvoTicketWidget;
//TSharedPtr<SInvoTransferWidget> UInvoFunctions::InvoTransferWidget;

FString UInvoFunctions::CurrentBalance = TEXT(""); // Initialize the static variable
//UInvoFunctions::FOnBalanceUpdated UInvoFunctions::OnBalanceUpdated;

bool UInvoFunctions::bIsTransferCompleted = false;

#define GET_CONNECTION	UNetConnection* PlayerNetConnection = UInvoFunctions::Internal_GetNetConnection(WorldContextObject)

FString UInvoFunctions::GetInvoPluginVersion()
{
	return FString(TEXT(INVO_PLUGIN_VERSION));
}

void UInvoFunctions::PrintSDKVersionOnScreen()
{
	FString Version = GetInvoPluginVersion();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Invo SDK Version: %s"), *Version));
}

// Called when the game starts or when spawned



bool UInvoFunctions::InvoTestCall(const UObject* WorldContextObject, int32& OutMaxPacket)
{

	// Print the Asset_ID
	//UE_LOG(LogTemp, Warning, TEXT("Asset_ID: %s"), AssetData.);
	return false;
}


void UInvoFunctions::InvoTestCallBeta(const UObject* WorldContextObject)
{
	UE_LOG(LogTemp, Warning, TEXT("Asset_ID: %s"), *AssetData.Asset_ID);
	UE_LOG(LogTemp, Warning, TEXT("RDC: %s"), *AssetData.RDC);
	
	//for (const auto& TokenPair : AssetData.TP)
	//{
	//	//FString TokenPairString = ENUM_TO_STRING(ETokenPair, TokenPair); // Assume there's a function for this
	//	//UE_LOG(LogTemp, Warning, TEXT("Token Pair: %s"), *TokenPairString);
	//	
	//}
	//
}	//

class UNetConnection* UInvoFunctions::Internal_GetNetConnection(const UObject* WorldContextObject)
{
	const APlayerController* MyController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (MyController)
	{
		UNetConnection* MyNetConnection = MyController->GetNetConnection();
		return MyNetConnection;
	}

	return nullptr;
}

UInvoFunctions::UInvoFunctions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


FInvoAssetData UInvoFunctions::GetInvoUserSettingsInput()
{
	/*
	*
	* Please go into your Project Config Folder and in Your DefaultEngine.ini
	* Copy Paste the information below..
	* [/Script/Invo.UInvoFunctions]
	* SecretKey = YOUR_INVO_SECRETE_KEY
	*
	* [/Script/Invo.UInvoFunctions]
	  SECRETKEY=YOUR_KEY

	*/


	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FPaths::NormalizeFilename(SecretsIniFilePath);

	FString SecretKey;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), TEXT("SECRETKEY"), SecretKey, SecretsIniFilePath))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Secret Key: %s"), *SecretKey);
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("Failed to get secret key from config file"));
	}

	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
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

void UInvoFunctions::GetInvoFacts()
{
	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);

	FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);
	FString SecretKey;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), TEXT("SECRETKEY"), SecretKey, SecretsNormalizeConfigIniPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Secret Key: %s"), *SecretKey);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get secret key from config file"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Jesus %s"), *SecretKey);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString Asset_ID = Settings->AssetData.Asset_ID.Replace(TEXT(" "), TEXT("%20"));
	FString SecreteKeyNormalizeString = SecretKey.Replace(TEXT(" "), TEXT("%20"));
	FString Url = FString::Printf(TEXT("https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"), *Asset_ID, *SecreteKeyNormalizeString);
	Request->SetURL(Url);
	Request->SetVerb("GET");
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
						//UE_LOG(LogTemp, Warning, TEXT("Fact: %s"), *Fact);
						// Do something with the fact
					}
				}
			}

			// Use the Json reader to create a Json object
			TSharedPtr<FJsonObject> JsonObject;
			if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
			{
				// Now you can access the values in the Json object like this:
				//FString Fact = JsonObject->GetStringField("facts");
				//UE_LOG(LogTemp, Warning, TEXT("Fact: %s"), *Fact);
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

//	Request->ProcessRequest();

}


void UInvoFunctions::OpenWebView(const FString& Url, TSharedRef<SWindow>& Window, TSharedRef<SWebBrowser>& WebBrowser)
{
	const uint16 MajorVersion = FEngineVersion::Current().GetMajor();
	// Check the version of Unreal Engine.
	if (MajorVersion >= 5)
	{
		// Create a struct to hold the function parameters
	
		UWorld* World = GWorld->GetWorld();
	
		if (World)
		{
	
			Window = SNew(SWindow)
				.Title(FText::FromString(TEXT("Web Browser")))
				.ClientSize(FVector2D(800, 600));
	
	
			WebBrowser = SNew(SWebBrowser)
				.ShowControls(true)
				.ShowAddressBar(false)
				.OnUrlChanged_Lambda([&](const FText& NewUrlText) {
				FString NewUrl = NewUrlText.ToString();
				// Handle URL changes here
				UE_LOG(LogTemp, Warning, TEXT("Testing %s"), *NewUrl);
				
				if (!CheckSecretsIni("AuthCodeKey"))
				{
					FString AuthCode = ExtractCodeFromUrl(NewUrl);
					UInvoHttpManager::GetInstance()->SetAuthCode(AuthCode);
	
					//FString KeyString = TEXT("0123456789abcdef0123456789abcdef");
					FString HexKeyString = TEXT("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF"); // 64 hex characters
	
					FString EncryptDataAuthCode = EncryptData(AuthCode, HexKeyString);
					UpdateSecretsIni("AuthCodeKey", EncryptDataAuthCode);
	
					UE_LOG(LogTemp, Warning, TEXT("AuthCode is %s"), *AuthCode);
				}
	
				
	
				if (!CheckSecretsIni("PlayerID"))
				{
					// Create Player ID 
					FString UniqueIDStr;
					GenerateUniquePlayerID(UniqueIDStr);
					FString Message = FString::Printf(TEXT("OnTransferClicked with ID %s"), *UniqueIDStr);
					GEngine->AddOnScreenDebugMessage(1, 3.0, FColor::Green, Message);
					UE_LOG(LogTemp, Warning, TEXT("This log message is from file %s on line %d"), TEXT(__FILE__), __LINE__);
	
					UInvoHttpManager::GetInstance()->CreatePlayerID(UniqueIDStr);
				}
			
	
				HandleURLChange(NewUrl, Window, WebBrowser);
				
				
				});
	
	
			if (!Window->IsActive())
			{
				Window->SetContent(WebBrowser);
				FSlateApplication::Get().AddWindow(Window);
				WebBrowser->LoadURL(Url);
			}
	
		}
		else
		{
			// Handle the case when the world is not valid
			UE_LOG(LogTemp, Warning, TEXT(" UWorld not working"));
		}
	}
	else
	{
	
		TSharedPtr<SWebBrowser> WebBrowserWidget = SNew(SWebBrowser)
			.InitialURL(Url)
			.ShowControls(false)
			.ShowAddressBar(false);
	
		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget).PossiblyNullContent(WebBrowserWidget.ToSharedRef())
		);
	
		Window = SNew(SWindow)
			.Title(FText::FromString(TEXT("Web Browser")))
			.ClientSize(FVector2D(800, 600));
	
		WebBrowser = SNew(SWebBrowser)
			.ShowControls(true)
			.ShowAddressBar(false)
			.OnUrlChanged_Lambda([](const FText& NewUrlText) {
			FString NewUrl = NewUrlText.ToString();
			// Handle URL changes here
	
			if (NewUrl.Contains(TEXT("buttonClicked"))) {
				// Button on the website was clicked
				FString Message = TEXT("Button Clicked!");
				float Duration = 5.0f;
				FColor Color = FColor::Green;
				GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
			}
				});
	
		Window->SetContent(WebBrowser);
		FSlateApplication::Get().AddWindow(Window);
	
		if (Url.IsEmpty())
			WebBrowser->LoadURL(TEXT("http://localhost:5173/"));
		else
			WebBrowser->LoadURL(Url);
	
	}
}

void UInvoFunctions::HandleURLChange(const FString& NewUrl, TSharedRef<SWindow>& Window, TSharedRef<SWebBrowser>& WebBrowser)
{
	if (NewUrl.Contains(TEXT("closeButton"))) {
		UE_LOG(LogTemp, Warning, TEXT("Close Button Clicked "));
	
		// Button on the website was clicked
		FString Message = TEXT("closeButton  Clicked!");
		float Duration = 5.0f;
		FColor Color = FColor::Green;
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
					HandleJavaScriptCallback("closeButton", WebBrowser, Window);
				}
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	
		// Access the game thread
		FGraphEventRef Task_Render = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				// Perform game thread operations here
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
	
				}
			}, TStatId(), nullptr, ENamedThreads::ActualRenderingThread);
	}
	else if (NewUrl.Contains(TEXT("PageA")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Button Clicked "));
	
		// Button on the website was clicked
		FString Message = TEXT("Button Clicked!");
		float Duration = 5.0f;
		FColor Color = FColor::Green;
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	
	
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
					//HandleJavaScriptCallback("ButtonClicked", WebBrowser);
					HandleJavaScriptTestCallback("PageA", WebBrowser, Window);
				}
	
	
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	
	
		// Access the game thread
		FGraphEventRef Task_Render = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				// Perform game thread operations here
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
	
				}
	
			}, TStatId(), nullptr, ENamedThreads::ActualRenderingThread);
	}
	else if (NewUrl.Contains(TEXT("PageB")))
	{
	
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
					//HandleJavaScriptCallback("ButtonClicked", WebBrowser);
					HandleJavaScriptTestCallback("PageB", WebBrowser, Window);
				}
	
	
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	
	
	}
	else if (NewUrl.Contains(TEXT("Transfer_Completed")))
	{
	
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
					//HandleJavaScriptCallback("ButtonClicked", WebBrowser);
					//HandleJavaScriptTestCallback("Transfer", WebBrowser);
					UInvoFunctions::bIsTransferCompleted = true;
				}
	
	
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	
	
	}
	else if (NewUrl.Contains(TEXT("/external/auth/callback")))
	{
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
					//HandleJavaScriptCallback("ButtonClicked", WebBrowser);
					//HandleJavaScriptTestCallback("Transfer", WebBrowser);
					FSlateApplication::Get().RequestDestroyWindow(Window);
				}
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	
	
		/*
		FString HtmlContent;
		FString Url = TEXT("https://api.dev.ourinvo.com"); // Replace with your API endpoint URL
	
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->SetURL(Url);
	
		HttpRequest->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
			{
				if (bWasSuccessful && Response.IsValid())
				{
	
					FString HtmlContent = Response->GetContentAsString();
	
					// Now you have the HTML content in 'HtmlContent'
					UE_LOG(LogTemp, Warning, TEXT("HTML Content: %s"), *HtmlContent);
	
					// Parse the HTML content and extract the 'code' value as shown in the previous response.
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("HTTP Request Failed"));
				}
			});
	
		HttpRequest->ProcessRequest();
	
		*/
	
		/*
		
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				{
				//	HandleJavaScriptTestCallback("code", WebBrowser);
					UE_LOG(LogTemp, Warning, TEXT("Nwe URL is %s"), *NewUrl);
					// Assuming you have the HTML content as a FString named HtmlContent
					FString JsonString;
					// Find the start and end of the JSON string within the HTML content
					if (HtmlContent.FindChar('{', m) != INDEX_NONE && HtmlContent.FindLastChar('}', m) != INDEX_NONE)
					{
						// Extract the JSON string between '{' and '}'
						int32 StartIndex = HtmlContent.FindChar('{', m);
						int32 EndIndex = HtmlContent.FindLastChar('}', m);
	
						if (StartIndex != INDEX_NONE && EndIndex != INDEX_NONE)
						{
							JsonString = HtmlContent.Mid(StartIndex, EndIndex - StartIndex + 1);
	
							// Now you have the JSON string, which is: {"code":"4/0AfJohXlyZPD_fDU4yk10MWElowiN020oZF6rjVounr2EEc_4kpJ6WopKD1cMrSYI7Oubyg","message":"Code Saved"}
						}
					}
					
	
					// Parse the JSON string
					TSharedPtr<FJsonObject> JsonObject;
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
					if (FJsonSerializer::Deserialize(Reader, JsonObject))
					{
						FString CodeValue;
						if (JsonObject->TryGetStringField("code", CodeValue))
						{
							// CodeValue now contains the value of the "code" field
							UE_LOG(LogTemp, Warning, TEXT("Code Value: %s"), *CodeValue);
						}
					}
	
				}
	
	
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	
		*/
	}
	else 
	{
	
		UE_LOG(LogTemp, Warning, TEXT("None loop URL is %s"), *NewUrl);
	
	}
	
	// Add more conditions for other URL changes if needed
	// else if (NewUrl.Contains(TEXT("PageA"))) { ... }
	// else if (NewUrl.Contains(TEXT("PageB"))) { ... }
	// ...
}


void UInvoFunctions::OpenInvoWebPage(UObject* WorldContextObject, FString Url = "")
{
	//FString URL = TEXT("https://www.ourinvo.com");
	//FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	// React Local Sample Page Ui http://localhost:5173/
	// Get the Unreal Engine version.
	FString ReactWepPageUrl = "http://localhost:5173/";
	TSharedRef<SWindow> Window = SNew(SWindow);
	TSharedRef<SWebBrowser> WebBrowser = SNew(SWebBrowser);
	OpenWebView(ReactWepPageUrl, Window, WebBrowser);

}

void UInvoFunctions::OpenInvoInitWebPage()
{
	//FString URL = TEXT("https://www.ourinvo.com");
	//FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	// React Local Sample Page Ui http://localhost:5173/
	// Get the Unreal Engine version.
	FString ReactWepPageUrl = "https://api.dev.ourinvo.com/";
	TSharedRef<SWindow> Window = SNew(SWindow);
	TSharedRef<SWebBrowser> WebBrowser = SNew(SWebBrowser);
	OpenWebView(ReactWepPageUrl, Window, WebBrowser);
}



void UInvoFunctions::CloseInvoWebBrowser()
{
	// Call your function here using the provided parameters
	// Make sure to access any game-related objects/components safely on the game thread 
}

// Define the ExecuteOnGameThread function that will be called on the game thread
void UInvoFunctions::ExecuteOnGameThread(UObject* WorldContextObject)
{
	// Call your function here using the provided parameters
	// Make sure to access any game-related objects/components safely on the game thread
}


void UInvoFunctions::HandleJavaScriptCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget, TSharedRef<SWindow> Window)
{
	// Call your function here using the provided parameters
	// Make sure to access any game-related objects/components safely on the game thread
	if (!WebBrowserWidget.IsValid())
	{
		return; // Web browser widget is not valid, exit the function
	}

	if (Message == "closeButton")
	{
		// Handle the button click callback
		UE_LOG(LogTemp, Log, TEXT("Received JavaScript callback: ButtonClicked"));

		FSlateApplication::Get().RequestDestroyWindow(Window);

		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Green, TEXT("Curreent thread is not a game thread %s"));

	}
}


void UInvoFunctions::HandleJavaScriptTestCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget, TSharedRef<SWindow> Window)
{
	// Call your function here using the provided parameters
	// Make sure to access any game-related objects/components safely on the game thread
	if (!WebBrowserWidget.IsValid())
	{
		return; // Web browser widget is not valid, exit the function
	}
	
	if (Message == "PageA")
	{
		// Handle the button click callback
		UE_LOG(LogTemp, Log, TEXT("Received JavaScript callback: ButtonClicked"));
		FString JsonData = TEXT("");
	
		InvoAPIJsonReturnCall(TEXT("London"), JsonData, [](TSharedPtr<FJsonObject> JsonObject)
			{
				// Do something with JsonObject
				// This will be called when the HTTP request completes
	
				TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");
				for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
				{
					// Get the weather object
					TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();
	
					if (WeatherObject.IsValid())
					{
						FString Description = WeatherObject->GetStringField("description");
						UE_LOG(LogTemp, Warning, TEXT("Weather description: %s"), *Description);
						FString Message = FString::Printf(TEXT("Weather Descripions is %s"), *Description);
						GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					}
				}
	
			});
	
		// Access the game thread
		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{
				if (Window.ToSharedPtr().IsValid() && WebBrowserWidget.IsValid())
				{
					//FJsonObject JsonObject = InvoAPIJsonReturnCall();
	
				}
	
			}, TStatId(), nullptr, ENamedThreads::GameThread);
	}
	else if (Message == "PageB")
	{
		FString JsonData = TEXT("");
		InvoAPIJsonReturnCall(TEXT("San Diego"), JsonData, [](TSharedPtr<FJsonObject> JsonObject)
			{
				// Do something with JsonObject
				// This will be called when the HTTP request completes
	
				TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");
				for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
				{
					// Get the weather object
					TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();
	
					if (WeatherObject.IsValid())
					{
						FString Description = WeatherObject->GetStringField("description");
						UE_LOG(LogTemp, Warning, TEXT("Weather description: %s"), *Description);
						FString Message = FString::Printf(TEXT("Weather Descripions is %s"), *Description);
						GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					}
				}
	
			});
	
	}
	else if (Message == "auth")
	{
		FString JsonData = TEXT("");
		InvoAPIJsonReturn(TEXT("https://api.dev.ourinvo.com/"), JsonData, [](TSharedPtr<FJsonObject> JsonObject)
			{
				// Do something with JsonObject
				// This will be called when the HTTP request completes
	
				TArray<TSharedPtr<FJsonValue>> Data = JsonObject->GetArrayField("code");
				for (int32 Index = 0; Index != Data.Num(); ++Index)
				{
					// Get the weather object
					TSharedPtr<FJsonObject> DataObject = Data[Index]->AsObject();
	
					if (DataObject.IsValid())
					{
						FString Description = DataObject->GetStringField("code");
						UE_LOG(LogTemp, Warning, TEXT("Auth Code description: %s"), *Description);
						FString Message = FString::Printf(TEXT("Weather Descripions is %s"), *Description);
						GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					}
				}
	
			});
	
	}
}

bool UInvoFunctions::CloseWebBrowser(const FString& Message)
{
	// Call your function here using the provided parameters
	// Make sure to access any game-related objects/components safely on the game thread
	return true;
}

void UInvoFunctions::MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const TMap<FString, FString>& FormData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();


	// Set HTTP method (GET, POST, PUT, etc.)
	Request->SetVerb(HttpMethod);

	// Set the request URL
	Request->SetURL(URL);

	if (Headers.IsEmpty())
	{
		Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

	}
	else

	{
		for (const auto& Header : Headers)
		{
			Request->SetHeader(Header.Key, Header.Value);
		}
	}
	// Set headers, if any

	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);

	FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);
	FString AuthCodeKey;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), TEXT("AUTHCODEKEY"), AuthCodeKey, SecretsNormalizeConfigIniPath))
	{
		FString HexKeyString = TEXT("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF"); // 64 hex characters

		FString DecryptDataAuthCode = UInvoFunctions::DecryptData(AuthCodeKey, HexKeyString);
		UE_LOG(LogTemp, Warning, TEXT("Decrypted AuthCode Key: %s"), *DecryptDataAuthCode);
		if (!DecryptDataAuthCode.IsEmpty())
		{

			Request->SetHeader(TEXT("auth_code"), DecryptDataAuthCode);

		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthCode Key is empty: %s"), *AuthCodeKey);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Need to Initiliaze Invo SDK first.")));

			UInvoFunctions::OpenInvoInitWebPage();
			return;

		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get authcode key from config file"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Need to Initiliaze Invo SDK first.")));

		UInvoFunctions::OpenInvoInitWebPage();
		return;
	}

	// Format the payload as x-www-form-urlencoded
	FString Payload;
	for (const auto& Pair : FormData)
	{
		if (!Payload.IsEmpty())
		{
			Payload.Append(TEXT("&"));
		}
		Payload.Append(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value));
	}

	Request->SetContentAsString(Payload);

	// Bind the request's completion delegate
	//Request->OnProcessRequestComplete().BindUObject(this, &UInvoHttpManager::HttpRequestCompleted);
	Request->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if(bWasSuccessful && Response.IsValid())

				// Check the response code
				if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
				{
					// Deserialize the response into a JSON object
					FString ResponseString = Response->GetContentAsString();
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
					TSharedPtr<FJsonValue> JsonValue;
					if (FJsonSerializer::Deserialize(Reader, JsonValue))
					{
						if (JsonValue->Type == EJson::Object)
						{
							// Handle JSON object
							TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
							Callback(JsonObject);
						}
						else if (JsonValue->Type == EJson::Array)
						{
							// Handle JSON array
							TArray<TSharedPtr<FJsonValue>> JsonArray = JsonValue->AsArray();
							for (const TSharedPtr<FJsonValue>& Item : JsonArray)
							{
								if (Item->Type == EJson::Object)
								{
									TSharedPtr<FJsonObject> JsonObject = Item->AsObject();
									Callback(JsonObject);
								}
							}
						}
					}
					else
					{
						// JSON parsing failed
						UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
					}
				}
				else
				{
					// Unsuccessful API call
					UE_LOG(LogTemp, Error, TEXT("API call failed with status code: %d"), Response->GetResponseCode());
				}

			else

				// Network-related errors or other issues
				UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));

		});

	// Execute the request
	Request->ProcessRequest();

}

void UInvoFunctions::MakeHttpRequest(const FString& Url, const FString& Method, FString& JsonData /*= TEXT("")*/, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{
	// Instantiate the HTTP module
	FHttpModule* HttpModule = &FHttpModule::Get();

	// Create the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(Method);
	HttpRequest->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));

	// Set the JSON data in the request if it's not empty
	if (!JsonData.IsEmpty())
	{
		HttpRequest->SetContentAsString(JsonData);
	}

	// Bind a lambda function to process the HTTP response
	HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (bWasSuccessful && Response.IsValid())
			{
				// Check the response code
				if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
				{
					// Deserialize the response into a JSON object
					FString ResponseString = Response->GetContentAsString();
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
					TSharedPtr<FJsonValue> JsonValue;
					if (FJsonSerializer::Deserialize(Reader, JsonValue))
					{
						if (JsonValue->Type == EJson::Object)
						{
							// Handle JSON object
							TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
							Callback(JsonObject);
						}
						else if (JsonValue->Type == EJson::Array)
						{
							// Handle JSON array
							TArray<TSharedPtr<FJsonValue>> JsonArray = JsonValue->AsArray();
							for (const TSharedPtr<FJsonValue>& Item : JsonArray)
							{
								if (Item->Type == EJson::Object)
								{
									TSharedPtr<FJsonObject> JsonObject = Item->AsObject();
									Callback(JsonObject);
								}
							}
						}
					}
					else
					{
						// JSON parsing failed
						UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
					}
				}
				else
				{
					// Unsuccessful API call
					UE_LOG(LogTemp, Error, TEXT("API call failed with status code: %d"), Response->GetResponseCode());
				}
			}
			else
			{
				// Network-related errors or other issues
				UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
			}
		});

	// Send the HTTP request
	HttpRequest->ProcessRequest();
}

void UInvoFunctions::MakeHttpRequest(const FString& Url, const FString& HttpMethod, const FString& Content, TFunction<void(const bool, const FString&)> Callback)
{
	// Create HTTP Request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(HttpMethod);
	HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(Content);

	// Set the callback
	HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (bSuccess && Response.IsValid())
			{
				// Execute the delegate with the response's content as a string.
				Callback(bSuccess, Response->GetContentAsString());
			}
			else
			{
				// Handle the failure case. You can adjust this as per your requirements.
				Callback(bSuccess, TEXT("Failed to get a valid response."));
			}
		});


	// Finally, send the HTTP request.
	HttpRequest->ProcessRequest();
}

void UInvoFunctions::MakeHttpRequestBP(const FString& Url, const FString& HttpMethod, const FString& Content, FOnHttpResponseReceived OnResponseReceived)
{
	// ... (same setup as before)
		// Create HTTP Request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(HttpMethod);
	HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(Content);


	HttpRequest->OnProcessRequestComplete().BindLambda([OnResponseReceived](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (bSuccess && Response.IsValid())
			{
				OnResponseReceived.ExecuteIfBound(Response->GetContentAsString());
			}
			else
			{
				OnResponseReceived.ExecuteIfBound(TEXT("Failed to get a valid response."));
			}
		});


	// Finally, send the HTTP request.
	//HttpRequest->ProcessRequest();

	// ... (send the request)
}

void UInvoFunctions::InvoAPIJsonReturnCall(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const TMap<FString, FString>& FormData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{

}
void UInvoFunctions::InvoAPIJsonReturnCall(const FString& City, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{
	FJsonObject JsonRespObject;

	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

	FString Asset_ID = Settings->AssetData.Asset_ID.Replace(TEXT(" "), TEXT("%20"));
	//FString Asset_ID = "";

	FString Url = FString::Printf(TEXT("https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"), *City.Replace(TEXT(" "), TEXT("%20")), *UInvoFunctions::GetSecretsIniKeyValue("GameID"));

	MakeHttpRequest(Url, TEXT("GET"), JsonData, [Callback](TSharedPtr<FJsonObject> JsonObject)
		{
			if (JsonObject.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Its working on the you."));

				Callback(JsonObject);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to get weather data"));
			}
		});
}

void UInvoFunctions::InvoAPIJsonReturn(const FString& Endpoint, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{
	FJsonObject JsonRespObject;

	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

	MakeHttpRequest(Endpoint, TEXT("GET"), JsonData, [Callback](TSharedPtr<FJsonObject> JsonObject)
		{
			if (JsonObject.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Its working on the you."));

				Callback(JsonObject);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to get weather data"));
			}
		});
}

void UInvoFunctions::GetInvoFunctionOne(FOnInvoAPICallCompleted OnCompleted)
{
	FString JsonData = TEXT("");
	InvoAPIJsonReturnCall(TEXT("Los Angeles"), JsonData, [OnCompleted](TSharedPtr<FJsonObject> JsonObject)
		{
			// Do something with JsonObject
			// This will be called when the HTTP request completes

			TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");

			for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
			{
				// Get the weather object
				TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();

				if (WeatherObject.IsValid() && JsonObject->HasField("name"))
				{
					FString Description = WeatherObject->GetStringField("description");
					FString CityName = JsonObject->GetStringField("name");
					UE_LOG(LogTemp, Warning, TEXT("Weather description: %s in %s"), *Description, *CityName);
					FString Message = FString::Printf(TEXT("Weather Descripions is %s in %s"), *Description, *CityName);
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					bool bSuccess = true;
					OnCompleted.ExecuteIfBound(bSuccess);
				}
			}

		});
}

void UInvoFunctions::GetInvoFunctionTwo(FOnInvoAPICallCompleted OnCompleted)
{
	FString JsonData = TEXT("");
	InvoAPIJsonReturnCall(TEXT("San Diego"), JsonData, [OnCompleted](TSharedPtr<FJsonObject> JsonObject)
		{
			// Do something with JsonObject
				// This will be called when the HTTP request completes

			TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");

			for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
			{
				// Get the weather object
				TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();

				if (WeatherObject.IsValid() && JsonObject->HasField("name"))
				{
					FString Description = WeatherObject->GetStringField("description");
					FString CityName = JsonObject->GetStringField("name");
					UE_LOG(LogTemp, Warning, TEXT("Weather description: %s in %s"), *Description, *CityName);
					FString Message = FString::Printf(TEXT("Weather Descripions is %s in %s"), *Description, *CityName);
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					bool bSuccess = true;
					OnCompleted.ExecuteIfBound(bSuccess);

				}
			}

		});
}

void UInvoFunctions::GetInvoFunctionThree(FOnInvoAPICallCompleted OnCompleted)
{
	FString JsonData = TEXT("");
	InvoAPIJsonReturnCall(TEXT("London"), JsonData, [OnCompleted](TSharedPtr<FJsonObject> JsonObject)
		{
			// Do something with JsonObject
				// This will be called when the HTTP request completes

			TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");

			for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
			{
				// Get the weather object
				TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();


				if (WeatherObject.IsValid() && JsonObject->HasField("name"))
				{
					FString Description = WeatherObject->GetStringField("description");
					FString CityName = JsonObject->GetStringField("name");
					UE_LOG(LogTemp, Warning, TEXT("Weather description: %s in %s"), *Description, *CityName);
					FString Message = FString::Printf(TEXT("Weather Descripions is %s in %s"), *Description, *CityName);
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					bool bSuccess = true;
					OnCompleted.ExecuteIfBound(bSuccess);

				}
			}


		});

}

void UInvoFunctions::GetInvoFunctionFour(FOnCurrencyAmountFetchedBP OnCompleted)
{
	FString JsonData = TEXT("");
	InvoAPIJsonReturnCall(TEXT("London"), JsonData, [OnCompleted](TSharedPtr<FJsonObject> JsonObject)
		{
			// Do something with JsonObject
				// This will be called when the HTTP request completes

			TArray<TSharedPtr<FJsonValue>> WeatherData = JsonObject->GetArrayField("weather");

			for (int32 Index = 0; Index != WeatherData.Num(); ++Index)
			{
				// Get the weather object
				TSharedPtr<FJsonObject> WeatherObject = WeatherData[Index]->AsObject();


				if (WeatherObject.IsValid() && JsonObject->HasField("name"))
				{
					FString Description = WeatherObject->GetStringField("description");
					FString CityName = JsonObject->GetStringField("name");
					UE_LOG(LogTemp, Warning, TEXT("Weather description: %s in %s"), *Description, *CityName);
					FString Message = FString::Printf(TEXT("Weather Descripions is %s in %s"), *Description, *CityName);
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
					bool bSuccess = true;
					OnCompleted.ExecuteIfBound("100");

				}
			}


		});

}


void UInvoFunctions::InvoAPICallFunction(FOnInvoAPICallCompleted OnCompleted)
{
	SimulateAPICall(OnCompleted);
}

void UInvoFunctions::SimulateAPICall(FOnInvoAPICallCompleted OnCompleted)
{
	// Simulate the API call completion after 3 seconds
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([OnCompleted]()
		{
			// Simulate API call completed with success
			bool bSuccess = true;
			OnCompleted.ExecuteIfBound(bSuccess);
		});

	const float DelaySeconds = 3.0f;

	UWorld* World = GWorld->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, DelaySeconds, false);
	}

}


void UInvoFunctions::GetInvoEthBlockNumberBP(FOnInvoAPICallCompleted OnBlockNumberReceived)
{
	// Call the existing GetInvoEthBlockNumber function and pass a lambda to handle the result
	GetInvoEthBlockNumber([OnBlockNumberReceived](const FString& Result)
		{
			// Assign the result to the output parameter
			//BlockNumber = Result;
			bool bSuccess = true;
			UE_LOG(LogTemp, Warning, TEXT("Block Chain test %s"), *Result);
			OnBlockNumberReceived.ExecuteIfBound(bSuccess);
		});

}

void UInvoFunctions::GetInvoEthBlockNumber(TFunction<void(const FString&)> OnBlockNumberReceived)
{
	FString APIKey = "cb539443ebed4038bd4ae05f5223e49a";
	FString BlockchainUrl = FString::Printf(TEXT("https://mainnet.infura.io/v3/%s"), *APIKey);
	FString HttpMethod = "POST";

	// The JSON data to send in the request
	FString JsonData = TEXT("{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}");

	//Call the existing MakeHttpRequest function with the blockchain URL, HTTP method, and JSON data
	MakeHttpRequest(BlockchainUrl, HttpMethod, JsonData, [OnBlockNumberReceived](TSharedPtr<FJsonObject> JsonObject)
		{
			if (JsonObject.IsValid() && JsonObject->HasField("result"))
			{
				// Get the result (block number) from the JSON response
				FString BlockNumber = JsonObject->GetStringField("result");
				OnBlockNumberReceived(BlockNumber);
			}
		});
}

void UInvoFunctions::RegisterInvoGameDev(TFunction<void(const FString&)> OnRegisteredDatabaseReceived)
{
	// Create a connection to the database
	// Assuming global connection to the database is already established
	//pqxx::connection c("dbname=my_first_database user=postgres password=1234");

	// Endpoint and HTTP Method
	FString RegisterDataBaseEndpoint = FString::Printf(TEXT("http://127.0.0.1:3030/register"));
	FString HttpMethod = "POST";

	// Extract settings and developer information
	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
	auto DeveloperRegistrationInfo = Settings->DeveloperRegistrationInfo;
	//auto DeveloperRegistrationInfo = FInvoRegistrationInfo();


	// Create a JSON object for the new user table fields
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("user_type", DeveloperRegistrationInfo.UserType);
	JsonObject->SetStringField("company_name", DeveloperRegistrationInfo.CompanyName);
	JsonObject->SetStringField("contact_email", DeveloperRegistrationInfo.ContactEmail);
	JsonObject->SetStringField("company_addy_1", DeveloperRegistrationInfo.CompanyAddress1);
	JsonObject->SetStringField("company_city", DeveloperRegistrationInfo.CompanyCity);
	JsonObject->SetStringField("company_state", DeveloperRegistrationInfo.CompanyState);
	JsonObject->SetStringField("company_zipcode", DeveloperRegistrationInfo.CompanyZipcode);
	JsonObject->SetStringField("company_contact_number", DeveloperRegistrationInfo.CompanyContactNumber);
	JsonObject->SetStringField("company_contact", DeveloperRegistrationInfo.CompanyContact);
	JsonObject->SetStringField("company_ein_tax_ssn", DeveloperRegistrationInfo.TaxIdentificationNumber);
	JsonObject->SetStringField("password", DeveloperRegistrationInfo.Password);

	// Convert JSON object to string
	FString JsonData;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonData);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	// Call the existing MakeHttpRequest function
	MakeHttpRequest(RegisterDataBaseEndpoint, HttpMethod, JsonData, [OnRegisteredDatabaseReceived](TSharedPtr<FJsonObject> JsonObject)
		{
			// Handle the response (checking for 'company_name' as an indication of successful registration)
			if (JsonObject.IsValid() && JsonObject->HasField("company_name"))
			{
				FString Message = FString::Printf(TEXT("Registered successfully: %s"), *JsonObject->GetStringField("company_name"));
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
				OnRegisteredDatabaseReceived(JsonObject->GetStringField("company_name"));
			}
			else
			{
				FString Message = FString::Printf(TEXT("Registration failed."));
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, Message);
			}
		});

}

void UInvoFunctions::RegisterInvoGameDevBP(FOnInvoAPICallCompleted OnRegisteredDatabaseReceived)
{
	RegisterInvoGameDev([OnRegisteredDatabaseReceived](const FString& Result)
		{
			// Assign the result to the output parameter
			//BlockNumber = Result;
			bool bSuccess = true;
			UE_LOG(LogTemp, Warning, TEXT("Datbase test %s"), *Result);
			OnRegisteredDatabaseReceived.ExecuteIfBound(bSuccess);
		});
}

void UInvoFunctions::TransferCurrency(int64 SourceGameID, int64 SourcePlayerID, int64 TargetGameID, int64 TargetPlayerID, float Amount, FString CurrencyName, TFunction<void(const FString&)> OnTransferCompleted)
{
	// Endpoint and HTTP Method
	FString TransferEndpoint = FString::Printf(TEXT("http://127.0.0.1:3030/transfer"));
	FString HttpMethod = "POST";

	// Create a JSON object for the transfer details
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("source_game_id", SourceGameID);
	JsonObject->SetNumberField("source_player_id", SourcePlayerID);
	JsonObject->SetNumberField("target_game_id", TargetGameID);
	JsonObject->SetNumberField("target_player_id", TargetPlayerID);
	JsonObject->SetNumberField("amount", Amount);
	//JsonObject->SetStringField("currency_name", CurrencyName); // Hiding for now. Unsure if we should provide this parameter

	// Convert JSON object to string
	FString JsonData;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonData);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	// Call the existing MakeHttpRequest function
	MakeHttpRequest(TransferEndpoint, HttpMethod, JsonData, [OnTransferCompleted](TSharedPtr<FJsonObject> JsonObject)
		{
			// Handle the response
			if (JsonObject.IsValid() && JsonObject->HasField("status") && JsonObject->GetStringField("status") == "success")
			{
				FString Message = FString::Printf(TEXT("Transfer successful: %s"), *JsonObject->GetStringField("message"));
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
				OnTransferCompleted(JsonObject->GetStringField("message"));
			}
			else
			{
				FString Message = FString::Printf(TEXT("Transfer failed."));
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, Message);
				OnTransferCompleted(TEXT("Failed"));
			}
		});
}

void UInvoFunctions:: TransferCurrency(int64 TargetGameID, int64 TargetPlayerID, int Amount, FString CurrencyName, const FString& Pin,  TFunction<void(const FString&)> OnTransferCompleted)
{
	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
	TMap<FString, FString> FormData;


	if (Settings->Game_ID.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Enter Your GameID in the Invo Plugins Game ID Feild")));
		return;
	}

	if (CheckSecretsIni("GameId"))
	{
		//FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Enter Your GameID in the Secrets.Ini file in the Config Folder")));
		//return;
	}

	FString PlayerID = UInvoFunctions::GetSecretsIniKeyValue("PlayerID");
	FString Skey = UInvoFunctions::GetSecretsIniKeyValue("SKeyCode");
	FString Game_ID = UInvoFunctions::GetSecretsIniKeyValue("GameID");

	FormData.Add(TEXT("from_player_id"), PlayerID);
	FormData.Add(TEXT("from_game_id"), Settings->Game_ID);
	if (!CheckSecretsIni("SKeyCode"))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("SKey Code is not Set in Ini File.")));
		return;
	}

	if (FString::FromInt(TargetGameID).IsEmpty() || Pin.IsEmpty() || FString::FromInt(Amount).IsEmpty() || FString::FromInt(TargetGameID).IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Make Sure All Feilds are Filled.")));
		return;
	}
	
	FString DecryptedSKey = UInvoFunctions::DecryptData(Skey, Pin, "SKeyCode");
	
	FormData.Add(TEXT("to_player_id"), FString::FromInt(TargetPlayerID));
	FormData.Add(TEXT("to_game_id"),  FString::FromInt(TargetGameID));
	FormData.Add(TEXT("amount"), FString::FromInt(Amount));
	FormData.Add(TEXT("s_key"), DecryptedSKey);


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


	// Make the HTTP request
	MakeHttpRequest(Endpoint, HttpMethod, Headers, FormData, [OnTransferCompleted](TSharedPtr<FJsonObject> JsonObject)
		{

			if (JsonObject.IsValid() && JsonObject->HasField("message"))
			{
				FString CurrencyAmount;
				TSharedPtr<FJsonObject> Data = JsonObject->GetObjectField("data");

				CurrencyAmount = FString::FromInt(Data->GetIntegerField("current_balance"));

				OnTransferCompleted(CurrencyAmount);
				UE_LOG(LogTemp, Warning, TEXT("Json Data is valid is  %s"), *CurrencyAmount);
			}
			else
			{
				OnTransferCompleted(FString("Failed to retrieve currency amount."));
				UE_LOG(LogTemp, Warning, TEXT("Json Data is is not valid"));

			}
		});
}

void UInvoFunctions::TransferCurrencyBP(int64 SourceGameID, int64 SourcePlayerID, int64 TargetGameID, int64 TargetPlayerID, float Amount, FString CurrencyName, FOnInvoAPICallCompleted OnTransferCompleted)
{
	TransferCurrency(SourceGameID, SourcePlayerID, TargetGameID, TargetPlayerID, Amount, CurrencyName, [OnTransferCompleted](const FString& Result)
		{
			bool bSuccess = Result == "Currency transferred successfully";
			OnTransferCompleted.ExecuteIfBound(bSuccess);
		});
}

void UInvoFunctions::InvoTransferCurrencyBP(int64 TargetGameID, int64 TargetPlayerID, float Amount, FString CurrencyName, const FString& Pin, FOnCurrencyAmountFetchedBP OnTransferCompleted)
{
	TransferCurrency(TargetGameID, TargetPlayerID, Amount, CurrencyName, Pin, [OnTransferCompleted](const FString& Result)
		{
			OnTransferCompleted.ExecuteIfBound(Result);
		});
}


void UInvoFunctions::FetchCurrenciesForUser(int64 GameID, int64 PlayerID, TFunction<void(const TArray<FCurrencyData>&)> OnCurrenciesFetched)
{
	FString FetchCurrenciesEndpoint = FString::Printf(TEXT("http://127.0.0.1:3030/currencies?game_id=%lld&player_id=%lld"), GameID, PlayerID);
	FString HttpMethod = "GET";
	FString None = "";

	// Make the HTTP request
	MakeHttpRequest(FetchCurrenciesEndpoint, HttpMethod, None, [OnCurrenciesFetched](TSharedPtr<FJsonObject> JsonObject)
		{
			if (JsonObject.IsValid() && JsonObject->HasField("currency_id"))
			{

				TArray<FCurrencyData> Currencies;
				TSharedPtr<FJsonObject> CurrencyObject = JsonObject;

				FCurrencyData Currency;
				Currency.CurrencyID = CurrencyObject->GetStringField("currency_id");
				Currency.GameID = CurrencyObject->GetStringField("game_id");
				Currency.UserID = CurrencyObject->GetStringField("user_id");
				Currency.CurrencyName = CurrencyObject->GetStringField("currency_name");
				Currency.CurrencyAmount = CurrencyObject->GetStringField("currency_amount");

				// ... Extract any other fields you need ...

				Currencies.Add(Currency);

				OnCurrenciesFetched(Currencies);
			}
			else
			{
				TArray<FCurrencyData> EmptyArray;
				OnCurrenciesFetched(EmptyArray);
			}
		});
}


void UInvoFunctions::FetchCurrenciesForUserBP(int64 GameID, int64 PlayerID, FFetchCurrenciesCompleted Completed)
{
	FetchCurrenciesForUser(GameID, PlayerID, [Completed](const TArray<FCurrencyData>& Currencies)
		{
			Completed.ExecuteIfBound(Currencies);
		});
}

void UInvoFunctions::GetInvoCurrencyAmountForPlayer(int64 GameID, int64 PlayerID, TFunction<void(const FString&)> OnCurrencyAmountFetched)
{
	FString FetchCurrenciesEndpoint = FString::Printf(TEXT("http://127.0.0.1:3030/currencies?game_id=%lld&player_id=%lld"), GameID, PlayerID);
	FString HttpMethod = "GET";
	FString None = "";

	// Make the HTTP request
	MakeHttpRequest(FetchCurrenciesEndpoint, HttpMethod, None, [OnCurrencyAmountFetched](TSharedPtr<FJsonObject> JsonObject)
		{
			if (JsonObject.IsValid() && JsonObject->HasField("currency_amount"))
			{
				FString CurrencyAmount;


				CurrencyAmount = JsonObject->GetStringField("currency_amount");

				OnCurrencyAmountFetched(CurrencyAmount);
			}
			else
			{
				OnCurrencyAmountFetched(FString("Failed to retrieve currency amount."));
			}
		});
}

void UInvoFunctions::GetInvoCurrencyAmountForPlayer(TFunction<void(const FString&)> OnCurrencyAmountFetched)
{
	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
	TMap<FString, FString> FormData;

	if (Settings->Game_ID.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Enter Your GameID in the Invo Plugins Game ID Feild")));
		return;
	}

	//if (UInvoFunctions::GetSecretsIniKeyValue("GameID").IsEmpty())
	//{
	//	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Enter Your GameID in the Secrets.ini File in your Config Project Config Folder")));
	//	return;
	//}

	FString PlayerID = UInvoFunctions::GetSecretsIniKeyValue("PlayerID");
	FString Game_ID = UInvoFunctions::GetSecretsIniKeyValue("GameID");
	FormData.Add(TEXT("player_id"), PlayerID);
	FormData.Add(TEXT("game_id"), Settings->Game_ID);


	// 3. Directly make the HTTP request without using UInvoFunctions.
	FString Endpoint = "https://api.dev.ourinvo.com/v1/external/player/currentbalance"; // Replace with your actual server address
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



	for (const auto& Header : Headers)
	{
		UE_LOG(LogTemp, Warning, TEXT("Headers %s"), *Header.Value);

	}
	UE_LOG(LogTemp, Warning, TEXT("Payload is  %s"), *Payload);


	// Make the HTTP request

	MakeHttpRequest(Endpoint, HttpMethod, Headers, FormData, [OnCurrencyAmountFetched](TSharedPtr<FJsonObject> JsonObject)
		{

			if (JsonObject.IsValid() && JsonObject->HasField("message"))
			{
				FString CurrencyAmount;
				TSharedPtr<FJsonObject> Data = JsonObject->GetObjectField("data");

				CurrencyAmount = FString::FromInt(Data->GetIntegerField("current_balance"));
				
				OnCurrencyAmountFetched(CurrencyAmount);
				UE_LOG(LogTemp, Warning, TEXT("Json Data is valid is  %s"),*CurrencyAmount);
			}
			else
			{
				OnCurrencyAmountFetched(FString("Failed to retrieve currency amount."));
				UE_LOG(LogTemp, Warning, TEXT("Json Data is is not valid"));

			}
		});


}


void UInvoFunctions::GetInvoCurrencyAmountForPlayerBP(int64 GameID, int64 PlayerID, const FOnCurrencyAmountFetchedBP& OnCurrencyAmountFetchedBP)
{
	GetInvoCurrencyAmountForPlayer(GameID, PlayerID, [OnCurrencyAmountFetchedBP](const FString& CurrencyAmount)
		{
			OnCurrencyAmountFetchedBP.ExecuteIfBound(CurrencyAmount);
		});
}

void UInvoFunctions::GetInvoCurrencyAmountForPlayerBP(const FOnCurrencyAmountFetchedBP& OnCurrencyAmountFetchedBP)
{
	GetInvoCurrencyAmountForPlayer([OnCurrencyAmountFetchedBP](const FString& CurrencyAmount)
		{
			OnCurrencyAmountFetchedBP.ExecuteIfBound(CurrencyAmount);
		});
}


void UInvoFunctions::InvoTransferCurrencyWebViewBP(FOnInvoAPICallCompleted OnTransferCompleted)
{
	FString Url = "http://localhost:5173/transfer";
	TSharedRef<SWindow> Window = SNew(SWindow);
	TSharedRef<SWebBrowser> WebBrowser = SNew(SWebBrowser);
	OpenWebView(Url, Window, WebBrowser);
	OnTransferCompleted.ExecuteIfBound(UInvoFunctions::bIsTransferCompleted);

}




void UInvoFunctions::InvoBindTicketUIKey()
{

	UWorld* World = GWorld->GetWorld();
	if (World)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, "Invo Show InvoBindTicketUIKey ");

		APlayerController* PC = World->GetFirstPlayerController();
		if (PC && PC->InputComponent)
		{
			//PC->SetInputMode(FInputModeUIOnly());
			//PC->bShowMouseCursor = true;
			//UInvoFunctions::InvoShowTicketWidget();
			//PC->InputComponent->BindKey(EKeys::F1, IE_Pressed, PC, [PC]()
			//{
			//	UInvoFunctions::InvoShowTicketWidget();
			//});
		}
	}
}


/*
void UInvoFunctions::InvoShowTicketWidget(FHttpResponseReceived ResponseContent)
{

	// Ensure we don't already have the widget open
	//if (!InvoTicketWidget.IsValid())
	//{
	//	SAssignNew(InvoTicketWidget, SInvoTicketWidget);
	//
	//	if (GEngine && GEngine->GameViewport)
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, "Invo Game viewport open  ");
	//
	//		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(InvoTicketWidget.ToSharedRef()));
	//
	//	}
	//}


	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoTicket", "WindowTitle", "Invo Ticket System"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMinimize(true)
		.SupportsMaximize(true);

	Window->SetContent(SNew(SInvoTicketWidget));

	FSlateApplication::Get().AddWindow(Window);

}
*/

void UInvoFunctions::InvoShowTicketWidget()
{
	// Create a Ticket Widget instance
	
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoTicket", "WindowTitle", "Invo Ticket System"))
		.ClientSize(FVector2D(600, 500))
		.SupportsMinimize(true)
		.SupportsMaximize(true);
	
	Window->SetContent(SNew(SInvoTicketWidget));
	
	FSlateApplication::Get().AddWindow(Window);

	
	/*
	// Bind the ResponseContent delegate to the HttpRequestCompleted method of UInvoHttpManager
	UInvoHttpManager::GetInstance()->OnHttpRequestCompleted.AddLambda([ResponseContent](FHttpRequestPtr Request, FHttpResponsePtr HttpResponse, bool bSuccess)
		{
			// Extract content from the HttpResponse
			FString ContentString = HttpResponse.IsValid() ? HttpResponse->GetContentAsString() : TEXT("Invalid Response");

			// Execute the provided delegate
			ResponseContent.ExecuteIfBound(bSuccess, ContentString);
		});
	*/
	
}


void UInvoFunctions::InvoShowGDPRWidget()
{
	// Create a GDPR Widget instance
	TSharedRef<SWindow>	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoGDPR", "WindowTitle", "Invo GDPR"))
		.ClientSize(FVector2D(700, 200))
		.SupportsMinimize(true)
		.SupportsMaximize(true);
	
	Window->SetContent(SNew(SInvoGDPRWidget));
	
	FSlateApplication::Get().AddWindow(Window);
}

void UInvoFunctions::InvoShowSKeyInputDialog()
{
	// Create a GDPR Widget instance
	TSharedRef<SWindow>	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoSKey", "WindowTitle", "Invo Skey"))
		.ClientSize(FVector2D(400, 120))
		.SupportsMinimize(true)
		.SupportsMaximize(true);
	
	Window->SetContent(SNew(SKeyInputDialog));
	
	FSlateApplication::Get().AddWindow(Window);
}

void UInvoFunctions::InvoShowTransferWidget(const FOnCurrencyAmountFetchedBP& OnCurrencyAmountFetchedBP)
{
	// Create a InvoTransfer Widget instance
	TSharedRef<SWindow>	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoTransfer", "WindowTitle", "Invo Transfer System"))
		.ClientSize(FVector2D(500, 500))
		.SupportsMinimize(true)
		.SupportsMaximize(true);
	
	Window->SetContent(SNew(SInvoTransferWidget).OnCurrencyAmountFetchedBP2(OnCurrencyAmountFetchedBP));
	FSlateApplication::Get().AddWindow(Window);
}

void UInvoFunctions::InvoShowPurchaseWidget()
{
	// Create a InvoTransfer Widget instance
	TSharedRef<SWindow>	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoPurchase", "WindowTitle", "Invo Purchase System"))
		.ClientSize(FVector2D(800, 600))
		.SupportsMinimize(true)
		.SupportsMaximize(true);
	
	Window->SetContent(SNew(SInvoPurchaseWidget));
	FSlateApplication::Get().AddWindow(Window);
}

void UInvoFunctions::InvoShowTradeWidget()
{
	// Create a InvoTransfer Widget instance
	TSharedRef<SWindow>	Window = SNew(SWindow)

		.Title(NSLOCTEXT("InvoTrade", "WindowTitle", "Invo Trade System"))
		.ClientSize(FVector2D(500, 500))
		.SupportsMinimize(true)
		.SupportsMaximize(true);
	
	Window->SetContent(SNew(SInvoTradeWidget));
	FSlateApplication::Get().AddWindow(Window);
}

TMap<FString, FString> UInvoFunctions::InvoConvertJSONStringToMap(const FString& JSONString)
{
	TMap<FString, FString> ResultMap;


	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);


	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		for (auto Pair : JsonObject->Values)
		{
			if (Pair.Value.IsValid() && Pair.Value->Type == EJson::String)
			{
				ResultMap.Add(Pair.Key, Pair.Value->AsString());
			}
		}
	}

	return ResultMap;
}

FString UInvoFunctions::ExtractCodeFromHTMLSource(const FString& HtmlSource)
{
	FRegexPattern Pattern(TEXT("\"code\":\"([^\"]+)\""));
	FRegexMatcher Matcher(Pattern, HtmlSource);

	if (Matcher.FindNext())
	{
		return Matcher.GetCaptureGroup(1);
	}

	// Return an empty string if the pattern wasn't found
	return FString();
}

FString UInvoFunctions::ExtractCodeFromUrl(const FString& Url)
{
	// Split the URL by '?' to get the query parameters
	TArray<FString> UrlParts;
	Url.ParseIntoArray(UrlParts, TEXT("?"), true);
	FString CodeValue = "";
	if (UrlParts.Num() > 1)
	{
		// Split the query parameters by '&'
		TArray<FString> QueryParams;
		UrlParts[1].ParseIntoArray(QueryParams, TEXT("&"), true);

		for (FString Param : QueryParams)
		{
			// Split each parameter by '=' to get the key-value pair
			TArray<FString> KeyValue;
			Param.ParseIntoArray(KeyValue, TEXT("="), true);

			if (KeyValue.Num() > 1 && KeyValue[0] == TEXT("code"))
			{
				// If the key is "code", store its value

				FString LocalCodeValue = KeyValue[1];
				FString DecodedCode = FGenericPlatformHttp::UrlDecode(LocalCodeValue);
				CodeValue = DecodedCode; // Store t
				UE_LOG(LogTemp, Warning, TEXT("Extracted Code from URL: %s"), *CodeValue);

				// You can now use the CodeValue for any further processing you need
				return CodeValue;
			}
		}
	}
	return CodeValue;

}




FString UInvoFunctions::EncryptData(const FString& DataToEncrypt, const FString& KeyString)
{
	FAES::FAESKey Key;
	//TArray<uint8> KeyBytes;
	//HexToBytes(KeyString, KeyBytes);
	//FMemory::Memcpy(Key.Key, KeyBytes.GetData(), KeyBytes.Num());

	//FString PaddedData = PadStringToAESBlockSize(DataToEncrypt);

	TArray<uint8> DataBytes;
	//StringToBytes(PaddedData, DataBytes);

	FString EncryptedString;


	if (InitializeAESKey(KeyString))
	{
		// You can now use the Key for encryption and decryption
		//FAES::EncryptData(DataBytes.GetData(), DataBytes.Num(), Key);

		BytesToString(DataBytes, EncryptedString);
		UE_LOG(LogTemp, Warning, TEXT("check dtatabytes num %i."), DataBytes.Num());
		UE_LOG(LogTemp, Warning, TEXT("check encrpytiong num %s."), *EncryptedString);

		TestEncryptDecrypt(DataToEncrypt, KeyString, EncryptedString);

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize AES key."));
	}



	return EncryptedString;
}

FString UInvoFunctions::DecryptData(const FString& DataToDecrypt, const FString& KeyString,  const FString& KeyStringName)
{

	FString DecryptedString;
	FString HexKeyString;// 64 hex characters

	if (KeyString.IsEmpty())
	{
		HexKeyString = TEXT("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF"); // 64 hex characters

	}
	else
	{
		HexKeyString = KeyString;
	}
	if (InitializeAESKey(HexKeyString))
	{
		// You can now use the Key for encryption and decryption
		
		//FAES::DecryptData(InvoPrivate::AuthCodePlainTextBytes.GetData(), InvoPrivate::AuthCodePlainTextBytes.Num(), InvoPrivate::Key);

		//BytesToString(InvoPrivate::AuthCodePlainTextBytes, DecryptedString);

		FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
		FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);

		FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);
		FString AuthCodeKey;
		TArray<uint8> OutAuthCodeBytes;
		if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), *KeyStringName, AuthCodeKey, SecretsNormalizeConfigIniPath))
		{

			UE_LOG(LogTemp, Warning, TEXT("Encryped %s Hex Key: %s"),*KeyStringName, *AuthCodeKey);
			if (!AuthCodeKey.IsEmpty())
			{

				HexToBytes(AuthCodeKey, OutAuthCodeBytes);
				FAES::DecryptData(OutAuthCodeBytes.GetData(), OutAuthCodeBytes.Num(), InvoPrivate::Key);

				for (uint8 b : OutAuthCodeBytes)
				{
					if (b != 0) // Skip padding
					{
						DecryptedString += static_cast<TCHAR>(b);
					}
				}
				FString UnpaddedData = UnpadStringFromAESBlockSize(DecryptedString); // Removing the padding
				UE_LOG(LogTemp, Warning, TEXT("Jesus Decrypte %s Key padding removed: %s"), *KeyStringName, *UnpaddedData);
				UE_LOG(LogTemp, Warning, TEXT("Jesus Decrypte %s  Key: %s"), *KeyStringName, *DecryptedString);

			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s Key is empty: %s"),*KeyStringName, *AuthCodeKey);
				return TEXT("AuthCode Key is empty: %s");

			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get %s key from config file"),*KeyStringName);
			return TEXT("Failed to get authcode key from config file");
		}

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize AES key."));
		return TEXT("Failed to initialize AES key.");
	}


	return DecryptedString;
}


/*
FString UInvoFunctions::EncryptData(const FString& DataToEncrypt, const FString& KeyString)
{
	InitializeAESKey(KeyString, InvoPrivate::Key);

	TArray<uint8> DataBytes;
	StringToBytes(DataToEncrypt, DataBytes);

	// Ensure the data is a multiple of 16 bytes (128 bits)
	while (DataBytes.Num() % 16 != 0)
	{
		DataBytes.Add(0);
	}

	FAES::EncryptData(DataBytes.GetData(), DataBytes.Num(), InvoPrivate::Key);
	FString EncryptedHexString = BytesToHex(DataBytes);

	return EncryptedHexString;
}


FString UInvoFunctions::DecryptData(const FString& DataToDecrypt, const FString& KeyString)
{

	TArray<uint8> DataBytes;
	HexToBytes(DataToDecrypt, DataBytes);
	FAES::DecryptData(DataBytes.GetData(), DataBytes.Num(), InvoPrivate::Key);

	// Convert back to string
	FString DecryptedText;
	BytesToString(DataBytes, DecryptedText);
	DecryptedText;

	return DecryptedText;
}
*/
TArray<uint8> UInvoFunctions::StringToBytes(const FString& String)
{
	TArray<uint8> Bytes;
	Bytes.Append((uint8*)TCHAR_TO_UTF8(*String), String.Len() * sizeof(ANSICHAR));
	return Bytes;
}

FString UInvoFunctions::BytesToString(const TArray<uint8>& Bytes)
{
	return FString(UTF8_TO_TCHAR(Bytes.GetData()));
}



FString UInvoFunctions::PadStringToAESBlockSize(const FString& Data)
{
	FString PaddedData = Data;
	int32 NumBytesMissing = 16 - (Data.Len() % 16);
	for (int32 i = 0; i < NumBytesMissing; ++i)
	{
		PaddedData.AppendChar(' '); // Append spaces or any other padding character
	}
	return PaddedData;
}

FString UInvoFunctions:: UnpadStringFromAESBlockSize(const FString& Data)
{
	FString UnpaddedData = Data;
	UnpaddedData.TrimEndInline(); // Remove trailing spaces or the padding character used
	return UnpaddedData;
}

void UInvoFunctions::StringToBytes(const FString& InString, TArray<uint8>& OutBytes)
{
	// Converting FString to UTF-8
	FTCHARToUTF8 Convert(*InString);

	// Resizing the output array to fit the UTF-8 string
	OutBytes.SetNumUninitialized(Convert.Length());

	// Copying the UTF-8 bytes to the output array
	FMemory::Memcpy(OutBytes.GetData(), Convert.Get(), Convert.Length());
}

void UInvoFunctions::BytesToString(const TArray<uint8>& InBytes, FString& OutString)
{
	// Ensuring the byte array is null-terminated
	if (InBytes.Num() == 0 || InBytes.Last() != 0)
	{
		TArray<uint8> NullTerminatedBytes = InBytes;
		NullTerminatedBytes.Add(0);
		OutString = UTF8_TO_TCHAR(NullTerminatedBytes.GetData());
	}
	else
	{
		OutString = UTF8_TO_TCHAR(InBytes.GetData());
	}
}


bool UInvoFunctions::HexToBytes(const FString& HexString, TArray<uint8>& OutBytes)
{
	if (HexString.Len() % 2 != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid hex string size."));
		return false;
	}

	for (int32 Index = 0; Index < HexString.Len(); Index += 2)
	{
		FString ByteSubstring = HexString.Mid(Index, 2);
		uint8 Byte = FCString::Strtoi(*ByteSubstring, nullptr, 16);
		OutBytes.Add(Byte);
	}

	return true;
}


/*
bool UInvoFunctions::HexToBytes(const FString& HexString, TArray<uint8>& OutBytes)
{
	if (HexString.Len() % 2 != 0)
	{
		// Invalid hex string length
		return false;
	}

	for (int32 Index = 0; Index < HexString.Len(); Index += 2)
	{
		FString ByteString = HexString.Mid(Index, 2);
		uint8 Byte;
		if (ByteString.Len() == 2 && FChar::IsHexDigit(ByteString[0]) && FChar::IsHexDigit(ByteString[1]))
		{
			Byte = FCString::Atoi(*ByteString);
		}
		else
		{
			// Invalid hex digit
			return false;
		}

		OutBytes.Add(Byte);
	}

	return true;
}

*/

TArray<uint8> UInvoFunctions:: PadData(const TArray<uint8>& Data)
{
	TArray<uint8> PaddedData = Data;
	int PaddingSize = 16 - (Data.Num() % 16);
	for (int i = 0; i < PaddingSize; ++i)
	{
		PaddedData.Add(PaddingSize);
	}
	return PaddedData;
}

TArray<uint8> UInvoFunctions:: UnpadData(const TArray<uint8>& Data)
{
	TArray<uint8> UnpaddedData = Data;
	int LastByte = Data.Last();
	for (int i = 0; i < LastByte; ++i)
	{
		UnpaddedData.Pop();
	}
	return UnpaddedData;
}

void UInvoFunctions::UpdateSecretsIni(FString KeyVariable, FString KeyCodeValue)
{
	// Specify the path to the Secrets.ini file
	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);
	FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);

	// Get the existing value
	FString ExistingValue;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), *KeyVariable, ExistingValue, SecretsNormalizeConfigIniPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Existing Value: %s"), *ExistingValue);

		// Check whether the KeyCodeValue is expired (you might need to define IsExpired function)
		if (ExistingValue != KeyCodeValue)
		{
			// If expired, update the value
			GConfig->SetString(
				TEXT("/Script/Invo.UInvoFunctions"),
				*KeyVariable,
				*KeyCodeValue,
				SecretsNormalizeConfigIniPath
			);
			GConfig->Flush(false, SecretsNormalizeConfigIniPath);
			UE_LOG(LogTemp, Warning, TEXT("Updated Value: %s"), *KeyCodeValue);
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Value is not expired. No update performed."));
			GConfig->SetString(
				TEXT("/Script/Invo.UInvoFunctions"),
				*KeyVariable,
				*KeyCodeValue,
				SecretsNormalizeConfigIniPath
			);
			GConfig->Flush(false, SecretsNormalizeConfigIniPath);
			UE_LOG(LogTemp, Warning, TEXT("Added New Key: %s with Value: %s"), *KeyVariable, *KeyCodeValue);
		}
	}
	else // If the KeyVariable does not exist, add it
	{
		GConfig->SetString(
			TEXT("/Script/Invo.UInvoFunctions"),
			*KeyVariable,
			*KeyCodeValue,
			SecretsNormalizeConfigIniPath
		);
		GConfig->Flush(false, SecretsNormalizeConfigIniPath);
		UE_LOG(LogTemp, Warning, TEXT("Added New Key: %s with Value: %s"), *KeyVariable, *KeyCodeValue);
	}
}

bool UInvoFunctions::CheckSecretsIni(FString KeyVariable)
{
	// Specify the path to the Secrets.ini file
	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);
	FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);

	// Get the existing value
	FString ExistingValue;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), *KeyVariable, ExistingValue, SecretsNormalizeConfigIniPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Existing Value: %s"), *ExistingValue);
		return true;
	}
	else // If the KeyVariable does not exist, add it
	{
		UE_LOG(LogTemp, Warning, TEXT("Existing Value: %s does not exsist"), *ExistingValue);
		return false;
	}
	return true;
}

FString UInvoFunctions::GetSecretsIniKeyValue(const FString& KeyVariable)
{
	// Specify the path to the Secrets.ini file
	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);
	FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);

	// Get the existing value
	FString ExistingValue;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), *KeyVariable, ExistingValue, SecretsNormalizeConfigIniPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Existing Value: %s"), *ExistingValue);
		return ExistingValue;
	}
	else // If the KeyVariable does not exist, add it
	{
		UE_LOG(LogTemp, Warning, TEXT("Secrets Key %s Value Value: does not exsist"),*KeyVariable);
		return ExistingValue;
	}
	return ExistingValue;
}


bool UInvoFunctions::IsExpired(const FString& Value, const FString& Value2)
{
	// Here, you should implement the logic to check whether the value is expired.
	// Return true if expired, false otherwise.

	// Example:
	// return Value == TEXT("expired"); // Replace this with your actual condition
	return Value == Value2; // Placeholder, replace with your actual code
}

/*
bool UInvoFunctions::InitializeAESKey(const FString& HexKeyString)
{
	if (HexKeyString.Len() != FAES::FAESKey::KeySize * 2) // Each byte is represented by 2 hex characters
	{
		// Invalid hex string size
		UE_LOG(LogTemp, Error, TEXT("Invalid hex key size."));
		return false;
	}

	for (int32 Index = 0; Index < HexKeyString.Len(); Index += 2)
	{
		TCHAR Char1 = HexKeyString[Index];
		TCHAR Char2 = HexKeyString[Index + 1];

		if (FChar::IsHexDigit(Char1) && FChar:: IsHexDigit(Char2))
		{
			int32 HighNibble = Char1 <= TEXT('9') ? Char1 - TEXT('0') : Char1 - TEXT('A') + 10;
			int32 LowNibble = Char2 <= TEXT('9') ? Char2 - TEXT('0') : Char2 - TEXT('A') + 10;

			InvoPrivate::Key.Key[Index / 2] = (HighNibble << 4) + LowNibble;
		}
		else
		{
			// Invalid character in hex string or conversion failed
			UE_LOG(LogTemp, Error, TEXT("Failed to convert hex key to bytes."));
			return false;
		}
	}

	return true;
}
*/

bool UInvoFunctions::InitializeAESKey(const FString& KeyString)
{
	if (KeyString.Len() == FAES::FAESKey::KeySize * 2) // Hex key
	{
		for (int32 Index = 0; Index < KeyString.Len(); Index += 2)
		{
			TCHAR Char1 = KeyString[Index];
			TCHAR Char2 = KeyString[Index + 1];

			if (FChar::IsHexDigit(Char1) && FChar::IsHexDigit(Char2))
			{
				int32 HighNibble = Char1 <= TEXT('9') ? Char1 - TEXT('0') : Char1 - TEXT('A') + 10;
				int32 LowNibble = Char2 <= TEXT('9') ? Char2 - TEXT('0') : Char2 - TEXT('A') + 10;

				InvoPrivate::Key.Key[Index / 2] = (HighNibble << 4) + LowNibble;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to convert hex key to bytes."));
				return false;
			}
		}
	}
	else if (KeyString.Len() == 6) // 6-digit key
	{
		for (int32 Index = 0; Index < KeyString.Len(); ++Index)
		{
			TCHAR Char = KeyString[Index];

			if (FChar::IsDigit(Char))
			{
				InvoPrivate::Key.Key[Index] = Char;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid character in 6-digit key."));
				return false;
			}
		}

		// Padding the rest of the key with zeros (or any other value)
		for (int32 Index = KeyString.Len(); Index < FAES::FAESKey::KeySize; ++Index)
		{
			InvoPrivate::Key.Key[Index] = 0;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid key size."));
		return false;
	}

	return true;
}

void UInvoFunctions:: TestEncryptDecrypt(const FString& PlainText, const FString& KeyString, FString& EncryptedHexStringOut)
{
	// 1. Generate a 256-bit key and convert it to FAESKey
	//FString KeyString = TEXT("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF"); // Example 256-bit key as a string
	// Ensure Key.Key is filled with bytes from KeyString (you might want to convert hex to bytes if necessary)


	if (!InitializeAESKey(KeyString))
		return;

	// 2. Define a string to encrypt
	//FString PlainText = TEXT("4/0AfJohXkqyycBbdQhBEnAQY_4fErjMbDqLSdf8mn8tIhhmaouH8Kum4c8yc9US_OXnbuL4g");

	// 3. Convert the string into a byte array (UTF-8 example)
	for (TCHAR c : PlainText)
	{
		InvoPrivate::AuthCodePlainTextBytes.Add(static_cast<uint8>(c));
	}

	// Ensure the data is a multiple of 16 bytes (128 bits)
	while (InvoPrivate::AuthCodePlainTextBytes.Num() % 16 != 0)
	{
		InvoPrivate::AuthCodePlainTextBytes.Add(0);
	}

	// 4. Encrypt the data
	FAES::EncryptData(InvoPrivate::AuthCodePlainTextBytes.GetData(), InvoPrivate::AuthCodePlainTextBytes.Num(), InvoPrivate::Key);

	// Printing the encrypted data as a hex string
	FString EncryptedHexString = BytesToHex(InvoPrivate::AuthCodePlainTextBytes);
	UE_LOG(LogTemp, Warning, TEXT("Encrypted Hex String: %s"), *EncryptedHexString);
	EncryptedHexStringOut = EncryptedHexString;



	// 5. Decrypt the data back (for testing)
	//FAES::DecryptData(InvoPrivate::AuthCodePlainTextBytes.GetData(), InvoPrivate::AuthCodePlainTextBytes.Num(), InvoPrivate::Key);

	TArray<uint8> BytesOut;
	HexToBytes(EncryptedHexStringOut, BytesOut);

	FAES::DecryptData(BytesOut.GetData(), BytesOut.Num(), InvoPrivate::Key);

	// Convert back to string and compare with the original
	FString DecryptedText;
	for (uint8 b : BytesOut)
	{
		if (b != 0) // Skip padding
		{
			DecryptedText += static_cast<TCHAR>(b);
		}
	}

	if (DecryptedText == PlainText)
	{
		UE_LOG(LogTemp, Warning, TEXT("Encryption and decryption test passed. and Text is %s"),*DecryptedText);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Encryption and decryption test failed."));
	}
}

FString UInvoFunctions:: BytesToHex(const TArray<uint8>& Bytes)
{
	FString HexString;
	for (uint8 Byte : Bytes)
	{
		HexString.Append(FString::Printf(TEXT("%02x"), Byte));
	}
	return HexString;
}

void UInvoFunctions::GenerateUniquePlayerID(FString& OutUniquePlayerID)
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World && World->IsGameWorld())
		{
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World,0);
			if (PlayerController)
			{
				uint32 UniqueID = PlayerController->GetUniqueID();
				FString UniqueIDString = FString::FromInt(UniqueID);
				FString Message = FString::Printf(TEXT("Creating Unique PlayerID for SDK %s"), *UniqueIDString);
				GEngine->AddOnScreenDebugMessage(1, 3.0, FColor::Green, Message);
				OutUniquePlayerID = UniqueIDString;

				FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
				FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);

				FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);
				FString PlayerIdKey;
				TArray<uint8> OutAuthCodeBytes;
				if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), TEXT("PLAYERID"), PlayerIdKey, SecretsNormalizeConfigIniPath))
				{

					UE_LOG(LogTemp, Warning, TEXT("Encryped AuthCode Hex Key: %s"), *PlayerIdKey);
					if (!PlayerIdKey.IsEmpty())
					{

					}
					else
					{

						UE_LOG(LogTemp, Error, TEXT("AuthCode Key is empty: %s"), *PlayerIdKey);
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to get authcode key from config file"));
				}

				return;
				
			}
		}
	}
	
}

bool UInvoFunctions::FillBPObjectFromJSON(const FString& JSONString, UObject* BPObject)
{
	if (BPObject == nullptr)
	{
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		//JsonObjectConverter::JsonObjectToUStruct(JsonObject, BPObject->GetClass(), BPObject, 0, 0);
		return true;
	}

	return false;
}

void UInvoFunctions::InvoGetCurrentBalance()
{
	FString CurrencyBalance;
	const UInvoFunctions* SettingsBalance = GetDefault<UInvoFunctions>();
	if (SettingsBalance->Game_ID.IsEmpty())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Yellow, TEXT("Need to Set GameID in Secrets.Ini in your Project Config Folder "));
		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Yellow, TEXT("Need to Set GameID in Invo Plugin Settings "));
		return;

	}
	else
	{
		if (UInvoFunctions::CheckSecretsIni("PlayerID"))
		{

			const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
			TMap<FString, FString> FormData;

			FString PlayerID = UInvoFunctions::GetSecretsIniKeyValue("PlayerID");
			FormData.Add(TEXT("player_id"), PlayerID);
			FormData.Add(TEXT("game_id"), Settings->Game_ID);


			// 3. Directly make the HTTP request without using UInvoFunctions.
			FString Endpoint = "https://api.dev.ourinvo.com/v1/external/player/currentbalance"; // Replace with your actual server address
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



			for (const auto& Header : Headers)
			{
				UE_LOG(LogTemp, Warning, TEXT("Headers %s"), *Header.Value);

			}
			UE_LOG(LogTemp, Warning, TEXT("Payload is  %s"), *Payload);

			// Make the HTTP Request
		UInvoHttpManager::GetInstance()->MakeHttpRequest(Endpoint, HttpMethod, Headers, FormData,
				[&](const bool bSuccess, const FString& ResponseContent)
				{

					if (UInvoHttpManager::GetInstance()->ValidateHttpManagerResponseContent(ResponseContent))
					{
						// Handle the valid response
						// Log the response's content as a string.
						FString StringbSuccess = bSuccess ? "True" : "False";
						UE_LOG(LogTemp, Warning, TEXT("HTTP Response: %s and is bSucess %s"), *ResponseContent, *StringbSuccess);


						TSharedPtr<FJsonObject> OutDataObject;
						TArray<TSharedPtr<FJsonValue>> OutDataArray;
						FString OutMessage;
						bool OutResults;
						UInvoHttpManager::GetInstance()-> ParseJSON(ResponseContent, OutDataObject, OutDataArray, OutMessage, OutResults);
						FString Amount = OutDataObject->GetStringField("current_balance");
						
						//OutBalance = Amount;
						UE_LOG(LogTemp, Warning, TEXT("Currenace balance %s"), *Amount);
						//OnBalanceReceived.ExecuteIfBound(Amount);
						//OnBalanceUpdated.Broadcast(Amount);  // Broadcasting the updated balance

						//HandleHttpResponse(bSuccess, Amount);

						
					}
					else
					{
						// Handle the invalid response
						UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid response with response %s"), *ResponseContent);

						FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to get a valid response")));

					}
				});
		}

	}
}



void UInvoFunctions::HandleHttpResponse(bool bWasSuccessful, const FString& ResponseContent)
{
	if (bWasSuccessful)
	{
		// Handle success.
		UE_LOG(LogTemp, Warning, TEXT("Success Love: %s"), *ResponseContent);

		//TSharedPtr<FJsonObject> OutDataObject;
		//TArray<TSharedPtr<FJsonValue>> OutDataArray;
		//FString OutMessage;
		//bool OutResults;
		//UInvoHttpManager::GetInstance()->ParseJSON(ResponseContent, OutDataObject, OutDataArray, OutMessage, OutResults);
		//FString Amount = OutDataObject->GetStringField("current_balance");
		UInvoFunctions::CurrentBalance = ResponseContent;
		//OnBalanceUpdated.Broadcast(bWasSuccessful, CurrentBalance);

	}
	else
	{
		// Handle failure.
		UE_LOG(LogTemp, Error, TEXT("Failure: %s"), *ResponseContent);
	}
}

void UInvoFunctions::CreateBrowserWidget()
{

	if (GEngine && GEngine->GameViewport)
	{
		// Create and configure your web browser widget
		TSharedPtr<SWebBrowser> WebBrowserWidget = SNew(SWebBrowser)
			.InitialURL(TEXT("https://www.createlex.com"))
			.ShowControls(true)
			.SupportsTransparency(true);

		// Add the widget to the game's viewport
		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SBox)
			.WidthOverride(800)  // Set the desired width
			.HeightOverride(600) // Set the desired height
			[
				WebBrowserWidget.ToSharedRef()
			]
		);
	}

}



