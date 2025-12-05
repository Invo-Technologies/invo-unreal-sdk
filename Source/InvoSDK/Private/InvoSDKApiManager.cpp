#include "InvoSDKApiManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/ModuleManager.h"

UInvoSDKApiManager* UInvoSDKApiManager::InvoRequest(
    UObject* /*WorldContextObject*/,
    const FString& BaseUrl,
    const FString& Endpoint,
    const TMap<FString, FString>& Headers,
    const FString& Body,
    EInvoHttpMethod Method)
{
    UInvoSDKApiManager* Node = NewObject<UInvoSDKApiManager>();
    Node->RequestBaseUrl = BaseUrl;
    Node->RequestEndpoint = Endpoint;
    Node->RequestHeaders = Headers;
    Node->RequestBody = Body;
    Node->RequestMethod = Method;
    return Node;
}

void UInvoSDKApiManager::Activate()
{
    if (!FModuleManager::Get().IsModuleLoaded("HTTP"))
    {
        FModuleManager::Get().LoadModule("HTTP");
    }

    FHttpModule& HttpModule = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

    const FString FullUrl = RequestBaseUrl + RequestEndpoint;
    Request->SetURL(FullUrl);

    switch (RequestMethod)
    {
    case EInvoHttpMethod::GET:    Request->SetVerb(TEXT("GET")); break;
    case EInvoHttpMethod::POST:   Request->SetVerb(TEXT("POST")); break;
    case EInvoHttpMethod::PUT:    Request->SetVerb(TEXT("PUT")); break;
    case EInvoHttpMethod::DELETE: Request->SetVerb(TEXT("DELETE")); break;
    default: Request->SetVerb(TEXT("GET")); break;
    }

    bool bHasContentType = false;
    for (const auto& Pair : RequestHeaders)
    {
        if (Pair.Key.Equals(TEXT("Content-Type"), ESearchCase::IgnoreCase))
        {
            bHasContentType = true;
            break;
        }
    }
    if (!bHasContentType)
    {
        Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    }

    for (const auto& Pair : RequestHeaders)
    {
        Request->SetHeader(Pair.Key, Pair.Value);
    }

    if (RequestMethod == EInvoHttpMethod::POST || RequestMethod == EInvoHttpMethod::PUT)
    {
        Request->SetContentAsString(RequestBody);
    }

    Request->OnProcessRequestComplete().BindUObject(this, &UInvoSDKApiManager::HandleResponse);

    if (!Request->ProcessRequest())
    {
        FJsonObjectWrapper EmptyJson;
        OnFail.Broadcast(TEXT("Failed to start HTTP request"), -1, false, EmptyJson);
        SetReadyToDestroy();
    }
}

void UInvoSDKApiManager::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString BodyString;
    int32 StatusCode = -1;
    FJsonObjectWrapper JsonWrapper;

    if (Response.IsValid())
    {
        StatusCode = Response->GetResponseCode();
        BodyString = Response->GetContentAsString();

        // Parse JSON into JsonWrapper
        TSharedPtr<FJsonObject> ParsedJson;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyString);
        if (FJsonSerializer::Deserialize(Reader, ParsedJson) && ParsedJson.IsValid())
        {
            JsonWrapper.JsonObject = ParsedJson;
        }
    }

    const bool bHttpOk = (StatusCode >= 200 && StatusCode < 300);
    if (bWasSuccessful && Response.IsValid() && bHttpOk)
    {
        UE_LOG(LogTemp, Log, TEXT("InvoSDK Request Success [%d]: %s"), StatusCode, *BodyString);
        OnSuccess.Broadcast(BodyString, StatusCode, true, JsonWrapper);   // ✅ 4 args
    }
    else
    {
        const FString ErrorText = Response.IsValid() ? BodyString : TEXT("HTTP request failed (no response)");
        UE_LOG(LogTemp, Error, TEXT("InvoSDK Request Failed [%d]: %s"), StatusCode, *ErrorText);
        OnFail.Broadcast(ErrorText, StatusCode, false, JsonWrapper);      // ✅ 4 args
    }

    SetReadyToDestroy();
}


