#include "InvoHttp.h"
#include "InvoLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"

DEFINE_LOG_CATEGORY_STATIC(LogInvoHttp, Log, All);

static bool IsAbsoluteUrl(const FString& Path)
{
    return Path.StartsWith(TEXT("https://")) || Path.StartsWith(TEXT("http://"));
}

static bool IsRetriableStatus(int32 Status)
{
    return Status == 429 || Status == 500 || Status == 502 || Status == 503 || Status == 504;
}

FString FInvoHttp::UserAgent()
{
    return TEXT("invonetwork-unreal-sdk/2.0.0 (+https://invo.network)");
}

FString FInvoHttp::Configure(const FString& InBaseUrl, int32 InMaxRetries, double InRetryBaseDelay)
{
    // Enforced here, not only in the subsystem: no path may downgrade to plaintext with credentials.
    const FString Problem = UInvoLibrary::SecureBaseUrlProblem(InBaseUrl);
    if (!Problem.IsEmpty())
    {
        UE_LOG(LogInvoHttp, Error, TEXT("[Invo] %s"), *Problem);
        BaseUrl.Empty();
        return Problem;
    }
    BaseUrl = InBaseUrl;
    while (BaseUrl.EndsWith(TEXT("/")))
    {
        BaseUrl.LeftChopInline(1);
    }
    MaxRetries = FMath::Max(0, InMaxRetries);
    RetryBaseDelay = InRetryBaseDelay;
    return FString();
}

void FInvoHttp::Get(const FString& Path, const TMap<FString, FString>& AuthHeaders, FInvoResultCallback Done)
{
    Start(TEXT("GET"), Path, nullptr, AuthHeaders, true, MoveTemp(Done));
}

void FInvoHttp::Post(const FString& Path, const TSharedPtr<FJsonObject>& Body, const TMap<FString, FString>& AuthHeaders, bool bIdempotent, FInvoResultCallback Done)
{
    Start(TEXT("POST"), Path, Body, AuthHeaders, bIdempotent, MoveTemp(Done));
}

void FInvoHttp::PostUrl(const FString& Url, const TSharedPtr<FJsonObject>& Body, const TMap<FString, FString>& AuthHeaders, FInvoResultCallback Done)
{
    const FString Problem = UInvoLibrary::SecureBaseUrlProblem(Url);
    if (!Problem.IsEmpty())
    {
        Done(FInvoResult::Failure(FInvoError::Make(Problem, TEXT("INVALID_INPUT"))));
        return;
    }
    Start(TEXT("POST"), Url, Body, AuthHeaders, false, MoveTemp(Done));
}

void FInvoHttp::Start(const FString& Verb, const FString& PathOrUrl, const TSharedPtr<FJsonObject>& Body, const TMap<FString, FString>& AuthHeaders, bool bIdempotent, FInvoResultCallback Done)
{
    if (!IsAbsoluteUrl(PathOrUrl) && BaseUrl.IsEmpty())
    {
        Done(FInvoResult::Failure(FInvoError::Make(TEXT("The INVO SDK is not configured (no base URL)."), TEXT("NOT_CONFIGURED"))));
        return;
    }

    TSharedRef<FPending> Pending = MakeShared<FPending>();
    Pending->Verb = Verb;
    Pending->Url = IsAbsoluteUrl(PathOrUrl) ? PathOrUrl : BaseUrl + PathOrUrl;
    Pending->bIdempotent = bIdempotent;
    Pending->Done = MoveTemp(Done);
    Pending->Headers.Add(TEXT("Accept"), TEXT("application/json"));
    Pending->Headers.Add(TEXT("User-Agent"), UserAgent());
    for (const TPair<FString, FString>& Header : AuthHeaders)
    {
        Pending->Headers.Add(Header.Key, UInvoLibrary::SanitizeHeaderValue(Header.Value));
    }
    if (Body.IsValid())
    {
        Pending->Headers.Add(TEXT("Content-Type"), TEXT("application/json"));
        TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Pending->BodyText);
        FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);
    }
    Send(Pending);
}

void FInvoHttp::Send(TSharedRef<FPending> Pending)
{
    TWeakPtr<FInvoHttp> WeakThis = AsShared();
    TFunction<void(const FInvoRawResponse&)> OnDone = [WeakThis, Pending](const FInvoRawResponse& Raw)
    {
        if (TSharedPtr<FInvoHttp> This = WeakThis.Pin())
        {
            This->OnResponse(Pending, Raw);
        }
    };
    if (Transport)
    {
        Transport(Pending->Verb, Pending->Url, Pending->Headers, Pending->BodyText, MoveTemp(OnDone));
    }
    else
    {
        RunNetwork(*Pending, MoveTemp(OnDone));
    }
}

void FInvoHttp::RunNetwork(const FPending& Pending, TFunction<void(const FInvoRawResponse&)> OnDone) const
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Pending.Url);
    Request->SetVerb(Pending.Verb);
    Request->SetTimeout(DefaultTimeout);
    for (const TPair<FString, FString>& Header : Pending.Headers)
    {
        Request->SetHeader(Header.Key, Header.Value);
    }
    if (!Pending.BodyText.IsEmpty())
    {
        Request->SetContentAsString(Pending.BodyText);
    }
    Request->OnProcessRequestComplete().BindLambda(
        [OnDone = MoveTemp(OnDone)](FHttpRequestPtr, FHttpResponsePtr Response, bool bConnectedSuccessfully)
        {
            FInvoRawResponse Raw;
            if (!bConnectedSuccessfully || !Response.IsValid())
            {
                Raw.bNetworkError = true;
                Raw.NetworkMessage = TEXT("network error or timeout");
                OnDone(Raw);
                return;
            }
            Raw.Status = Response->GetResponseCode();
            Raw.Body = Response->GetContentAsString();
            for (const FString& Line : Response->GetAllHeaders())
            {
                FString Name, Value;
                if (Line.Split(TEXT(":"), &Name, &Value))
                {
                    Raw.Headers.Add(Name.TrimStartAndEnd().ToLower(), Value.TrimStartAndEnd());
                }
            }
            OnDone(Raw);
        });
    Request->ProcessRequest();
}

void FInvoHttp::OnResponse(TSharedRef<FPending> Pending, const FInvoRawResponse& Raw)
{
    const FString LoggableUrl = UInvoLibrary::RedactUrl(Pending->Url);

    if (Raw.bNetworkError)
    {
        const FInvoError Err = FInvoError::Make(FString::Printf(TEXT("Network error calling %s: %s"), *LoggableUrl, *Raw.NetworkMessage));
        if (Pending->bIdempotent && Pending->Attempt < MaxRetries)
        {
            UE_LOG(LogInvoHttp, Verbose, TEXT("%s %s: %s (retrying)"), *Pending->Verb, *LoggableUrl, *Raw.NetworkMessage);
            RetryAfter(Pending, Backoff(Pending->Attempt));
            return;
        }
        UE_LOG(LogInvoHttp, Warning, TEXT("%s %s: %s"), *Pending->Verb, *LoggableUrl, *Raw.NetworkMessage);
        Pending->Done(FInvoResult::Failure(Err));
        return;
    }

    TSharedPtr<FJsonObject> Parsed;
    if (!Raw.Body.IsEmpty())
    {
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Raw.Body);
        if (!FJsonSerializer::Deserialize(Reader, Parsed))
        {
            Parsed.Reset();
        }
    }
    const FString* RequestId = Raw.Headers.Find(TEXT("x-invo-request-id"));
    if (!RequestId)
    {
        RequestId = Raw.Headers.Find(TEXT("x-request-id"));
    }

    if (Raw.Status < 200 || Raw.Status >= 300)
    {
        const FInvoError Err = FInvoError::FromResponse(Raw.Status, Parsed, Raw.Body, RequestId ? *RequestId : FString());
        double Wait = -1.0;
        if (Pending->bIdempotent && Pending->Attempt < MaxRetries && IsRetriableStatus(Raw.Status))
        {
            if (Raw.Status == 429)
            {
                const FString* Header = Raw.Headers.Find(TEXT("retry-after"));
                const double RetryAfterSecs = UInvoLibrary::RetryAfterSeconds(Parsed, Header ? *Header : FString());
                if (RetryAfterSecs < 0.0)
                {
                    Wait = Backoff(Pending->Attempt);
                }
                else if (RetryAfterSecs <= MaxRetryAfter)
                {
                    Wait = RetryAfterSecs;
                }
                // Longer than MaxRetryAfter: surface the 429 rather than stall the game.
            }
            else
            {
                Wait = Backoff(Pending->Attempt);
            }
        }
        UE_LOG(LogInvoHttp, Log, TEXT("%s %s -> %d %s%s"), *Pending->Verb, *LoggableUrl, Raw.Status, *Err.Code, Wait >= 0.0 ? TEXT(" (retrying)") : TEXT(""));
        if (Wait >= 0.0)
        {
            RetryAfter(Pending, Wait);
            return;
        }
        Pending->Done(FInvoResult::Failure(Err));
        return;
    }

    Pending->Done(FInvoResult::Success(Parsed));
}

void FInvoHttp::RetryAfter(TSharedRef<FPending> Pending, double Seconds)
{
    Pending->Attempt++;
    TWeakPtr<FInvoHttp> WeakThis = AsShared();
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis, Pending](float)
    {
        if (TSharedPtr<FInvoHttp> This = WeakThis.Pin())
        {
            This->Send(Pending);
        }
        return false;
    }), static_cast<float>(Seconds));
}

double FInvoHttp::Backoff(int32 Attempt) const
{
    const double Ceiling = RetryBaseDelay * FMath::Pow(2.0, static_cast<double>(Attempt));
    return FMath::FRand() * Ceiling + RetryBaseDelay;
}
