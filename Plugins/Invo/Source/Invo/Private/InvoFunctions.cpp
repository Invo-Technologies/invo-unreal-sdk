// Alex Kissi Jr for OurInvo  CopyRight 2023 SDK Unreal Engine Uplugin.
#include "InvoFunctions.h"
//#include "WebBrowserWidget.h"
#include "InvoWebBrowser.h"

#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetConnection.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/WorldSettings.h"

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


//#include "Engine/Plugins/Runtime/Database/DatabaseSupport/Public/DatabaseSupport.h"

// Postgres
//#include "Engine/Plugins/Runtime/Database/DatabaseSupport/SourceDatabaseSupport/Public/Database.h"

//#include "DatabaseSupport/Public/DatabaseSupport.h"

//#include "Plugins/Runtime/Database/DatabaseSupport/Source/DatabaseSupport/Public/Database.h"

//#include "Plugins/Runtime/Database/DatabaseSupport/Source/DatabaseSupport/Public/DatabaseSupport.h"


// Initialize the static shared references
TSharedRef<SWebBrowser> UInvoFunctions::WebBrowser = SNew(SWebBrowser);
TSharedRef<SWindow> UInvoFunctions::Window = SNew(SWindow);

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
	UE_LOG(LogTemp, Warning, TEXT("Testing Game_ID fisrt %s"), *Settings->Game_ID)


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

	// Add more conditions for other URL changes if needed
	// else if (NewUrl.Contains(TEXT("PageA"))) { ... }
	// else if (NewUrl.Contains(TEXT("PageB"))) { ... }
	// ...
}


void UInvoFunctions::OpenInvoWebPage(UObject* WorldContextObject)
{
	//FString URL = TEXT("https://www.ourinvo.com");
	//FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);

	// Get the Unreal Engine version.
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

				HandleURLChange(NewUrl);

				//if (NewUrl.Contains(TEXT("buttonClicked"))) {
				//	UE_LOG(LogTemp, Warning, TEXT("Button Clicked "));
				//	
				//	// Button on the website was clicked
				//	FString Message = TEXT("Button Clicked!");
				//	float Duration = 5.0f;
				//	FColor Color = FColor::Green;
				//	GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);	
				//	
				//	
				//	// Access the game thread
				//	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//			{
				//				HandleJavaScriptCallback("ButtonClicked", WebBrowser);
				//			}
				//
				//
				//		}, TStatId(), nullptr, ENamedThreads::GameThread);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task_Render = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			// Perform game thread operations here
				//				if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//				{
				//					
				//				}							
				//
				//		}, TStatId(), nullptr, ENamedThreads::ActualRenderingThread);
				//
				//}
				//
				//UE_LOG(LogTemp, Warning, TEXT("Testing %s"), *NewUrl);
				//
				//if (NewUrl.Contains(TEXT("PageA"))) {
				//	UE_LOG(LogTemp, Warning, TEXT("Button Clicked "));
				//
				//	// Button on the website was clicked
				//	FString Message = TEXT("Button Clicked!");
				//	float Duration = 5.0f;
				//	FColor Color = FColor::Green;
				//	GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//			{
				//				HandleJavaScriptCallback("ButtonClicked", WebBrowser);
				//			}
				//
				//
				//		}, TStatId(), nullptr, ENamedThreads::GameThread);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task_Render = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			// Perform game thread operations here
				//			if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//			{
				//
				//			}
				//
				//		}, TStatId(), nullptr, ENamedThreads::ActualRenderingThread);
				//
				//}
				//
				//
				//if (NewUrl.Contains(TEXT("PageB"))) {
				//	UE_LOG(LogTemp, Warning, TEXT("Button Test Clicked "));
				//
				//	// Button on the website was clicked
				//	FString Message = TEXT("Button Test Clicked!");
				//	float Duration = 5.0f;
				//	FColor Color = FColor::Green;
				//	GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//			{
				//				//HandleJavaScriptTestCallback("ButtonTestClicked", WebBrowser);
				//				GetInvoFunctionOne();
				//			}
				//
				//
				//		}, TStatId(), nullptr, ENamedThreads::GameThread);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task_Render = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//	{
				//		// Perform game thread operations here
				//		if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//		{
				//
				//		}
				//
				//	}, TStatId(), nullptr, ENamedThreads::ActualRenderingThread);
				//}
				//
				//
				//
				//
				//if (NewUrl.Contains(TEXT("buttonClicked"))) {
				//	UE_LOG(LogTemp, Warning, TEXT("Button Clicked "));
				//
				//	// Button on the website was clicked
				//	FString Message = TEXT("Button Clicked!");
				//	float Duration = 5.0f;
				//	FColor Color = FColor::Green;
				//	GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//			{
				//				HandleJavaScriptCallback("ButtonClicked", WebBrowser);
				//			}
				//
				//
				//		}, TStatId(), nullptr, ENamedThreads::GameThread);
				//
				//
				//	// Access the game thread
				//	FGraphEventRef Task_Render = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
				//		{
				//			// Perform game thread operations here
				//			if (Window.ToSharedPtr().IsValid() && WebBrowser.ToSharedPtr().IsValid())
				//			{
				//
				//			}
				//
				//		}, TStatId(), nullptr, ENamedThreads::ActualRenderingThread);
				//
				//}
				//
				});
		
			
			if (!Window->IsActive())
			{
				Window->SetContent(UInvoFunctions::WebBrowser);
				FSlateApplication::Get().AddWindow(Window);
				WebBrowser->LoadURL(TEXT("http://localhost:5173/"));
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
		
		FString URL = TEXT("http://localhost:5173/");

		TSharedPtr<SWebBrowser> WebBrowserWidget = SNew(SWebBrowser)
			.InitialURL(URL)
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
		WebBrowser->LoadURL(TEXT("http://localhost:5173/"));
	}
	
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

					TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
					if (FJsonSerializer::Deserialize(Reader, JsonObject))
					{

						// Successful API call and JSON parsing
						Callback(JsonObject);
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

void UInvoFunctions ::InvoAPIJsonReturnCall(const FString& City, FString& JsonData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
{
	FJsonObject JsonRespObject;

	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
	
	FString Asset_ID = Settings->AssetData.Asset_ID.Replace(TEXT(" "), TEXT("%20"));

	FString Url = FString::Printf(TEXT("https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"), *City.Replace(TEXT(" "), TEXT("%20")), *Settings->Game_ID);

	MakeHttpRequest(Url, TEXT("GET"),JsonData,[Callback](TSharedPtr<FJsonObject> JsonObject)
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
	InvoAPIJsonReturnCall(TEXT("San Diego"),JsonData, [OnCompleted](TSharedPtr<FJsonObject> JsonObject)
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

	FString RegisterDataBaseEndpoint = FString::Printf(TEXT("http://127.0.0.1:3030/register"));
	FString HttpMethod = "POST";


	const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
	auto DeveloperRegistrationInfo = Settings->DeveloperRegistrationInfo;
	auto GameNme = DeveloperRegistrationInfo.GameName;

	// Create a JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("name_or_company_name", DeveloperRegistrationInfo.DeveloperName);
	JsonObject->SetStringField("contact_email", DeveloperRegistrationInfo.ContactEmail);
	JsonObject->SetStringField("phone_number", DeveloperRegistrationInfo.PhoneNumber);
	JsonObject->SetStringField("city", DeveloperRegistrationInfo.City);
	JsonObject->SetStringField("state", DeveloperRegistrationInfo.State);
	JsonObject->SetStringField("country", DeveloperRegistrationInfo.Country);
	JsonObject->SetStringField("website_url", DeveloperRegistrationInfo.WebsiteURL);
	JsonObject->SetStringField("tax_identification_number", DeveloperRegistrationInfo.TaxIdentificationNumber);
	JsonObject->SetStringField("username", DeveloperRegistrationInfo.Username);
	JsonObject->SetStringField("password", DeveloperRegistrationInfo.Password);


	// Convert JSON object to string
	FString JsonData;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonData);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	UE_LOG(LogTemp, Warning, TEXT("Testing %s"), *JsonData);

	//Call the existing MakeHttpRequest function with the blockchain URL, HTTP method, and JSON data
	MakeHttpRequest(RegisterDataBaseEndpoint, HttpMethod, JsonData, [OnRegisteredDatabaseReceived](TSharedPtr<FJsonObject> JsonObject)
		{	

			/*
			
			if (JsonObject.IsValid() )
			{
				// Get the result (block number) from the JSON response
				//UE_LOG(LogTemp, Warning, TEXT("Works %s"), *JsonData);
				FString BlockNumber = JsonObject->GetStringField("message");
				FString Message = FString::Printf(TEXT("Works %s "), *BlockNumber);
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
				OnRegisteredDatabaseReceived(BlockNumber);
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Some Error %s"), *JsonData);
				FString Message = FString::Printf(TEXT("Problem register "));
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, Message);

			}

			*/

			if (JsonObject.IsValid() && JsonObject->HasField("name_or_company_name"))
			{
				FString Message = FString::Printf(TEXT("Registered successfully: %s"), *JsonObject->GetStringField("name_or_company_name"));
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Message);
				OnRegisteredDatabaseReceived(JsonObject->GetStringField("name_or_company_name"));
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
