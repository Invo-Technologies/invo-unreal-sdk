#pragma once

#include "CoreMinimal.h"
#include "InvoTypes.h"

class FJsonObject;

/** What a transport hands back: a network failure, or a status with headers and body text. */
struct FInvoRawResponse
{
    bool bNetworkError = false;
    FString NetworkMessage;
    int32 Status = 0;
    /** Header names lower-cased. */
    TMap<FString, FString> Headers;
    FString Body;
};

/** Test and mock hook: (verb, url, headers, body text, done). */
using FInvoTransport = TFunction<void(const FString&, const FString&, const TMap<FString, FString>&, const FString&, TFunction<void(const FInvoRawResponse&)>)>;

/**
 * HTTP core for the INVO Unreal SDK, a port of the Python/Godot SDKs' transport rules:
 *  - Retries apply ONLY to idempotent requests (network error, 429 honouring retry_after up to
 *    20 s, and 5xx). A non-idempotent POST is never auto-retried: a timeout can fire while the
 *    server is mid-way through moving money.
 *  - Exponential backoff with jitter.
 *  - https is enforced (loopback exempt): credentials travel in headers.
 *  - 3xx is reported as an error rather than treated as success.
 *  - An explicit User-Agent (the backend refuses blank and scanner agents).
 *  - Logs never carry query strings or the by-email path segment (they hold player PII).
 */
class FInvoHttp : public TSharedFromThis<FInvoHttp>
{
public:
    static constexpr float DefaultTimeout = 30.f;
    static constexpr double MaxRetryAfter = 20.0;

    /** Empty when accepted; the reason otherwise (and the base URL is cleared). */
    FString Configure(const FString& InBaseUrl, int32 InMaxRetries = 2, double InRetryBaseDelay = 0.25);

    const FString& GetBaseUrl() const { return BaseUrl; }

    /** GET: always idempotent, so safe to retry. Path is relative to the base URL. */
    void Get(const FString& Path, const TMap<FString, FString>& AuthHeaders, FInvoResultCallback Done);

    /** POST with a JSON body (null sends none). bIdempotent marks it safe to auto-retry. */
    void Post(const FString& Path, const TSharedPtr<FJsonObject>& Body, const TMap<FString, FString>& AuthHeaders, bool bIdempotent, FInvoResultCallback Done);

    /** POST to an absolute URL (your own token or checkout endpoint). The same https rule applies. */
    void PostUrl(const FString& Url, const TSharedPtr<FJsonObject>& Body, const TMap<FString, FString>& AuthHeaders, FInvoResultCallback Done);

    /** Replaces the network with Transport (tests and the offline mock). Pass nullptr to restore. */
    void SetTransport(FInvoTransport InTransport) { Transport = MoveTemp(InTransport); }
    bool HasTransportOverride() const { return (bool)Transport; }

    static FString UserAgent();

private:
    struct FPending
    {
        FString Verb;
        FString Url;
        TMap<FString, FString> Headers;
        FString BodyText;
        bool bIdempotent = false;
        int32 Attempt = 0;
        FInvoResultCallback Done;
    };

    FString BaseUrl;
    int32 MaxRetries = 2;
    double RetryBaseDelay = 0.25;
    FInvoTransport Transport;

    void Start(const FString& Verb, const FString& PathOrUrl, const TSharedPtr<FJsonObject>& Body, const TMap<FString, FString>& AuthHeaders, bool bIdempotent, FInvoResultCallback Done);
    void Send(TSharedRef<FPending> Pending);
    void OnResponse(TSharedRef<FPending> Pending, const FInvoRawResponse& Raw);
    void RetryAfter(TSharedRef<FPending> Pending, double Seconds);
    double Backoff(int32 Attempt) const;
    void RunNetwork(const FPending& Pending, TFunction<void(const FInvoRawResponse&)> OnDone) const;
};
