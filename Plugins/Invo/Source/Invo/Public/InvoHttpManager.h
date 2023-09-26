// InvoHttpManager.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "InvoHttpManager.generated.h"

// Define a delegate with the necessary parameters for the callback.
//DECLARE_DYNAMIC_DELEGATE_TwoParams(FHttpRequestCompletedDelegate, bool, bWasSuccessful, const FString&, ResponseContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHttpRequestCompletedDelegate, bool, bWasSuccessful, const FString&, ResponseContent);
//DECLARE_DELEGATE_TwoParams(FHttpRequestCompletedDelegate, bool, const FString&);

UCLASS()
class INVO_API UInvoHttpManager : public UObject
{
    GENERATED_BODY()

public:
    static UInvoHttpManager* GetInstance();


    typedef TFunction<void(bool, const FString&)> HttpRequestCallback;


    void MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const FString& Payload, HttpRequestCallback Callback);

    // This delegate is what we will bind to in the widget.
    //UPROPERTY(BlueprintAssignable, Category = "Invo")
    FHttpRequestCompletedDelegate OnHttpRequestCompleted;





private:
    static UInvoHttpManager* Instance;

    void HttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    //FHttpResponseReceived ResponseContentDelegate;
};



