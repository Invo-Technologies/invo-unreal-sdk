// Alex Kissi Jr for OurInvo  CopyRight 2023 SDK Unreal Engine Uplugin.
#include "InvoFunctions.h"
//#include "WebBrowserWidget.h"

#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetConnection.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AES.h"
#include "Misc/Base64.h"

#include "InvoHttpManager.h"
#include <regex>


// UI Widgets 
#include "SInvoGDPRWidget.h"
#include "SInvoTicketWidget.h"
#include "SInvoTransferWidget.h"
#include "SInvoPurchaseWidget.h"
#include "SInvoTradeWidget.h"

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
#include "WebBrowser/Public/SWebBrowser.h"
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


// Initialize the static shared references
TSharedRef<SWebBrowser> UInvoFunctions::WebBrowser = SNew(SWebBrowser);
TSharedRef<SWindow> UInvoFunctions::Window = SNew(SWindow);

//Ticket Support
TSharedPtr<SInvoTicketWidget> UInvoFunctions::InvoTicketWidget;
TSharedPtr<SInvoTransferWidget> UInvoFunctions::InvoTransferWidget;


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

bool UInvoFunctions::GetMaxPacket(const UObject* WorldContextObject, int32& OutMaxPacket)
{
	if (GET_CONNECTION)
	{
		OutMaxPacket = PlayerNetConnection->MaxPacket;
		return true;
	}

	OutMaxPacket = INDEX_NONE;
	return false;
}


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

}

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
	UE_LOG(LogTemp, Warning, TEXT("Testing Game_ID fisrt %i"), Settings->Game_ID)


		if (Settings)
		{
			FInvoAssetData AssetData = Settings->AssetData;
			//UE_LOG(LogTemp, Warning, TEXT("Testing Game_ID %s"), *Settings->Game_ID)

			return AssetData;
			// Access the properties of AssetData...
		}

	return Settings->AssetData;

}

void UInvoFunctions::GetInvoFacts()
{
	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

	FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
	FPaths::NormalizeFilename(SecretsIniFilePath);

	FString SecretKey;
	if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), TEXT("SECRETKEY"), SecretKey, SecretsIniFilePath))
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

	Request->ProcessRequest();

}


void UInvoFunctions::OpenWebView(const FString& Url)
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

				FString AuthCode = ExtractCodeFromUrl(NewUrl);
				UInvoHttpManager::GetInstance()->SetAuthCode(AuthCode);
				UE_LOG(LogTemp, Warning, TEXT("AuthCode is %s"), *AuthCode);
				HandleURLChange(NewUrl);
				
				
				});


			if (!Window->IsActive())
			{
				Window->SetContent(UInvoFunctions::WebBrowser);
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

		Window->SetContent(UInvoFunctions::WebBrowser);
		FSlateApplication::Get().AddWindow(Window);

		if (Url.IsEmpty())
			WebBrowser->LoadURL(TEXT("http://localhost:5173/"));
		else
			WebBrowser->LoadURL(Url);

	}
}

void UInvoFunctions::HandleURLChange(const FString& NewUrl)
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
					HandleJavaScriptCallback("closeButton", WebBrowser);
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
					HandleJavaScriptTestCallback("PageA", WebBrowser);
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
					HandleJavaScriptTestCallback("PageB", WebBrowser);
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
	OpenWebView(ReactWepPageUrl);
}

void UInvoFunctions::OpenInvoInitWebPage(UObject* WorldContextObject, FString Url = "")
{
	//FString URL = TEXT("https://www.ourinvo.com");
	//FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	// React Local Sample Page Ui http://localhost:5173/
	// Get the Unreal Engine version.
	FString ReactWepPageUrl = "https://api.dev.ourinvo.com/";
	OpenWebView(ReactWepPageUrl);
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


void UInvoFunctions::HandleJavaScriptCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget)
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


void UInvoFunctions::HandleJavaScriptTestCallback(const FString& Message, TSharedPtr<SWebBrowser> WebBrowserWidget)
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
				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
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


void UInvoFunctions::InvoAPIJsonReturnCall(const FString& City, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{
	FJsonObject JsonRespObject;

	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();

	FString Asset_ID = Settings->AssetData.Asset_ID.Replace(TEXT(" "), TEXT("%20"));

	FString Url = FString::Printf(TEXT("https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"), *City.Replace(TEXT(" "), TEXT("%20")), Settings->Game_ID);

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


void UInvoFunctions::TransferCurrencyBP(int64 SourceGameID, int64 SourcePlayerID, int64 TargetGameID, int64 TargetPlayerID, float Amount, FString CurrencyName, FOnInvoAPICallCompleted OnTransferCompleted)
{
	TransferCurrency(SourceGameID, SourcePlayerID, TargetGameID, TargetPlayerID, Amount, CurrencyName, [OnTransferCompleted](const FString& Result)
		{
			bool bSuccess = Result == "Currency transferred successfully";
			OnTransferCompleted.ExecuteIfBound(bSuccess);
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

void UInvoFunctions::GetInvoCurrencyAmountForPlayerBP(int64 GameID, int64 PlayerID, const FOnCurrencyAmountFetchedBP& OnCurrencyAmountFetchedBP)
{
	GetInvoCurrencyAmountForPlayer(GameID, PlayerID, [OnCurrencyAmountFetchedBP](const FString& CurrencyAmount)
		{
			OnCurrencyAmountFetchedBP.ExecuteIfBound(CurrencyAmount);
		});
}


void UInvoFunctions::InvoTransferCurrencyWebViewBP(FOnInvoAPICallCompleted OnTransferCompleted)
{
	FString Url = "http://localhost:5173/transfer";
	OpenWebView(Url);
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
	

	Window = SNew(SWindow)
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
	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoGDPR", "WindowTitle", "Invo GDPR"))
		.ClientSize(FVector2D(700, 200))
		.SupportsMinimize(true)
		.SupportsMaximize(true);

	Window->SetContent(SNew(SInvoGDPRWidget));

	FSlateApplication::Get().AddWindow(Window);
}

void UInvoFunctions::InvoShowTransferWidget()
{
	// Create a InvoTransfer Widget instance
	Window = SNew(SWindow)
		.Title(NSLOCTEXT("InvoTransfer", "WindowTitle", "Invo Transfer System"))
		.ClientSize(FVector2D(500, 500))
		.SupportsMinimize(true)
		.SupportsMaximize(true);

	Window->SetContent(SNew(SInvoTransferWidget));
	FSlateApplication::Get().AddWindow(Window);
}

void UInvoFunctions::InvoShowPurchaseWidget()
{
	// Create a InvoTransfer Widget instance
	Window = SNew(SWindow)
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
	Window = SNew(SWindow)
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
	std::wstring Source(*HtmlSource); // Convert FString to std::wstring
	std::wregex Pattern(L"\"code\":\"([^\"]+)\""); // Regular expression pattern to match "code":"<value>"
	std::wsmatch Matches;

	if (std::regex_search(Source, Matches, Pattern) && Matches.size() > 1)
	{
		// Convert the matched value to FString and return
		return FString(Matches[1].str().c_str());
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
