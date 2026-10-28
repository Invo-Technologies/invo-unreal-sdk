#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvoTypes.h"
#include "InvoLibrary.generated.h"

/**
 * Pure helpers: client-side validation (ported from the Python/JS SDKs so a coerced-but-unintended
 * value never reaches the wire), URL and key checks, and the error classifiers. No state, so the
 * automation tests in Private/Tests/InvoCoreTests.cpp cover all of it.
 */
UCLASS()
class INVOSDK_API UInvoLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Max single currency purchase in USD (the backend rejects more). */
    static constexpr double MaxUsdAmount = 999.99;
    /** Max item price in game-currency units. */
    static constexpr double MaxItemPrice = 999999.99;

    // ---------------- validation ----------------

    /** True for a plain decimal string ("5", "4.99"), greater than 0, at most MaxValue, at most 2 places.
     *  Hex, scientific notation, signs and whitespace are refused. */
    static bool IsValidMoney(const FString& Value, double MaxValue);

    /** E.164: a leading +, a country code and up to 15 digits, e.g. "+15555550100". */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Validation")
    static bool IsE164Phone(const FString& Value);

    /** Formats a money value the backend accepts: up to 2 places, no trailing zeros ("4.99", "5"). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Validation")
    static FString FormatMoney(double Value);

    /** A collision-safe client_request_id / idempotency key (128 random bits from the OS).
     *  Never derive keys from time or frame counters: two sessions could deduplicate each other's sends. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Validation")
    static FString NewClientRequestId();

    // ---------------- urls and keys ----------------

    /** Empty when Url is acceptable as an API base, else the reason. https only; http is allowed
     *  for localhost, 127.0.0.1 and [::1] (local development). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Security")
    static FString SecureBaseUrlProblem(const FString& Url);

    /** Strips CR/LF and other control characters so a hostile value cannot smuggle headers. */
    static FString SanitizeHeaderValue(const FString& Value);

    /** The log-safe form of a URL: no query string, and the by-email path segment redacted. */
    static FString RedactUrl(const FString& Url);

    /** "Sandbox" for ivsdk_test_ keys, "Production" for ivsdk_live_, empty when neither. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Security")
    static FString KeyEnvironment(const FString& Key);

    /** Seconds from a 429's `retry_after` body field or Retry-After header, -1 when absent. */
    static double RetryAfterSeconds(const TSharedPtr<FJsonObject>& Body, const FString& RetryAfterHeader);

    // ---------------- error classifiers ----------------

    /** The session token expired: re-mint and retry (the subsystem does this once for you). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsTokenExpired(const FInvoError& Error) { return Error.Code == TEXT("SDK_TOKEN_EXPIRED"); }

    /** A sandbox key was sent to production, or a production key to the sandbox. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsKeyForOtherEnvironment(const FInvoError& Error) { return Error.Code == TEXT("KEY_FOR_OTHER_ENVIRONMENT"); }

    /** The recipient has no phone approval set up: fall back to a claim code. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsReceiverNotEnrolled(const FInvoError& Error);

    /** An item purchase failed on balance (400 only; a 429 with the same words is a rate limit). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsInsufficientBalance(const FInvoError& Error);

    /** 409: the call needs the phone owner's approval first. Not a failure: show Message and re-issue once approved. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsPhoneShareApprovalRequired(const FInvoError& Error) { return Error.Code == TEXT("PHONE_SHARE_APPROVAL_REQUIRED"); }

    /** 409 on a phone change: the player secured their phone, so use a phone change request instead. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsPhoneChangeRequiresVerification(const FInvoError& Error) { return Error.Code == TEXT("PHONE_CHANGE_REQUIRES_VERIFICATION"); }

    /** 403: money-out is paused for 24 hours after a passkey reset. Do not retry-loop. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsPasskeyRecoveryCooldown(const FInvoError& Error) { return Error.Code == TEXT("PASSKEY_RECOVERY_COOLDOWN"); }

    /** 409: moving more than the non-Steam balance to a non-Steam destination. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsSteamValueNonTransferable(const FInvoError& Error) { return Error.Code == TEXT("STEAM_VALUE_NON_TRANSFERABLE"); }

    /** 409: moving non-Steam value into a Steam title. Pick another destination. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsNonSteamValueIntoSteamBlocked(const FInvoError& Error) { return Error.Code == TEXT("NON_STEAM_VALUE_INTO_STEAM_BLOCKED"); }

    /** An idempotency-keyed duplicate (409). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsDuplicateRequest(const FInvoError& Error);

    /** HTTP 429 on any route. Hold the button for GetRetryAfterSeconds. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static bool IsRateLimited(const FInvoError& Error) { return Error.Status == 429; }

    /** Seconds to wait before retrying a throttled call, -1 when absent. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static float GetRetryAfterSeconds(const FInvoError& Error);

    /** A string field of the error body (approval_id, phone_last4, transferable_max, ...), empty when absent. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Errors")
    static FString GetErrorBodyString(const FInvoError& Error, const FString& Field);
};
