#include "InvoTypes.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"

/** Shown when a sandbox key reaches production or the other way round (HTTP 401). */
static const TCHAR* KeyForOtherEnvironmentMessage =
    TEXT("This key belongs to the other Invo environment. Check Sandbox/Production in Project Settings > Plugins > Invo SDK: ")
    TEXT("sandbox keys start with ivsdk_test_ and production keys (kept on your game server) with ivsdk_live_.");

/** Error tokens a few txn-state routes send in `error` with no `code`; promoted so callers can branch on Code. */
static bool IsPromotableErrorToken(const FString& Value)
{
    static const TCHAR* Tokens[] = {
        TEXT("flow_paused"), TEXT("spending_limit_exceeded"), TEXT("receiver_not_enrolled_use_claim_code"),
        TEXT("no_channel_on_file"), TEXT("rate_limited")
    };
    for (const TCHAR* Token : Tokens)
    {
        if (Value.Equals(Token, ESearchCase::CaseSensitive))
        {
            return true;
        }
    }
    return false;
}

static bool IsBareToken(const FString& Value)
{
    if (Value.IsEmpty() || Value.Len() > 64)
    {
        return false;
    }
    for (TCHAR C : Value)
    {
        if (!FChar::IsAlnum(C) && C != TEXT('_'))
        {
            return false;
        }
    }
    return true;
}

static FJsonObjectWrapper WrapObject(const TSharedPtr<FJsonObject>& Object)
{
    FJsonObjectWrapper Wrapper;
    Wrapper.JsonObject = Object.IsValid() ? Object : MakeShared<FJsonObject>();
    FString Text;
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Text);
    FJsonSerializer::Serialize(Wrapper.JsonObject.ToSharedRef(), Writer);
    Wrapper.JsonString = Text;
    return Wrapper;
}

FInvoError FInvoError::Make(const FString& InMessage, const FString& InCode, int32 InStatus)
{
    FInvoError Err;
    Err.Message = InMessage;
    Err.Code = InCode;
    Err.Status = InStatus;
    return Err;
}

FInvoError FInvoError::InvalidInput(const FString& Label, const FString& Value, const FString& Why)
{
    return Make(FString::Printf(TEXT("%s %s (got \"%s\")."), *Label, *Why, *Value), TEXT("INVALID_INPUT"), 0);
}

FInvoError FInvoError::FromResponse(int32 InStatus, const TSharedPtr<FJsonObject>& Parsed, const FString& RawText, const FString& InRequestId)
{
    FInvoError Err;
    Err.Status = InStatus;
    Err.RequestId = InRequestId;
    Err.Message = FString::Printf(TEXT("INVO request failed (HTTP %d)"), InStatus);

    if (Parsed.IsValid())
    {
        FString Value;
        if (Parsed->TryGetStringField(TEXT("code"), Value) || Parsed->TryGetStringField(TEXT("error_code"), Value))
        {
            Err.Code = Value;
        }
        FString ErrorField;
        const bool bHasError = Parsed->TryGetStringField(TEXT("error"), ErrorField);
        if (bHasError)
        {
            Err.Message = ErrorField;
        }
        else if (Parsed->TryGetStringField(TEXT("message"), Value))
        {
            Err.Message = Value;
        }
        if (Err.Code.IsEmpty() && bHasError && IsPromotableErrorToken(ErrorField))
        {
            Err.Code = ErrorField;
        }
        Err.Body = WrapObject(Parsed);
    }
    else
    {
        FString Trimmed = RawText;
        Trimmed.TrimStartAndEndInline();
        if (!Trimmed.IsEmpty())
        {
            Err.Message = RawText;
        }
    }

    if (Err.Code == TEXT("KEY_FOR_OTHER_ENVIRONMENT"))
    {
        Err.Message = KeyForOtherEnvironmentMessage;
    }
    return Err;
}

FInvoError& FInvoError::PromoteBareErrorToken()
{
    FString ErrorField;
    if (Code.IsEmpty() && Body.JsonObject.IsValid() && Body.JsonObject->TryGetStringField(TEXT("error"), ErrorField) && IsBareToken(ErrorField))
    {
        Code = ErrorField;
    }
    return *this;
}

FString FInvoError::ToString() const
{
    return FString::Printf(TEXT("InvoError(code=%s, status=%d, message=%s)"), *Code, Status, *Message);
}

FInvoResult FInvoResult::Success(const TSharedPtr<FJsonObject>& InData)
{
    FInvoResult Result;
    Result.bOk = true;
    Result.Data = WrapObject(InData);
    return Result;
}

FInvoResult FInvoResult::Failure(const FInvoError& InError)
{
    FInvoResult Result;
    Result.bOk = false;
    Result.Error = InError;
    return Result;
}

TSharedRef<FJsonObject> FInvoResult::Object() const
{
    return Data.JsonObject.IsValid() ? Data.JsonObject.ToSharedRef() : MakeShared<FJsonObject>();
}
