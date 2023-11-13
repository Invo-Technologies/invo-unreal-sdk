// Fill out your copyright notice in the Description page of Project Settings.

#include "InvoHttpManager.h"
#include "InvoFunctions.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"
#include "Runtime/Core/Public/Misc/MessageDialog.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"

#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"

#include "Serialization/JsonSerializer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/WorldSettings.h"

//#include "ThirdParty/OpenSSL/include/openssl/aes.h"
//#include "ThirdParty/OpenSSL/include/openssl/rand.h"


UInvoHttpManager* UInvoHttpManager::Instance = nullptr;

//Initialize the static delegate
//FHttpResponseReceived UInvoFunctions::OnHttpResponseReceived;


//UInvoHttpManager::FOnHttpResponseReceived UInvoHttpManager::OnHttpResponseReceived2;
//UInvoFunctions::FOnBalanceUpdated UInvoFunctions::OnBalanceUpdated;

UInvoHttpManager* UInvoHttpManager::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<UInvoHttpManager>();
        Instance->AddToRoot();
        //UInvoHttpManager::OnHttpResponseReceived2.AddStatic(&UInvoFunctions::HandleHttpResponse);
        //UInvoFunctions::OnBalanceUpdated.AddStatic(&UInvoFunctions::HandleHttpResponse);

       
    }
    return Instance;
}

void UInvoHttpManager::MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const TMap<FString, FString>& FormData, HttpRequestCallback Callback)
{
    // Create HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    if (AuthCode.IsEmpty())
    {
        // Trigger Google authentication process again
        // Maybe show a message to the user or log it
        //UE_LOG(LogTemp, Warning, TEXT("Auth code not available. Please authenticate again."));
        //return;
    }

    // Set HTTP method (GET, POST, PUT, etc.)
    Request->SetVerb(HttpMethod);

    // Set the request URL
    Request->SetURL(URL);

    if (Headers.IsEmpty())
    {
        Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
        Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
        //Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

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
    Request->OnProcessRequestComplete().BindUObject(this, &UInvoHttpManager::HttpRequestCompleted);

    // Bind the completion to a new method that will use the callback
    Request->OnProcessRequestComplete().BindLambda([this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (Callback)
            {
                Callback(bWasSuccessful, Response->GetContentAsString());
                HttpRequestCompleted(Request, Response, bWasSuccessful);
            }

            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No Call back "));
            }
        });

    // Execute the request
    Request->ProcessRequest();
}

//void UInvoHttpManager ::MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const TMap<FString, FString>& FormData, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
//{
//    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
//
//
//    // Set HTTP method (GET, POST, PUT, etc.)
//    Request->SetVerb(HttpMethod);
//
//    // Set the request URL
//    Request->SetURL(URL);
//
//    if (Headers.IsEmpty())
//    {
//        Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
//        Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
//
//    }
//    else
//
//    {
//        for (const auto& Header : Headers)
//        {
//            Request->SetHeader(Header.Key, Header.Value);
//        }
//    }
//    // Set headers, if any
//
//    FString SecretsIniFilePath = FPaths::ProjectConfigDir() + TEXT("Secrets.ini");
//    FString SecretsNormalizeConfigIniPath = FConfigCacheIni::NormalizeConfigIniPath(SecretsIniFilePath);
//
//    FPaths::NormalizeFilename(SecretsNormalizeConfigIniPath);
//    FString AuthCodeKey;
//    if (GConfig->GetString(TEXT("/Script/Invo.UInvoFunctions"), TEXT("AUTHCODEKEY"), AuthCodeKey, SecretsNormalizeConfigIniPath))
//    {
//        FString HexKeyString = TEXT("1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF"); // 64 hex characters
//
//        FString DecryptDataAuthCode = UInvoFunctions::DecryptData(AuthCodeKey, HexKeyString);
//        UE_LOG(LogTemp, Warning, TEXT("Decrypted AuthCode Key: %s"), *DecryptDataAuthCode);
//        if (!DecryptDataAuthCode.IsEmpty())
//        {
//
//            Request->SetHeader(TEXT("auth_code"), DecryptDataAuthCode);
//
//        }
//        else
//        {
//            UE_LOG(LogTemp, Error, TEXT("AuthCode Key is empty: %s"), *AuthCodeKey);
//            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Need to Initiliaze Invo SDK first.")));
//
//            UInvoFunctions::OpenInvoInitWebPage();
//            return;
//
//        }
//    }
//    else
//    {
//        UE_LOG(LogTemp, Error, TEXT("Failed to get authcode key from config file"));
//        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Need to Initiliaze Invo SDK first.")));
//
//        UInvoFunctions::OpenInvoInitWebPage();
//        return;
//    }
//
//    // Format the payload as x-www-form-urlencoded
//    FString Payload;
//    for (const auto& Pair : FormData)
//    {
//        if (!Payload.IsEmpty())
//        {
//            Payload.Append(TEXT("&"));
//        }
//        Payload.Append(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value));
//    }
//
//    Request->SetContentAsString(Payload);
//
//    // Bind the request's completion delegate
//    //Request->OnProcessRequestComplete().BindUObject(this, &UInvoHttpManager::HttpRequestCompleted);
//    Request->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
//        {
//            if (bWasSuccessful && Response.IsValid())
//
//                // Check the response code
//                if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
//                {
//                    // Deserialize the response into a JSON object
//                    FString ResponseString = Response->GetContentAsString();
//                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
//                    TSharedPtr<FJsonValue> JsonValue;
//                    if (FJsonSerializer::Deserialize(Reader, JsonValue))
//                    {
//                        if (JsonValue->Type == EJson::Object)
//                        {
//                            // Handle JSON object
//                            TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
//                            Callback(JsonObject);
//                        }
//                        else if (JsonValue->Type == EJson::Array)
//                        {
//                            // Handle JSON array
//                            TArray<TSharedPtr<FJsonValue>> JsonArray = JsonValue->AsArray();
//                            for (const TSharedPtr<FJsonValue>& Item : JsonArray)
//                            {
//                                if (Item->Type == EJson::Object)
//                                {
//                                    TSharedPtr<FJsonObject> JsonObject = Item->AsObject();
//                                    Callback(JsonObject);
//                                }
//                            }
//                        }
//                    }
//                    else
//                    {
//                        // JSON parsing failed
//                        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
//                    }
//                }
//                else
//                {
//                    // Unsuccessful API call
//                    UE_LOG(LogTemp, Error, TEXT("API call failed with status code: %d"), Response->GetResponseCode());
//                }
//
//            else
//
//                // Network-related errors or other issues
//                UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
//
//        });
//
//    // Execute the request
//    Request->ProcessRequest();
//
//}

void UInvoHttpManager::HttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    // If the request was not successful, fire the delegate with failure
    if (!bWasSuccessful)
    {
        OnHttpResponseReceived.Broadcast(false, TEXT("Request failed"));
        //InvoOnHttpRequestCompletedEvent.Broadcast(bWasSuccessful, Response.IsValid() ? Response->GetContentAsString() : TEXT("Request Failed"));

        return;
    }

    // If the request was successful but the response is invalid, fire the delegate with failure
    if (!Response.IsValid())
    {
        OnHttpResponseReceived.Broadcast(false, TEXT("Invalid response"));
        //InvoOnHttpRequestCompletedEvent.Broadcast(bWasSuccessful, Response.IsValid() ? Response->GetContentAsString() : TEXT("Invalid Response"));

        return;
    }

    // Check for HTTP error responses
    if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        // The request was successful and the response is OK
       // OnHttpResponseReceived.Broadcast(true, Response->GetContentAsString());
        

        //OnHttpResponseReceived2.Broadcast(bWasSuccessful, Response.IsValid() ? Response->GetContentAsString() : TEXT("Invalid Response"));

        //UInvoFunctions::OnHttpResponseReceived3.Broadcast(bWasSuccessful, Response.IsValid() ? Response->GetContentAsString() : TEXT("Invalid Response"));

        //InvoOnHttpRequestCompletedEvent.Broadcast(bWasSuccessful, Response.IsValid() ? Response->GetContentAsString() : TEXT("Success Response"));
      
        OnHttpRequestCompleted.Broadcast(bWasSuccessful, Response.IsValid() ? Response->GetContentAsString() : TEXT("Invalid Response"));
        

        return;  // Exit the function as the successful response has been handled
    }
    else
    {
        // The request was successful, but the server responded with an error
        FString ErrorMessage = FString::Printf(TEXT("HTTP Error: %d"), Response->GetResponseCode());
        OnHttpResponseReceived.Broadcast(false, ErrorMessage);
        //InvoOnHttpRequestCompletedEvent.Broadcast(bWasSuccessful, Response.IsValid() ? ErrorMessage : TEXT("Invalid Response"));

        return;  // Exit the function as the unsuccessful response has been handled
    }
}

/**/

void UInvoHttpManager::SetAuthCode(FString AuthCodeString)
{
    AuthCode = AuthCodeString;
}

FString UInvoHttpManager::GetAuthCode() const
{
    if (!AuthCode.IsEmpty())
        return AuthCode;
    return "None String";
}



void UInvoHttpManager::YourAESFunction()
{
  
    //AES_set_encrypt_key(Key, 256, &encryptKey);

    // Your encryption and decryption code here...
}

void UInvoHttpManager::ParseJSON(const FString& JSONString, TSharedPtr<FJsonObject>& OutDataObject, TArray<TSharedPtr<FJsonValue>>& OutDataArray, FString& OutMessage, bool& OutResults)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FString Message = JsonObject->GetStringField("message");
        bool Result = JsonObject->GetBoolField("result");
        OutMessage = Message;
        OutResults = Result;

        if (JsonObject->HasTypedField<EJson::Object>("data"))
        {

            TSharedPtr<FJsonObject> DataObject = JsonObject->GetObjectField("data");
            if (DataObject.IsValid())
            {
                UE_LOG(LogTemp, Warning, TEXT("Data json is detected %s"), *JSONString);
                OutDataObject = DataObject;
            }
        }
        else if (JsonObject->HasTypedField<EJson::Array>("data"))
        {

            TArray<TSharedPtr<FJsonValue>> DataObjectArray = JsonObject->GetArrayField("data");
            if (!DataObjectArray.IsEmpty())
            {
             

                OutDataArray = DataObjectArray;
               
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Data is neither an object nor an array"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Couldn't deserialize the JSON"));
    }
}


void UInvoHttpManager::CreatePlayerID(const FString& UniquePlayerID)
{
    // Player Registration 

    UWorld* World = GWorld->GetWorld();
    const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();


    if (World)
    {


        UE_LOG(LogTemp, Warning, TEXT("Testing player ID %s"), *UniquePlayerID);

        if (!Settings->Player_ID.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Player %s is already registered"), *Settings->Player_ID);

        }

        if (!UInvoFunctions::GetSecretsIniKeyValue("PlayerID").IsEmpty())
        {
            //UE_LOG(LogTemp, Warning, TEXT("Player %s is already registered"), *UInvoFunctions::GetSecretsIniKeyValue("PlayerID"));
        }

        // Settings from Invo SDK Feilds

        TMap<FString, FString> FormData;


        if (Settings->Game_ID.IsEmpty())
        {
            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Enter Your GameID in the Invo Plugins Game ID Feild")));
            return;
        }


        if (UInvoFunctions::GetSecretsIniKeyValue("GameID").IsEmpty())
        {
            //FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Enter Your GameID in the Invo Plugins Game ID Feild")));
            //return;
        }

        FormData.Add(TEXT("game_id"), Settings->Game_ID);
        FormData.Add(TEXT("player_name"), UniquePlayerID);


        // 3. Directly make the HTTP request without using UInvoFunctions.
        FString Endpoint = "https://api.dev.ourinvo.com/v1/external/player/register"; // Replace with your actual server address
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

                if (ValidateHttpManagerResponseContent(ResponseContent))
                {
                	// Handle the valid response
                	// Log the response's content as a string.
                	FString StringbSuccess = bSuccess ? "True" : "False";
                	UE_LOG(LogTemp, Warning, TEXT("HTTP Response: %s and is bSucess %s"), *ResponseContent, *StringbSuccess);
                
                    TSharedPtr<FJsonObject> OutDataObject;
                    TArray<TSharedPtr<FJsonValue>> OutDataArray;
                    FString OutMessage;
                    bool OutResults;
                    ParseJSON(ResponseContent, OutDataObject,OutDataArray, OutMessage, OutResults);
                    int32 PlayerId = OutDataObject->GetIntegerField("player_id");
                    int32 GameId = OutDataObject->GetIntegerField("game_id");
                    FString PlayerName = OutDataObject->GetStringField("player_name");
                    FString PlayerIDString = FString::FromInt(PlayerId);
                    if (!UInvoFunctions::CheckSecretsIni("PlayerID"))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("PlayerId %s is created "), *PlayerIDString);
                        UInvoFunctions::UpdateSecretsIni("PlayerID", PlayerIDString);
                        UInvoFunctions::UpdateSecretsIni("SKeyCode", PlayerName); // Temp Akissi Jesus switch to Skey
                        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Set A One Time 6 Digit Pin")));
                        UInvoFunctions::InvoShowSKeyInputDialog();



                    }
                    else if (!UInvoFunctions::CheckSecretsIni("SKeyCode"))
                    {
                        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please Set A One Time 6 Digit Pin")));
                        UInvoFunctions::InvoShowSKeyInputDialog();

                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("PlayerId %s is already creaed."), *PlayerIDString);
                    }
                   
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

}


bool UInvoHttpManager::ValidateHttpManagerResponseContent(const FString& ResponseContent)
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

void UInvoHttpManager::InvoFetchCurrencyBalanceBP()
{
    FString CurrencyBalance;
    //FString Game_ID = UInvoFunctions::GetSecretsIniKeyValue("GameID");
    const UInvoFunctions* SettingsBalance = GetDefault<UInvoFunctions>();
    if (SettingsBalance->Game_ID.IsEmpty())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Yellow, TEXT("Need to Set GameID in Invo Plugin Settings "));
        return;

    }
    else
    {
        if (UInvoFunctions::CheckSecretsIni("PlayerID"))
        {

            //const UInvoFunctions* Settings = GetDefault<UInvoFunctions>();
            TMap<FString, FString> FormData;

            FString PlayerID = UInvoFunctions::GetSecretsIniKeyValue("PlayerID");
            FormData.Add(TEXT("player_id"), PlayerID);
            FormData.Add(TEXT("game_id"), SettingsBalance->Game_ID);


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
                        UInvoHttpManager::GetInstance()->ParseJSON(ResponseContent, OutDataObject, OutDataArray, OutMessage, OutResults);
                        FString Amount = OutDataObject->GetStringField("current_balance");
                        UInvoHttpManager::GetInstance()->OnHttpResponseReceived.Broadcast(true, Amount);
                        //OutBalance = Amount;
                        UE_LOG(LogTemp, Warning, TEXT("Currenace balance %s"), *Amount);
                        

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

