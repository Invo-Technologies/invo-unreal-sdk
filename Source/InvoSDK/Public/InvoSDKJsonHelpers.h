#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"
#include "InvoSDKApiManager.h"
#include "InvoSDKJsonHelpers.generated.h"

UCLASS()
class INVOSDK_API UInvoSDKJsonHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
    static FString GetStringField(const FJsonObjectWrapper& JsonWrapper, const FString& FieldName)
    {
        if (JsonWrapper.JsonObject.IsValid() && JsonWrapper.JsonObject->HasField(FieldName))
        {
            return JsonWrapper.JsonObject->GetStringField(FieldName);
        }
        return FString();
    }

    UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
    static int32 GetIntegerField(const FJsonObjectWrapper& JsonWrapper, const FString& FieldName)
    {
        if (JsonWrapper.JsonObject.IsValid() && JsonWrapper.JsonObject->HasField(FieldName))
        {
            return JsonWrapper.JsonObject->GetIntegerField(FieldName);
        }
        return 0;
    }

    UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
    static bool GetBoolField(const FJsonObjectWrapper& JsonWrapper, const FString& FieldName)
    {
        if (JsonWrapper.JsonObject.IsValid() && JsonWrapper.JsonObject->HasField(FieldName))
        {
            return JsonWrapper.JsonObject->GetBoolField(FieldName);
        }
        return false;
    }

    UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
    static TArray<FJsonObjectWrapper> GetObjectsArray(const FJsonObjectWrapper& JsonWrapper, const FString& FieldName)
    {
        TArray<FJsonObjectWrapper> ResultArray;

        if (JsonWrapper.JsonObject.IsValid() && JsonWrapper.JsonObject->HasField(FieldName))
        {
            const TArray<TSharedPtr<FJsonValue>>* JsonArray = nullptr;
            if (JsonWrapper.JsonObject->TryGetArrayField(FieldName, JsonArray))
            {
                for (const TSharedPtr<FJsonValue>& JsonValue : *JsonArray)
                {
                    if (JsonValue->Type == EJson::Object)
                    {
                        // Wrap the object into an FJsonObjectWrapper and add to the result array
                        FJsonObjectWrapper JsonObjectWrapper;
                        JsonObjectWrapper.JsonObject = JsonValue->AsObject();
                        ResultArray.Add(JsonObjectWrapper);
                    }
                }
            }
        }

        return ResultArray;
    }

};
