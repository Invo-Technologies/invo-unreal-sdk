// Fill out your copyright notice in the Description page of Project Settings.

#include "InvoHttpManager.h"
#include "InvoFunctions.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"

UInvoHttpManager* UInvoHttpManager::Instance = nullptr;

//Initialize the static delegate
//FHttpResponseReceived UInvoFunctions::OnHttpResponseReceived;


UInvoHttpManager* UInvoHttpManager::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<UInvoHttpManager>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UInvoHttpManager::MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const FString& Payload, HttpRequestCallback Callback)
{
    // Create HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // Set HTTP method (GET, POST, PUT, etc.)
    Request->SetVerb(HttpMethod);

    // Set the request URL
    Request->SetURL(URL);

    if (Headers.IsEmpty())
    {
        Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
        Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    }
    else

    {
        for (const auto& Header : Headers)
        {
            Request->SetHeader(Header.Key, Header.Value);
        }
    }
    // Set headers, if any
 

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
        OnHttpResponseReceived.Broadcast(true, Response->GetContentAsString());
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

