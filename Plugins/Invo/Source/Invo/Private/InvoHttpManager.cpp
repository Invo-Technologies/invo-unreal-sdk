// Fill out your copyright notice in the Description page of Project Settings.

#include "InvoHttpManager.h"
#include "InvoFunctions.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"

UInvoHttpManager* UInvoHttpManager::Instance = nullptr;

//Initialize the static delegate
FHttpResponseReceived UInvoFunctions::OnHttpResponseReceived;


UInvoHttpManager* UInvoHttpManager::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<UInvoHttpManager>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UInvoHttpManager::MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers)
{
    // Create HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // Set HTTP method (GET, POST, PUT, etc.)
    Request->SetVerb(HttpMethod);

    // Set the request URL
    Request->SetURL(URL);

    // Set headers, if any
    for (const auto& Header : Headers)
    {
        Request->SetHeader(Header.Key, Header.Value);
    }

    // Bind the request's completion delegate
    Request->OnProcessRequestComplete().BindUObject(this, &UInvoHttpManager::HttpRequestCompleted);

    // Execute the request
    Request->ProcessRequest();
}

void UInvoHttpManager::HttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    // If the request was not successful, fire the delegate with failure
    if (!bWasSuccessful)
    {
        UInvoFunctions::OnHttpResponseReceived.ExecuteIfBound(false, TEXT("Request failed"));
        return;
    }

    // If the request was successful but the response is invalid, fire the delegate with failure
    if (!Response.IsValid())
    {
        UInvoFunctions::OnHttpResponseReceived.ExecuteIfBound(false, TEXT("Invalid response"));
        return;
    }

    // Check for HTTP error responses
    if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        // The request was successful and the response is OK
        UInvoFunctions::OnHttpResponseReceived.ExecuteIfBound(true, Response->GetContentAsString());
        return;  // Exit the function as the successful response has been handled
    }
    else
    {
        // The request was successful, but the server responded with an error
        FString ErrorMessage = FString::Printf(TEXT("HTTP Error: %d"), Response->GetResponseCode());
        UInvoFunctions::OnHttpResponseReceived.ExecuteIfBound(false, ErrorMessage);
        return;  // Exit the function as the unsuccessful response has been handled
    }
}
