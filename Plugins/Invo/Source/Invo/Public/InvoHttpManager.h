// InvoHttpManager.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "InvoHttpManager.generated.h"


UCLASS()
class INVO_API UInvoHttpManager : public UObject
{
    GENERATED_BODY()

public:
    static UInvoHttpManager* GetInstance();


    void MakeHttpRequest(const FString& URL, const FString& HttpMethod, const TMap<FString, FString>& Headers);


private:
    static UInvoHttpManager* Instance;

    void HttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};



