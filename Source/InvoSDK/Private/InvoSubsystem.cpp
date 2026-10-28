#include "InvoSubsystem.h"
#include "InvoHttp.h"
#include "InvoLibrary.h"
#include "InvoSDKSettings.h"
#include "InvoHostedApproval.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformMisc.h"

DEFINE_LOG_CATEGORY_STATIC(LogInvo, Log, All);

namespace
{
    const TCHAR* SecretEnvVar = TEXT("INVO_SANDBOX_SECRET");

    bool IsPresent(const FString& Value)
    {
        return !Value.TrimStartAndEnd().IsEmpty();
    }

    bool IsApprovalFlow(const FString& Flow)
    {
        return Flow == TEXT("transfer") || Flow == TEXT("send") || Flow == TEXT("send_receipt") || Flow == TEXT("transfer_receipt");
    }

    bool IsPollOutcome(const FString& Value)
    {
        return Value == TEXT("approved") || Value == TEXT("authorization_pending") || Value == TEXT("slow_down")
            || Value == TEXT("access_denied") || Value == TEXT("expired_token");
    }

    FInvoResult Fail(const FInvoError& Error)
    {
        return FInvoResult::Failure(Error);
    }

    /** Device-approval routes answer RFC 8628 errors as a bare `error` token; promote it to Code. */
    FInvoResult WithBareCode(const FInvoResult& Result)
    {
        FInvoResult Out = Result;
        if (!Out.bOk)
        {
            Out.Error.PromoteBareErrorToken();
        }
        return Out;
    }

    /** Money amounts go on the wire as JSON numbers parsed from the validated decimal string. */
    void SetMoneyField(const TSharedRef<FJsonObject>& Body, const FString& Field, const FString& Value)
    {
        Body->SetNumberField(Field, FCString::Atod(*Value));
    }

    /** Game ids are numeric on the platform; send them as numbers. */
    void SetGameIdField(const TSharedRef<FJsonObject>& Body, const FString& Field, const FString& Value)
    {
        Body->SetNumberField(Field, FCString::Atod(*Value));
    }
}

void UInvoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Http = MakeShared<FInvoHttp>();
    const UInvoSDKSettings* Settings = GetDefault<UInvoSDKSettings>();
    Setup(Settings->Environment);
}

void UInvoSubsystem::Deinitialize()
{
    // In-flight requests hold weak pointers to the transport, so dropping it cancels their callbacks.
    Http.Reset();
    GameServerHttp.Reset();
    RefreshWaiters.Empty();
    Super::Deinitialize();
}

FString UInvoSubsystem::DefaultBaseUrl(EInvoEnvironment InEnvironment)
{
    return InEnvironment == EInvoEnvironment::Production ? TEXT("https://invo.network") : TEXT("https://sandbox.invo.network/sandbox");
}

bool UInvoSubsystem::Setup(EInvoEnvironment InEnvironment, const FString& BaseUrlOverride)
{
    const FString Base = BaseUrlOverride.IsEmpty() ? DefaultBaseUrl(InEnvironment) : BaseUrlOverride;
    const FString Problem = UInvoLibrary::SecureBaseUrlProblem(Base);
    if (!Problem.IsEmpty())
    {
        // Refuse BEFORE changing anything: a failed Setup leaves the working configuration intact.
        UE_LOG(LogInvo, Error, TEXT("[Invo] %s"), *Problem);
        return false;
    }
    Environment = InEnvironment;
    GameId = GetDefault<UInvoSDKSettings>()->GameIdFor(InEnvironment);
    Http->Configure(Base);
    GameServerHttp.Reset();
    return true;
}

// ---------------------------------------------------------------- routing

FString UInvoSubsystem::ResolveSandboxSecret()
{
    FString Secret = FPlatformMisc::GetEnvironmentVariable(SecretEnvVar);
    Secret.TrimStartAndEndInline();
    if (Secret.IsEmpty())
    {
        Secret = GetDefault<UInvoSDKLocalSettings>()->SandboxSecret;
        Secret.TrimStartAndEndInline();
    }
    return Secret;
}

FString UInvoSubsystem::CommerceProblem() const
{
    if (Http.IsValid() && Http->HasTransportOverride())
    {
        return FString();
    }
    if (Environment == EInvoEnvironment::Sandbox)
    {
#if UE_BUILD_SHIPPING
        return TEXT("Direct sandbox mode is disabled in Shipping builds: a key inside a shipped game is exposed to every player. Use production with your game server.");
#else
        const FString Secret = ResolveSandboxSecret();
        if (Secret.IsEmpty())
        {
            return TEXT("No sandbox Platform API key. Set INVO_SANDBOX_SECRET, or Project Settings > Plugins > Invo SDK (Local). Get one in Test mode at console.invo.network/test.");
        }
        if (UInvoLibrary::KeyEnvironment(Secret) == TEXT("Production"))
        {
            return TEXT("That is a production key (ivsdk_live_). Production keys stay on your game server; the sandbox takes an ivsdk_test_ key.");
        }
        return FString();
#endif
    }
    FString Server = GetDefault<UInvoSDKSettings>()->GameServerUrl;
    Server.TrimStartAndEndInline();
    if (Server.IsEmpty())
    {
        return TEXT("Set Game Server URL in Project Settings > Plugins > Invo SDK: in production the store, send and transfer calls go through your game server.");
    }
    return UInvoLibrary::SecureBaseUrlProblem(Server);
}

bool UInvoSubsystem::IsCommerceReady() const
{
    return CommerceProblem().IsEmpty();
}

bool UInvoSubsystem::CommerceRoute(TSharedPtr<FInvoHttp>& OutHttp, TMap<FString, FString>& OutAuth, FInvoError& OutError)
{
    const FString Problem = CommerceProblem();
    if (!Problem.IsEmpty())
    {
        OutError = FInvoError::Make(Problem, Environment == EInvoEnvironment::Sandbox ? TEXT("SANDBOX_ONLY") : TEXT("NO_GAME_SERVER"));
        return false;
    }
    if (Environment == EInvoEnvironment::Sandbox || Http->HasTransportOverride())
    {
        OutHttp = Http;
        OutAuth.Add(TEXT("X-Game-Secret-Key"), Http->HasTransportOverride() ? TEXT("mock-secret") : ResolveSandboxSecret());
        return true;
    }
    if (!GameServerHttp.IsValid())
    {
        GameServerHttp = MakeShared<FInvoHttp>();
        GameServerHttp->Configure(GetDefault<UInvoSDKSettings>()->GameServerUrl.TrimStartAndEnd());
    }
    OutHttp = GameServerHttp;
    OutAuth = PlayerAuth();
    return true;
}

void UInvoSubsystem::CommerceGet(const FString& Path, FInvoResultCallback Done)
{
    TSharedPtr<FInvoHttp> Route;
    TMap<FString, FString> Auth;
    FInvoError Error;
    if (!CommerceRoute(Route, Auth, Error))
    {
        Done(Fail(Error));
        return;
    }
    Route->Get(Path, Auth, MoveTemp(Done));
}

void UInvoSubsystem::CommercePost(const FString& Path, TSharedPtr<FJsonObject> Body, bool bIdempotent, FInvoResultCallback Done)
{
    TSharedPtr<FInvoHttp> Route;
    TMap<FString, FString> Auth;
    FInvoError Error;
    if (!CommerceRoute(Route, Auth, Error))
    {
        Done(Fail(Error));
        return;
    }
    Route->Post(Path, Body, Auth, bIdempotent, MoveTemp(Done));
}

TMap<FString, FString> UInvoSubsystem::PlayerAuth() const
{
    TMap<FString, FString> Auth;
    if (!PlayerToken.IsEmpty())
    {
        Auth.Add(TEXT("Authorization"), TEXT("Bearer ") + PlayerToken);
    }
    return Auth;
}

FInvoResultCallback UInvoSubsystem::Wrap(const FInvoResultDelegate& OnDone)
{
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    return [WeakThis, OnDone](const FInvoResult& Result)
    {
        if (!Result.bOk && WeakThis.IsValid())
        {
            WeakThis->OnRequestFailed.Broadcast(Result.Error);
        }
        OnDone.ExecuteIfBound(Result);
    };
}

FInvoResultCallback UInvoSubsystem::WrapNative(FInvoResultCallback Done)
{
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    return [WeakThis, Done = MoveTemp(Done)](const FInvoResult& Result)
    {
        if (!Result.bOk && WeakThis.IsValid())
        {
            WeakThis->OnRequestFailed.Broadcast(Result.Error);
        }
        if (Done)
        {
            Done(Result);
        }
    };
}

void UInvoSubsystem::SetTransportForTesting(TFunction<void(const FString&, const FString&, const TMap<FString, FString>&, const FString&, TFunction<void(const FInvoRawResponse&)>)> Transport)
{
#if UE_BUILD_SHIPPING
    UE_LOG(LogInvo, Error, TEXT("[Invo] A test transport is refused in Shipping builds: it would fake commerce results."));
#else
    Http->SetTransport(MoveTemp(Transport));
    GameServerHttp.Reset();
#endif
}

// ---------------------------------------------------------------- player

void UInvoSubsystem::SetPlayerToken(const FString& Token, const FString& IdentityId)
{
    PlayerToken = UInvoLibrary::SanitizeHeaderValue(Token.TrimStartAndEnd());
    if (!PlayerToken.IsEmpty())
    {
        OnLoggedIn.Broadcast(IdentityId);
    }
}

void UInvoSubsystem::SetActivePlayer(const FString& Email, const FString& Phone, const FString& DisplayName)
{
    PlayerEmail = Email.TrimStartAndEnd();
    PlayerPhone = Phone.TrimStartAndEnd();
    PlayerName = DisplayName.TrimStartAndEnd();
}

bool UInvoSubsystem::RequireActivePlayer(FInvoResultCallback& Done, bool bNeedPhone)
{
    if (!IsPresent(PlayerEmail))
    {
        Done(Fail(FInvoError::Make(TEXT("No active player. Call SetActivePlayer after your own login."), TEXT("NO_ACTIVE_PLAYER"))));
        return false;
    }
    if (bNeedPhone && !UInvoLibrary::IsE164Phone(PlayerPhone))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("playerPhone"), PlayerPhone, TEXT("must be an E.164 phone number, e.g. \"+15555550100\""))));
        return false;
    }
    return true;
}

void UInvoSubsystem::LoginViaBackend(const FString& PayloadJson, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    FString Endpoint = GetDefault<UInvoSDKSettings>()->PlayerTokenEndpoint;
    Endpoint.TrimStartAndEndInline();
    if (Endpoint.IsEmpty())
    {
        Done(Fail(FInvoError::Make(TEXT("No Player Token Endpoint configured. Set it in Project Settings > Plugins > Invo SDK to an endpoint on YOUR backend that mints player tokens."), TEXT("NO_TOKEN_ENDPOINT"))));
        return;
    }
    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    if (!PayloadJson.IsEmpty())
    {
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(PayloadJson);
        if (!FJsonSerializer::Deserialize(Reader, Payload) || !Payload.IsValid())
        {
            Done(Fail(FInvoError::Make(TEXT("PayloadJson is not a JSON object."), TEXT("INVALID_INPUT"))));
            return;
        }
    }
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    Http->PostUrl(Endpoint, Payload, {}, [WeakThis, Endpoint, PayloadJson, Done](const FInvoResult& Result)
    {
        if (!Result.bOk || !WeakThis.IsValid())
        {
            Done(Result);
            return;
        }
        FString Token, IdentityId;
        Result.Object()->TryGetStringField(TEXT("token"), Token);
        Result.Object()->TryGetStringField(TEXT("identity_id"), IdentityId);
        if (Token.IsEmpty())
        {
            Done(Fail(FInvoError::Make(TEXT("The token endpoint responded without a `token` field."), TEXT("INVALID_RESPONSE"))));
            return;
        }
        WeakThis->TokenEndpoint = Endpoint;
        WeakThis->TokenPayloadJson = PayloadJson;
        WeakThis->SetPlayerToken(Token, IdentityId);
        Done(Result);
    });
}

void UInvoSubsystem::LoginSandbox(const FString& Email, const FString& Phone, const FString& DisplayName, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (Environment != EInvoEnvironment::Sandbox)
    {
        Done(Fail(FInvoError::Make(TEXT("LoginSandbox is sandbox-only. In production your backend mints player tokens: use LoginViaBackend."), TEXT("SANDBOX_ONLY"))));
        return;
    }
    if (!IsPresent(Email))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("playerEmail"), Email, TEXT("is required"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("player_email"), Email.TrimStartAndEnd());
    if (!Phone.IsEmpty())
    {
        if (!UInvoLibrary::IsE164Phone(Phone))
        {
            Done(Fail(FInvoError::InvalidInput(TEXT("playerPhone"), Phone, TEXT("must be an E.164 phone number, e.g. \"+15555550100\""))));
            return;
        }
        Body->SetStringField(TEXT("player_phone"), Phone);
    }
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    CommercePost(TEXT("/api/sdk/player-token"), Body, true, [WeakThis, Email, Phone, DisplayName, Done](const FInvoResult& Result)
    {
        if (!Result.bOk || !WeakThis.IsValid())
        {
            Done(Result);
            return;
        }
        FString Token, IdentityId;
        Result.Object()->TryGetStringField(TEXT("token"), Token);
        Result.Object()->TryGetStringField(TEXT("identity_id"), IdentityId);
        if (Token.IsEmpty())
        {
            Done(Fail(FInvoError::Make(TEXT("INVO response was missing the required `token` field."), TEXT("INVALID_RESPONSE"))));
            return;
        }
        WeakThis->SetActivePlayer(Email, Phone, DisplayName);
        WeakThis->SetPlayerToken(Token, IdentityId);
        Done(Result);
    });
}

void UInvoSubsystem::RefreshToken(TFunction<void(bool)> Done)
{
    if (TokenEndpoint.IsEmpty())
    {
        Done(false);
        return;
    }
    // Single flight: concurrent expiries share one re-mint and all learn whether THAT one worked.
    RefreshWaiters.Add(MoveTemp(Done));
    if (bRefreshing)
    {
        return;
    }
    bRefreshing = true;
    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    if (!TokenPayloadJson.IsEmpty())
    {
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(TokenPayloadJson);
        FJsonSerializer::Deserialize(Reader, Payload);
    }
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    Http->PostUrl(TokenEndpoint, Payload, {}, [WeakThis](const FInvoResult& Result)
    {
        if (!WeakThis.IsValid())
        {
            return;
        }
        FString Token;
        const bool bOk = Result.bOk && Result.Object()->TryGetStringField(TEXT("token"), Token) && !Token.IsEmpty();
        if (bOk)
        {
            WeakThis->PlayerToken = UInvoLibrary::SanitizeHeaderValue(Token);
        }
        WeakThis->bRefreshing = false;
        TArray<TFunction<void(bool)>> Waiters = MoveTemp(WeakThis->RefreshWaiters);
        WeakThis->RefreshWaiters.Reset();
        for (TFunction<void(bool)>& Waiter : Waiters)
        {
            Waiter(bOk);
        }
    });
}

static FInvoResult NoTokenFailure()
{
    return FInvoResult::Failure(FInvoError::Make(
        TEXT("No player token set. Call LoginViaBackend, SetPlayerToken with a token your backend minted, or (sandbox) LoginSandbox."),
        TEXT("NO_PLAYER_TOKEN")));
}

void UInvoSubsystem::PlayerGet(const FString& Path, FInvoResultCallback Done)
{
    if (!HasPlayerToken())
    {
        Done(NoTokenFailure());
        return;
    }
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    Http->Get(Path, PlayerAuth(), [WeakThis, Path, Done](const FInvoResult& Result)
    {
        if (Result.bOk || !UInvoLibrary::IsTokenExpired(Result.Error) || !WeakThis.IsValid())
        {
            Done(Result);
            return;
        }
        WeakThis->RefreshToken([WeakThis, Path, Done, Result](bool bRefreshed)
        {
            if (!bRefreshed || !WeakThis.IsValid())
            {
                Done(Result);
                return;
            }
            WeakThis->Http->Get(Path, WeakThis->PlayerAuth(), Done);
        });
    });
}

void UInvoSubsystem::PlayerPost(const FString& Path, TSharedPtr<FJsonObject> Body, FInvoResultCallback Done)
{
    if (!HasPlayerToken())
    {
        Done(NoTokenFailure());
        return;
    }
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    Http->Post(Path, Body, PlayerAuth(), false, [WeakThis, Path, Body, Done](const FInvoResult& Result)
    {
        if (Result.bOk || !UInvoLibrary::IsTokenExpired(Result.Error) || !WeakThis.IsValid())
        {
            Done(Result);
            return;
        }
        WeakThis->RefreshToken([WeakThis, Path, Body, Done, Result](bool bRefreshed)
        {
            if (!bRefreshed || !WeakThis.IsValid())
            {
                Done(Result);
                return;
            }
            WeakThis->Http->Post(Path, Body, WeakThis->PlayerAuth(), false, Done);
        });
    });
}

void UInvoSubsystem::GetBalance(const FInvoResultDelegate& OnDone)
{
    // GetBalanceNative already reports failures on OnRequestFailed; only the delegate is left.
    GetBalanceNative([OnDone](const FInvoResult& Result) { OnDone.ExecuteIfBound(Result); });
}

void UInvoSubsystem::GetBalanceNative(FInvoResultCallback Done)
{
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    PlayerGet(TEXT("/api/sdk/balance"), [WeakThis, Done = WrapNative(MoveTemp(Done))](const FInvoResult& Result)
    {
        if (Result.bOk && WeakThis.IsValid())
        {
            WeakThis->OnBalanceUpdated.Broadcast(Result);
        }
        Done(Result);
    });
}

void UInvoSubsystem::GetDestinations(const FString& Direction, const FInvoResultDelegate& OnDone)
{
    const FString Dir = Direction == TEXT("send") ? TEXT("send") : TEXT("transfer");
    PlayerGet(FString::Printf(TEXT("/api/sdk/destinations?direction=%s"), *Dir), Wrap(OnDone));
}

void UInvoSubsystem::GetPendingCollect(const FInvoResultDelegate& OnDone)
{
    PlayerGet(TEXT("/api/sdk/transfers/pending"), Wrap(OnDone));
}

// ---------------------------------------------------------------- phone approval

FString UInvoSubsystem::DefaultApprovalChannel()
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    return TEXT("app_browser");
#else
    return TEXT("qr");
#endif
}

static FInvoError TxnFlowProblem(const FString& TransactionId, const FString& Flow)
{
    if (!IsPresent(TransactionId))
    {
        return FInvoError::InvalidInput(TEXT("transactionId"), TransactionId, TEXT("is required"));
    }
    if (!IsApprovalFlow(Flow))
    {
        return FInvoError::InvalidInput(TEXT("flow"), Flow, TEXT("must be transfer, send, send_receipt or transfer_receipt"));
    }
    return FInvoError();
}

void UInvoSubsystem::BeginPhoneApproval(const FString& TransactionId, const FString& Flow, const FString& Channel, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    const FInvoError Problem = TxnFlowProblem(TransactionId, Flow);
    if (!Problem.Code.IsEmpty())
    {
        Done(Fail(Problem));
        return;
    }
    const FString Ch = Channel.IsEmpty() ? DefaultApprovalChannel() : Channel;
    if (Ch != TEXT("qr") && Ch != TEXT("app_browser"))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("channel"), Ch, TEXT("must be \"qr\" or \"app_browser\""))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("transaction_id"), TransactionId);
    Body->SetStringField(TEXT("flow"), Flow);
    Body->SetStringField(TEXT("channel"), Ch);
    PlayerPost(TEXT("/api/sdk/approvals/device/begin"), Body, [Done](const FInvoResult& Result)
    {
        if (!Result.bOk)
        {
            Done(WithBareCode(Result));
            return;
        }
        FString DeviceCode, Uri;
        Result.Object()->TryGetStringField(TEXT("device_code"), DeviceCode);
        Result.Object()->TryGetStringField(TEXT("verification_uri_complete"), Uri);
        if (DeviceCode.IsEmpty() || !Uri.StartsWith(TEXT("https://")))
        {
            Done(Fail(FInvoError::Make(TEXT("The phone approval grant was incomplete."), TEXT("INVALID_RESPONSE"))));
            return;
        }
        Done(Result);
    });
}

void UInvoSubsystem::PollPhoneApproval(const FString& DeviceCode, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!IsPresent(DeviceCode))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("deviceCode"), TEXT("<redacted>"), TEXT("is required"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("device_code"), DeviceCode);
    PlayerPost(TEXT("/api/sdk/approvals/device/poll"), Body, [Done](const FInvoResult& Result)
    {
        if (Result.bOk)
        {
            TSharedRef<FJsonObject> Object = Result.Object();
            FString Status;
            if (!Object->TryGetStringField(TEXT("status"), Status) || Status.IsEmpty())
            {
                Object->SetStringField(TEXT("status"), TEXT("approved"));
            }
            Done(FInvoResult::Success(Object));
            return;
        }
        // RFC 8628 outcomes arrive as 400 {"error": "<outcome>"}; they are answers, not failures.
        FString Outcome;
        if (Result.Error.Status == 400 && Result.Error.Body.JsonObject.IsValid()
            && Result.Error.Body.JsonObject->TryGetStringField(TEXT("error"), Outcome) && IsPollOutcome(Outcome))
        {
            TSharedRef<FJsonObject> Mapped = MakeShared<FJsonObject>();
            Mapped->Values = Result.Error.Body.JsonObject->Values;
            Mapped->SetStringField(TEXT("status"), Outcome);
            Done(FInvoResult::Success(Mapped));
            return;
        }
        Done(WithBareCode(Result));
    });
}

void UInvoSubsystem::ConfirmPhoneEnrollment(const FString& DeviceCode, bool bApprove, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!IsPresent(DeviceCode))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("deviceCode"), TEXT("<redacted>"), TEXT("is required"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("device_code"), DeviceCode);
    Body->SetStringField(TEXT("decision"), bApprove ? TEXT("approve") : TEXT("deny"));
    PlayerPost(TEXT("/api/sdk/approvals/device/confirm-enrollment"), Body, [Done](const FInvoResult& Result) { Done(WithBareCode(Result)); });
}

FString UInvoSubsystem::SettlePath(const FString& TransactionId, const FString& Flow)
{
    const FString Id = FGenericPlatformHttp::UrlEncode(TransactionId);
    if (Flow == TEXT("transfer")) { return FString::Printf(TEXT("/api/sdk/transfers/%s/approve"), *Id); }
    if (Flow == TEXT("send")) { return FString::Printf(TEXT("/api/sdk/send/%s/approve"), *Id); }
    if (Flow == TEXT("send_receipt")) { return FString::Printf(TEXT("/api/sdk/send/%s/confirm-receipt"), *Id); }
    return FString::Printf(TEXT("/api/sdk/transfers/%s/confirm-receipt"), *Id);
}

bool UInvoSubsystem::IsPastStep(const FString& Flow, const FString& CurrentStatus)
{
    const FString S = CurrentStatus.TrimStartAndEnd().ToLower();
    if (S.IsEmpty())
    {
        return false;
    }
    const bool bFinished = S == TEXT("completed") || S == TEXT("distributed");
    if (Flow == TEXT("transfer") || Flow == TEXT("send"))
    {
        return bFinished || S == TEXT("pending_claim") || S == TEXT("verified") || S == TEXT("pin_verified_awaiting_claim");
    }
    if (Flow == TEXT("send_receipt") || Flow == TEXT("transfer_receipt"))
    {
        return bFinished;
    }
    return false;
}

int32 UInvoSubsystem::NextPollInterval(int32 Current, const FString& Status, int32 Stated)
{
    static constexpr int32 DefaultInterval = 5;
    static constexpr int32 SlowDownIncrement = 5;
    if (Status.IsEmpty())
    {
        return Stated > 0 ? Stated : DefaultInterval;
    }
    if (Status == TEXT("slow_down"))
    {
        return FMath::Max(Current + SlowDownIncrement, Stated);
    }
    return Stated > 0 ? Stated : FMath::Max(1, Current);
}

void UInvoSubsystem::SettlePhoneApproval(const FString& TransactionId, const FString& Flow, const FString& DeviceCode, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    const FInvoError Problem = TxnFlowProblem(TransactionId, Flow);
    if (!Problem.Code.IsEmpty())
    {
        Done(Fail(Problem));
        return;
    }
    if (!IsPresent(DeviceCode))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("deviceCode"), TEXT("<redacted>"), TEXT("is required"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("device_code"), DeviceCode);
    PlayerPost(SettlePath(TransactionId, Flow), Body, [Done, TransactionId, Flow](const FInvoResult& Result)
    {
        if (Result.bOk)
        {
            Done(Result);
            return;
        }
        if (Result.Error.Code == TEXT("TRANSACTION_NOT_PENDING"))
        {
            FString Current;
            if (Result.Error.Body.JsonObject.IsValid())
            {
                Result.Error.Body.JsonObject->TryGetStringField(TEXT("current_status"), Current);
            }
            TSharedRef<FJsonObject> Mapped = MakeShared<FJsonObject>();
            Mapped->SetStringField(TEXT("status"), TEXT("not_pending"));
            Mapped->SetStringField(TEXT("transaction_id"), TransactionId);
            Mapped->SetStringField(TEXT("current_status"), Current);
            Mapped->SetBoolField(TEXT("already_settled"), IsPastStep(Flow, Current));
            Done(FInvoResult::Success(Mapped));
            return;
        }
        Done(WithBareCode(Result));
    });
}

// ---------------------------------------------------------------- store

void UInvoSubsystem::GetGameItems(const FInvoResultDelegate& OnDone)
{
    GetGameItemsNative(Wrap(OnDone));
}

void UInvoSubsystem::GetGameItemsNative(FInvoResultCallback Done)
{
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    // The sandbox route reads the key from the body too; in production your game server fills it in.
    Body->SetStringField(TEXT("game_secret"), Environment == EInvoEnvironment::Sandbox && !Http->HasTransportOverride() ? ResolveSandboxSecret() : FString());
    CommercePost(TEXT("/api/v1/game-items/list"), Body, true, MoveTemp(Done));
}

void UInvoSubsystem::OpenCheckout(const FString& UsdAmount, const FInvoResultDelegate& OnDone)
{
    OpenCheckoutNative(UsdAmount, Wrap(OnDone));
}

void UInvoSubsystem::OpenCheckoutNative(const FString& UsdAmount, FInvoResultCallback Done)
{
    if (!RequireActivePlayer(Done, false))
    {
        return;
    }
    if (!UInvoLibrary::IsValidMoney(UsdAmount, UInvoLibrary::MaxUsdAmount))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("usdAmount"), UsdAmount, TEXT("must be a plain decimal > 0 and <= 999.99 with at most 2 places"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("player_email"), PlayerEmail);
    SetMoneyField(Body, TEXT("usd_amount"), UsdAmount);

    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    FInvoResultCallback OnSession = [WeakThis, Done](const FInvoResult& Result)
    {
        if (!Result.bOk || !WeakThis.IsValid())
        {
            Done(Result);
            return;
        }
        WeakThis->OpenCheckoutUrl(Result, Done);
    };

    if (Environment == EInvoEnvironment::Production && !Http->HasTransportOverride())
    {
        FString Endpoint = GetDefault<UInvoSDKSettings>()->CheckoutSessionEndpoint;
        Endpoint.TrimStartAndEndInline();
        if (Endpoint.IsEmpty())
        {
            Done(Fail(FInvoError::Make(TEXT("Real-money purchases need a server endpoint: set Checkout Session Endpoint in Project Settings > Plugins > Invo SDK."), TEXT("NO_CHECKOUT_ENDPOINT"))));
            return;
        }
        Http->PostUrl(Endpoint, Body, PlayerAuth(), MoveTemp(OnSession));
        return;
    }
    CommercePost(TEXT("/api/checkout/sessions"), Body, false, MoveTemp(OnSession));
}

void UInvoSubsystem::OpenCheckoutUrl(const FInvoResult& Result, FInvoResultCallback Done)
{
    FString Url;
    Result.Object()->TryGetStringField(TEXT("checkout_url"), Url);
    // Scheme allowlist: a tampered response must never launch file:// or a custom-scheme handler.
    if (!Url.StartsWith(TEXT("https://")))
    {
        Done(Fail(FInvoError::Make(TEXT("Refusing to open a non-https checkout URL."), TEXT("UNSAFE_URL"))));
        return;
    }
    if (!Http->HasTransportOverride())
    {
        FString LaunchError;
        FPlatformProcess::LaunchURL(*Url, nullptr, &LaunchError);
        if (!LaunchError.IsEmpty())
        {
            UE_LOG(LogInvo, Warning, TEXT("[Invo] Could not open the checkout page: %s"), *LaunchError);
        }
    }
    OnCheckoutOpened.Broadcast(Url);
    Done(Result);
}

void UInvoSubsystem::PurchaseItem(const FString& ClientRequestId, const FString& ItemId, const FString& ItemName, int32 Quantity,
                                  const FString& UnitPrice, const FString& TotalPrice, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, false))
    {
        return;
    }
    const TCHAR* Labels[] = { TEXT("clientRequestId"), TEXT("itemId"), TEXT("itemName") };
    const FString* Values[] = { &ClientRequestId, &ItemId, &ItemName };
    for (int32 i = 0; i < UE_ARRAY_COUNT(Labels); ++i)
    {
        if (!IsPresent(*Values[i]))
        {
            Done(Fail(FInvoError::InvalidInput(Labels[i], *Values[i], TEXT("is required"))));
            return;
        }
    }
    if (Quantity < 1 || Quantity > 1000)
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("itemQuantity"), FString::FromInt(Quantity), TEXT("must be an integer 1..1000"))));
        return;
    }
    if (!UInvoLibrary::IsValidMoney(UnitPrice, UInvoLibrary::MaxItemPrice))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("unitPrice"), UnitPrice, TEXT("must be a plain decimal > 0 with at most 2 places"))));
        return;
    }
    if (!UInvoLibrary::IsValidMoney(TotalPrice, UInvoLibrary::MaxItemPrice))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("totalPrice"), TotalPrice, TEXT("must be a plain decimal > 0 with at most 2 places"))));
        return;
    }
    // Compare in whole cents so float error cannot decide the +/-0.01 boundary.
    const int64 Expected = FMath::FloorToInt64(FCString::Atod(*UnitPrice) * Quantity * 100.0 + 0.5);
    const int64 Total = FMath::FloorToInt64(FCString::Atod(*TotalPrice) * 100.0 + 0.5);
    if (FMath::Abs(Total - Expected) > 1)
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("totalPrice"), TotalPrice, FString::Printf(TEXT("must equal unitPrice x itemQuantity (%s x %d)"), *UnitPrice, Quantity))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("client_request_id"), ClientRequestId);
    Body->SetStringField(TEXT("player_email"), PlayerEmail);
    Body->SetStringField(TEXT("player_name"), PlayerName.IsEmpty() ? PlayerEmail : PlayerName);
    Body->SetStringField(TEXT("item_id"), ItemId);
    Body->SetStringField(TEXT("item_name"), ItemName);
    Body->SetNumberField(TEXT("item_quantity"), Quantity);
    SetMoneyField(Body, TEXT("unit_price"), UnitPrice);
    SetMoneyField(Body, TEXT("total_price"), TotalPrice);
    if (!PlayerPhone.IsEmpty())
    {
        Body->SetStringField(TEXT("player_phone"), PlayerPhone);
    }
    CommercePost(TEXT("/api/item-purchases/purchase-item"), Body, true, [Done](const FInvoResult& Result)
    {
        if (Result.bOk)
        {
            FString TxnId, OrderId;
            Result.Object()->TryGetStringField(TEXT("transaction_id"), TxnId);
            Result.Object()->TryGetStringField(TEXT("order_id"), OrderId);
            if (TxnId.IsEmpty() || OrderId.IsEmpty())
            {
                Done(Fail(FInvoError::Make(TEXT("INVO response was missing transaction_id or order_id."), TEXT("INVALID_RESPONSE"))));
                return;
            }
        }
        Done(Result);
    });
}

// ---------------------------------------------------------------- send / transfer / collect

static FInvoError MoveProblem(const FString& ClientRequestId, const FString& Phone, const FString& GameIdValue, const FString& Amount)
{
    if (!IsPresent(ClientRequestId))
    {
        return FInvoError::InvalidInput(TEXT("clientRequestId"), ClientRequestId, TEXT("is required"));
    }
    if (!UInvoLibrary::IsE164Phone(Phone))
    {
        return FInvoError::InvalidInput(TEXT("phone"), Phone, TEXT("must be an E.164 phone number, e.g. \"+15555550100\""));
    }
    if (!UInvoHostedApprovalLibrary::IsValidGameId(GameIdValue))
    {
        return FInvoError::InvalidInput(TEXT("gameId"), GameIdValue, TEXT("must be the numeric game id"));
    }
    if (!UInvoLibrary::IsValidMoney(Amount, UInvoLibrary::MaxItemPrice))
    {
        return FInvoError::InvalidInput(TEXT("amount"), Amount, TEXT("must be a plain decimal > 0 with at most 2 places"));
    }
    return FInvoError();
}

void UInvoSubsystem::InitiateSend(const FString& ClientRequestId, const FString& ReceiverPhone, const FString& ReceivingGameId,
                                  const FString& Amount, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, true))
    {
        return;
    }
    const FInvoError Problem = MoveProblem(ClientRequestId, ReceiverPhone, ReceivingGameId, Amount);
    if (!Problem.Code.IsEmpty())
    {
        Done(Fail(Problem));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("client_request_id"), ClientRequestId);
    Body->SetStringField(TEXT("sender_player_name"), PlayerName.IsEmpty() ? PlayerEmail : PlayerName);
    Body->SetStringField(TEXT("sender_player_email"), PlayerEmail);
    Body->SetStringField(TEXT("sender_player_phone"), PlayerPhone);
    Body->SetStringField(TEXT("receiver_player_phone"), ReceiverPhone);
    SetGameIdField(Body, TEXT("receiving_game_id"), ReceivingGameId);
    SetMoneyField(Body, TEXT("amount"), Amount);
    CommercePost(TEXT("/api/currency-sends/initiate-send"), Body, true, MoveTemp(Done));
}

void UInvoSubsystem::InitiateTransfer(const FString& ClientRequestId, const FString& TargetGameId, const FString& Amount,
                                      const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, true))
    {
        return;
    }
    const FInvoError Problem = MoveProblem(ClientRequestId, PlayerPhone, TargetGameId, Amount);
    if (!Problem.Code.IsEmpty())
    {
        Done(Fail(Problem));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("client_request_id"), ClientRequestId);
    Body->SetStringField(TEXT("source_player_name"), PlayerName.IsEmpty() ? PlayerEmail : PlayerName);
    Body->SetStringField(TEXT("source_player_email"), PlayerEmail);
    Body->SetStringField(TEXT("source_player_phone"), PlayerPhone);
    Body->SetStringField(TEXT("target_player_phone"), PlayerPhone);
    SetGameIdField(Body, TEXT("target_game_id"), TargetGameId);
    SetMoneyField(Body, TEXT("amount"), Amount);
    CommercePost(TEXT("/api/transfers/initiate-transfer"), Body, true, MoveTemp(Done));
}

void UInvoSubsystem::ClaimCurrency(const FString& ClaimCode, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, true))
    {
        return;
    }
    if (!IsPresent(ClaimCode))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("claimCode"), ClaimCode, TEXT("is required"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("claim_code"), ClaimCode.TrimStartAndEnd());
    Body->SetStringField(TEXT("receiver_player_name"), PlayerName.IsEmpty() ? PlayerEmail : PlayerName);
    Body->SetStringField(TEXT("receiver_player_email"), PlayerEmail);
    Body->SetStringField(TEXT("receiver_player_phone"), PlayerPhone);
    CommercePost(TEXT("/api/currency-sends/claim-currency"), Body, false, MoveTemp(Done));
}

void UInvoSubsystem::ClaimTransfer(const FString& ClaimCode, const FString& TargetCurrencyId, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, true))
    {
        return;
    }
    if (!IsPresent(ClaimCode) || !IsPresent(TargetCurrencyId))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("claimCode/targetCurrencyId"), ClaimCode, TEXT("are required"))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("claim_code"), ClaimCode.TrimStartAndEnd());
    Body->SetStringField(TEXT("target_player_name"), PlayerName.IsEmpty() ? PlayerEmail : PlayerName);
    Body->SetStringField(TEXT("target_player_email"), PlayerEmail);
    Body->SetStringField(TEXT("target_player_phone"), PlayerPhone);
    Body->SetStringField(TEXT("target_currency_id"), TargetCurrencyId);
    CommercePost(TEXT("/api/transfers/claim-transfer"), Body, false, MoveTemp(Done));
}

void UInvoSubsystem::GetSendStatus(const FString& TransactionId, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!IsPresent(TransactionId))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("transactionId"), TransactionId, TEXT("is required"))));
        return;
    }
    CommerceGet(FString::Printf(TEXT("/api/currency-sends/%s/status"), *FGenericPlatformHttp::UrlEncode(TransactionId)), MoveTemp(Done));
}

void UInvoSubsystem::GetTransferStatus(const FString& TransactionId, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!IsPresent(TransactionId))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("transactionId"), TransactionId, TEXT("is required"))));
        return;
    }
    CommerceGet(FString::Printf(TEXT("/api/transfers/%s/status"), *FGenericPlatformHttp::UrlEncode(TransactionId)), MoveTemp(Done));
}

// ---------------------------------------------------------------- phone

void UInvoSubsystem::ChangePlayerPhone(const FString& NewPhone, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, false))
    {
        return;
    }
    if (!UInvoLibrary::IsE164Phone(NewPhone))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("newPhone"), NewPhone, TEXT("must be an E.164 phone number, e.g. \"+15555550100\""))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("player_email"), PlayerEmail);
    Body->SetStringField(TEXT("new_phone"), NewPhone);
    TWeakObjectPtr<UInvoSubsystem> WeakThis(this);
    CommercePost(TEXT("/api/sdk/players/phone"), Body, false, [WeakThis, NewPhone, Done](const FInvoResult& Result)
    {
        if (Result.bOk && WeakThis.IsValid())
        {
            WeakThis->PlayerPhone = NewPhone;
        }
        Done(WithBareCode(Result));
    });
}

void UInvoSubsystem::CreatePhoneChangeRequest(const FString& NewPhone, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!RequireActivePlayer(Done, false))
    {
        return;
    }
    if (!UInvoLibrary::IsE164Phone(NewPhone))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("newPhone"), NewPhone, TEXT("must be an E.164 phone number, e.g. \"+15555550100\""))));
        return;
    }
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("player_email"), PlayerEmail);
    Body->SetStringField(TEXT("new_phone"), NewPhone);
    CommercePost(TEXT("/api/sdk/players/phone/change-requests"), Body, false, [Done](const FInvoResult& Result)
    {
        if (Result.bOk)
        {
            FString Id;
            if (!Result.Object()->TryGetStringField(TEXT("change_request_id"), Id) || Id.IsEmpty())
            {
                Done(Fail(FInvoError::Make(TEXT("INVO response was missing the required `change_request_id` field."), TEXT("INVALID_RESPONSE"))));
                return;
            }
        }
        Done(WithBareCode(Result));
    });
}

void UInvoSubsystem::GetPhoneChangeRequest(const FString& ChangeRequestId, const FInvoResultDelegate& OnDone)
{
    FInvoResultCallback Done = Wrap(OnDone);
    if (!IsPresent(ChangeRequestId))
    {
        Done(Fail(FInvoError::InvalidInput(TEXT("changeRequestId"), ChangeRequestId, TEXT("is required"))));
        return;
    }
    CommerceGet(FString::Printf(TEXT("/api/sdk/players/phone/change-requests/%s"), *FGenericPlatformHttp::UrlEncode(ChangeRequestId)), MoveTemp(Done));
}
