#include "InvoLibrary.h"
#include "Dom/JsonObject.h"
#include "Misc/Guid.h"

bool UInvoLibrary::IsValidMoney(const FString& Value, double MaxValue)
{
    // ^\d+(\.\d{1,2})?$ by hand: no regex dependency, and no trailing-newline loophole.
    int32 Dot = INDEX_NONE;
    int32 Digits = 0;
    for (int32 i = 0; i < Value.Len(); ++i)
    {
        const TCHAR C = Value[i];
        if (C == TEXT('.'))
        {
            if (Dot != INDEX_NONE || Digits == 0)
            {
                return false;
            }
            Dot = i;
        }
        else if (C >= TEXT('0') && C <= TEXT('9'))
        {
            ++Digits;
        }
        else
        {
            return false;
        }
    }
    if (Digits == 0)
    {
        return false;
    }
    if (Dot != INDEX_NONE)
    {
        const int32 Places = Value.Len() - Dot - 1;
        if (Places < 1 || Places > 2)
        {
            return false;
        }
    }
    const double N = FCString::Atod(*Value);
    return FMath::IsFinite(N) && N > 0.0 && N <= MaxValue;
}

bool UInvoLibrary::IsE164Phone(const FString& Value)
{
    if (Value.Len() < 3 || Value.Len() > 16 || Value[0] != TEXT('+') || Value[1] < TEXT('1') || Value[1] > TEXT('9'))
    {
        return false;
    }
    for (int32 i = 2; i < Value.Len(); ++i)
    {
        if (Value[i] < TEXT('0') || Value[i] > TEXT('9'))
        {
            return false;
        }
    }
    return true;
}

FString UInvoLibrary::FormatMoney(double Value)
{
    const int64 Cents = FMath::RoundToInt64(Value * 100.0);
    const int64 Whole = Cents / 100;
    const int64 Frac = FMath::Abs(Cents % 100);
    if (Frac == 0)
    {
        return FString::Printf(TEXT("%lld"), Whole);
    }
    if (Frac % 10 == 0)
    {
        return FString::Printf(TEXT("%lld.%lld"), Whole, Frac / 10);
    }
    return FString::Printf(TEXT("%lld.%02lld"), Whole, Frac);
}

FString UInvoLibrary::NewClientRequestId()
{
    // FGuid::NewGuid uses the OS generator (CoCreateGuid / uuid) - 122 random bits.
    return TEXT("unreal-") + FGuid::NewGuid().ToString(EGuidFormats::Digits).ToLower();
}

FString UInvoLibrary::SecureBaseUrlProblem(const FString& Url)
{
    FString Scheme, Rest;
    if (!Url.Split(TEXT("://"), &Scheme, &Rest) || Rest.IsEmpty())
    {
        return FString::Printf(TEXT("Invalid base URL: %s"), *Url);
    }
    Scheme.ToLowerInline();
    FString HostPort = Rest;
    HostPort.Split(TEXT("/"), &HostPort, nullptr);
    HostPort.ToLowerInline();
    FString Host = HostPort;
    if (HostPort.StartsWith(TEXT("[")))
    {
        int32 Close = INDEX_NONE;
        if (HostPort.FindChar(TEXT(']'), Close))
        {
            Host = HostPort.Left(Close + 1);
        }
    }
    else
    {
        HostPort.Split(TEXT(":"), &Host, nullptr);
    }
    const bool bLocal = Host == TEXT("localhost") || Host == TEXT("127.0.0.1") || Host == TEXT("[::1]");
    if (Scheme == TEXT("https") || (Scheme == TEXT("http") && bLocal))
    {
        return FString();
    }
    return FString::Printf(TEXT("Base URL must use https:// (got \"%s://\"). Plaintext would expose credentials on the wire."), *Scheme);
}

FString UInvoLibrary::SanitizeHeaderValue(const FString& Value)
{
    FString Out;
    Out.Reserve(Value.Len());
    for (TCHAR C : Value)
    {
        if (C >= 0x20 && C != 0x7F)
        {
            Out.AppendChar(C);
        }
    }
    return Out;
}

FString UInvoLibrary::RedactUrl(const FString& Url)
{
    FString Out = Url;
    Out.Split(TEXT("?"), &Out, nullptr);
    const int32 Idx = Out.Find(TEXT("/by-email/"));
    if (Idx != INDEX_NONE)
    {
        Out = Out.Left(Idx) + TEXT("/by-email/<redacted>");
    }
    return Out;
}

FString UInvoLibrary::KeyEnvironment(const FString& Key)
{
    if (Key.StartsWith(TEXT("ivsdk_test_"), ESearchCase::CaseSensitive))
    {
        return TEXT("Sandbox");
    }
    if (Key.StartsWith(TEXT("ivsdk_live_"), ESearchCase::CaseSensitive))
    {
        return TEXT("Production");
    }
    return FString();
}

static double ParseNonNegative(const FString& Text)
{
    FString Trimmed = Text;
    Trimmed.TrimStartAndEndInline();
    if (Trimmed.IsEmpty() || !Trimmed.IsNumeric())
    {
        return -1.0;
    }
    const double N = FCString::Atod(*Trimmed);
    return N >= 0.0 ? N : -1.0;
}

double UInvoLibrary::RetryAfterSeconds(const TSharedPtr<FJsonObject>& Body, const FString& RetryAfterHeader)
{
    if (Body.IsValid())
    {
        const TSharedPtr<FJsonValue> V = Body->TryGetField(TEXT("retry_after"));
        if (V.IsValid())
        {
            if (V->Type == EJson::Number && V->AsNumber() >= 0.0)
            {
                return V->AsNumber();
            }
            if (V->Type == EJson::String)
            {
                const double N = ParseNonNegative(V->AsString());
                if (N >= 0.0)
                {
                    return N;
                }
            }
        }
    }
    return ParseNonNegative(RetryAfterHeader);
}

bool UInvoLibrary::IsReceiverNotEnrolled(const FInvoError& Error)
{
    return Error.Code == TEXT("receiver_not_enrolled_use_claim_code")
        || Error.Message.Contains(TEXT("receiver_not_enrolled_use_claim_code"), ESearchCase::IgnoreCase);
}

bool UInvoLibrary::IsInsufficientBalance(const FInvoError& Error)
{
    if (Error.Status != 400)
    {
        return false;
    }
    return Error.Code == TEXT("INSUFFICIENT_BALANCE")
        || (Error.Body.JsonObject.IsValid() && Error.Body.JsonObject->HasField(TEXT("required_amount")))
        || Error.Message.Contains(TEXT("insufficient balance"), ESearchCase::IgnoreCase)
        || Error.Message.Contains(TEXT("insufficient_balance"), ESearchCase::IgnoreCase);
}

bool UInvoLibrary::IsDuplicateRequest(const FInvoError& Error)
{
    return Error.Code == TEXT("DUPLICATE_REQUEST")
        || (Error.Status == 409 && Error.Message.Contains(TEXT("duplicate"), ESearchCase::IgnoreCase));
}

float UInvoLibrary::GetRetryAfterSeconds(const FInvoError& Error)
{
    return static_cast<float>(RetryAfterSeconds(Error.Body.JsonObject, FString()));
}

FString UInvoLibrary::GetErrorBodyString(const FInvoError& Error, const FString& Field)
{
    FString Value;
    if (Error.Body.JsonObject.IsValid())
    {
        Error.Body.JsonObject->TryGetStringField(Field, Value);
    }
    return Value;
}
