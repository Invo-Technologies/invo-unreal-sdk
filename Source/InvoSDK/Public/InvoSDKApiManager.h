#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "JsonObjectWrapper.h"
#include "Http.h"
#include "InvoSDKApiManager.generated.h"

UENUM(BlueprintType)
enum class EInvoHttpMethod : uint8
{
    GET     UMETA(DisplayName = "GET"),
    POST    UMETA(DisplayName = "POST"),
    PUT     UMETA(DisplayName = "PUT"),
    DELETE  UMETA(DisplayName = "DELETE")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FInvoSDKOnResponse,
    const FString&, ResponseContent,
    int32, StatusCode,
    bool, bWasSuccessful,
    const FJsonObjectWrapper&, JsonObject   // ✅ New param
);

UCLASS()
class INVOSDK_API UInvoSDKApiManager : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FInvoSDKOnResponse OnSuccess;

    UPROPERTY(BlueprintAssignable)
    FInvoSDKOnResponse OnFail;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "InvoSDK|API")
    static UInvoSDKApiManager* InvoRequest(
        UObject* WorldContextObject,
        const FString& BaseUrl,
        const FString& Endpoint,
        const TMap<FString, FString>& Headers,
        const FString& Body,
        EInvoHttpMethod Method
    );

    virtual void Activate() override;

private:
    FString RequestBaseUrl;
    FString RequestEndpoint;
    TMap<FString, FString> RequestHeaders;
    FString RequestBody;
    EInvoHttpMethod RequestMethod;

    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
