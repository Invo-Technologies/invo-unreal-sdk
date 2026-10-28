#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"
#include "InvoTypes.generated.h"

/** Sandbox and production are separate INVO environments with separate game ids and keys. */
UENUM(BlueprintType)
enum class EInvoEnvironment : uint8
{
    Sandbox    UMETA(DisplayName = "Sandbox"),
    Production UMETA(DisplayName = "Production")
};

/**
 * The single error type every INVO call reports. The backend envelope is usually
 * `{"error": <message>, "code": <STABLE_CODE>}`, but a few routes carry only `error` or
 * `{"status": "error", "message": ...}`, so Code may be empty: branch on Code when present,
 * else fall back to Message. Classifiers live in UInvoErrorLibrary.
 */
USTRUCT(BlueprintType)
struct INVOSDK_API FInvoError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    FString Message;

    /** Stable machine code from the backend, empty when absent. */
    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    FString Code;

    /** HTTP status. 0 for client-side validation and network errors. */
    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    int32 Status = 0;

    /** Backend request id from the response headers. Quote it in support tickets. */
    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    FString RequestId;

    /** The parsed response body, when it was a JSON object. */
    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    FJsonObjectWrapper Body;

    static FInvoError Make(const FString& InMessage, const FString& InCode = FString(), int32 InStatus = 0);
    static FInvoError InvalidInput(const FString& Label, const FString& Value, const FString& Why);

    /** Builds the error from a non-2xx response (parsed object, or the raw text when not JSON). */
    static FInvoError FromResponse(int32 InStatus, const TSharedPtr<FJsonObject>& Parsed, const FString& RawText, const FString& InRequestId);

    /** When Code is empty and `error` is a bare token (letters, digits, _; at most 64), copy it into Code. */
    FInvoError& PromoteBareErrorToken();

    FString ToString() const;
};

/** What every async INVO call hands back. Data is the response object when bOk is true. */
USTRUCT(BlueprintType)
struct INVOSDK_API FInvoResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    bool bOk = false;

    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    FJsonObjectWrapper Data;

    /** Set when bOk is false. */
    UPROPERTY(BlueprintReadOnly, Category = "InvoSDK|Result")
    FInvoError Error;

    static FInvoResult Success(const TSharedPtr<FJsonObject>& InData);
    static FInvoResult Failure(const FInvoError& InError);

    /** Data's object, never null (an empty object when the response had no body). */
    TSharedRef<FJsonObject> Object() const;
};

/** Blueprint callback for every async call on UInvoSubsystem. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FInvoResultDelegate, const FInvoResult&, Result);

/** Native callback for C++ callers. */
using FInvoResultCallback = TFunction<void(const FInvoResult&)>;
