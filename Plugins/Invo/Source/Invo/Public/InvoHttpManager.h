// InvoHttpManager.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "InvoFunctions.h"
#include "InvoHttpManager.generated.h"

// Define a delegate with the necessary parameters for the callback.
//DECLARE_DYNAMIC_DELEGATE_TwoParams(FHttpRequestCompletedDelegate, bool, bWasSuccessful, const FString&, ResponseContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHttpRequestCompletedDelegate, bool, bWasSuccessful, const FString&, ResponseContent);
//DECLARE_DELEGATE_TwoParams(FHttpRequestCompletedDelegate, bool, const FString&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHttpResponseReceived, bool, bSuccess, const FString&, ResponseString);

UCLASS()
class INVO_API UInvoHttpManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Invo", meta = (DisplayName = "Get Invo Manager"))
    static UInvoHttpManager* GetInstance();


    typedef TFunction<void(bool, const FString&)> HttpRequestCallback;


    void MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers, const TMap<FString, FString>& FormData, HttpRequestCallback Callback);

    // This delegate is what we will bind to in the widget.
    UPROPERTY(BlueprintAssignable, Category = "Invo")
    FHttpRequestCompletedDelegate OnHttpRequestCompleted;


    // Declare the delegate as a static member
    UPROPERTY(BlueprintAssignable, Category = "Invo")
    FHttpResponseReceived OnHttpResponseReceived;

    void SetAuthCode(FString AuthCodeString);

    FString GetAuthCode() const;

    void CreatePlayerID(const FString& UniquePlayerID);

    void ParseJSON(const FString& JSONString, TSharedPtr<FJsonObject>& OutDataObject, TArray<TSharedPtr<FJsonValue>>& OutDataArray, FString& OutMessage, bool& OutResults);
   
    UFUNCTION(BlueprintCallable, Category = "Invo")
    static void InvoFetchCurrencyBalanceBP();

    bool ValidateHttpManagerResponseContent(const FString& ResponseContent);


private:
    static UInvoHttpManager* Instance;

    void HttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    FString AuthCode;

    void YourAESFunction();

  
    //FHttpResponseReceived ResponseContentDelegate;
};



