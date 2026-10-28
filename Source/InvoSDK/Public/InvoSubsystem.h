#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InvoTypes.h"
#include "InvoSubsystem.generated.h"

class FInvoHttp;
class FJsonObject;
struct FInvoRawResponse;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvoLoggedIn, const FString&, IdentityId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvoBalanceUpdated, const FInvoResult&, Balance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvoCheckoutOpened, const FString&, Url);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvoRequestFailed, const FInvoError&, Error);

/**
 * The INVO Network API for Unreal: Get it with GetGameInstance()->GetSubsystem<UInvoSubsystem>()
 * or the "Invo Subsystem" Blueprint node. Configured from Project Settings > Plugins > Invo SDK.
 *
 * Two kinds of call, routed the same way as INVO's Unity and Godot SDKs:
 *  - Player calls (balance, destinations, pending collect, phone approval) use a short-lived
 *    player token. Your backend mints it; call LoginViaBackend or SetPlayerToken. Safe to ship.
 *  - Commerce calls (store items, checkout, item purchase, send, transfer, claim, phone change)
 *    need the Platform API key. In the sandbox (Development builds only) the SDK sends them
 *    straight to INVO with your ivsdk_test_ key from Project Settings > Invo SDK (Local) or
 *    INVO_SANDBOX_SECRET. In production they go to YOUR game server (Game Server URL) with the
 *    player's token, and your server attaches the ivsdk_live_ key. A production key never ships
 *    in a build, and Shipping builds refuse direct mode.
 *
 * Every call answers once on its delegate with an FInvoResult. Money calls are never retried
 * automatically; a failure there is ambiguous about the money, so read the status before telling
 * the player it failed.
 */
UCLASS()
class INVOSDK_API UInvoSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---------------- events ----------------

    /** A player token is set and player calls are ready. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK")
    FInvoLoggedIn OnLoggedIn;

    /** Re-broadcast after GetBalance succeeds. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK")
    FInvoBalanceUpdated OnBalanceUpdated;

    /** A hosted checkout page was opened in the system browser. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK")
    FInvoCheckoutOpened OnCheckoutOpened;

    /** Any call failed. Hook global error UI or logging here. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK")
    FInvoRequestFailed OnRequestFailed;

    // ---------------- configuration ----------------

    /** Re-configure at runtime (the settings are applied on start). An invalid BaseUrlOverride
     *  leaves the previous configuration intact and returns false. The player token survives. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Config")
    bool Setup(EInvoEnvironment InEnvironment, const FString& BaseUrlOverride = TEXT(""));

    UFUNCTION(BlueprintPure, Category = "InvoSDK|Config")
    EInvoEnvironment GetEnvironment() const { return Environment; }

    UFUNCTION(BlueprintPure, Category = "InvoSDK|Config")
    FString GetGameId() const { return GameId; }

    /** "https://sandbox.invo.network/sandbox" or "https://invo.network". */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Config")
    static FString DefaultBaseUrl(EInvoEnvironment InEnvironment);

    /** True when commerce calls can go out now: a sandbox key in a Development build, or a Game Server URL in production. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Config")
    bool IsCommerceReady() const;

    /** Why commerce calls cannot go out, or empty when they can. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Config")
    FString CommerceProblem() const;

    // ---------------- player ----------------

    /** Hand the SDK a player token your backend minted. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player")
    void SetPlayerToken(const FString& Token, const FString& IdentityId = TEXT(""));

    UFUNCTION(BlueprintPure, Category = "InvoSDK|Player")
    bool HasPlayerToken() const { return !PlayerToken.IsEmpty(); }

    /** Who the wallet acts for. Your game knows the signed-in player; set them once after login. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player")
    void SetActivePlayer(const FString& Email, const FString& Phone, const FString& DisplayName);

    UFUNCTION(BlueprintPure, Category = "InvoSDK|Player")
    void GetActivePlayer(FString& Email, FString& Phone, FString& DisplayName) const { Email = PlayerEmail; Phone = PlayerPhone; DisplayName = PlayerName; }

    /** POSTs PayloadJson (whatever your backend needs) to the Player Token Endpoint, which answers
     *  {"token", "identity_id"?}. On success the SDK is logged in and re-mints through the same
     *  endpoint once when a token expires. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player", meta = (AutoCreateRefTerm = "OnDone"))
    void LoginViaBackend(const FString& PayloadJson, const FInvoResultDelegate& OnDone);

    /** Sandbox only: mint a player token for an existing sandbox player and log in as them. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player", meta = (AutoCreateRefTerm = "OnDone"))
    void LoginSandbox(const FString& Email, const FString& Phone, const FString& DisplayName, const FInvoResultDelegate& OnDone);

    /** The player's balances for this game. An empty `balances` array is a new player (0.00), not an error. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player", meta = (AutoCreateRefTerm = "OnDone"))
    void GetBalance(const FInvoResultDelegate& OnDone);

    /** Games this player can send or transfer to. Direction is "send" or "transfer". */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player", meta = (AutoCreateRefTerm = "OnDone"))
    void GetDestinations(const FString& Direction, const FInvoResultDelegate& OnDone);

    /** Currency waiting for the player (`pending` rows; `kind` is identity_gate or receiving_confirm). */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Player", meta = (AutoCreateRefTerm = "OnDone"))
    void GetPendingCollect(const FInvoResultDelegate& OnDone);

    // ---------------- phone approval (device grant) ----------------

    /** Starts phone approval for a send or transfer. Flow: transfer | send | send_receipt | transfer_receipt.
     *  Channel: "qr" (show VerificationUriComplete as a QR code) or "app_browser" (phones). Never retried. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Approval", meta = (AutoCreateRefTerm = "OnDone"))
    void BeginPhoneApproval(const FString& TransactionId, const FString& Flow, const FString& Channel, const FInvoResultDelegate& OnDone);

    /** One poll. Data.status is approved | authorization_pending | slow_down | access_denied | expired_token. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Approval", meta = (AutoCreateRefTerm = "OnDone"))
    void PollPhoneApproval(const FString& DeviceCode, const FInvoResultDelegate& OnDone);

    /** Answers "Set up INVO on this phone? Code ####" for a first-time phone. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Approval", meta = (AutoCreateRefTerm = "OnDone"))
    void ConfirmPhoneEnrollment(const FString& DeviceCode, bool bApprove, const FInvoResultDelegate& OnDone);

    /** THE MONEY STEP, after a poll answered approved. Single use, never retried. On an error read
     *  the transaction status before telling the player anything. TRANSACTION_NOT_PENDING comes back
     *  as success with status "not_pending" and already_settled. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Approval", meta = (AutoCreateRefTerm = "OnDone"))
    void SettlePhoneApproval(const FString& TransactionId, const FString& Flow, const FString& DeviceCode, const FInvoResultDelegate& OnDone);

    /** The settle endpoint for a flow (no secrets in it). */
    static FString SettlePath(const FString& TransactionId, const FString& Flow);

    /** Whether current_status proves this flow's step already happened. Fails closed. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Approval")
    static bool IsPastStep(const FString& Flow, const FString& CurrentStatus);

    /** RFC 8628 wait before the next poll. Status is the last poll's ("" right after begin). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Approval")
    static int32 NextPollInterval(int32 Current, const FString& Status, int32 Stated);

    /** "app_browser" on phones, "qr" everywhere else. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|Approval")
    static FString DefaultApprovalChannel();

    // ---------------- store ----------------

    /** The game's item catalog from the INVO console. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Store", meta = (AutoCreateRefTerm = "OnDone"))
    void GetGameItems(const FInvoResultDelegate& OnDone);

    /** Creates a hosted checkout for UsdAmount ("4.99") and opens it in the system browser. In
     *  production the session comes from your Checkout Session Endpoint. Only https URLs are opened. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Store", meta = (AutoCreateRefTerm = "OnDone"))
    void OpenCheckout(const FString& UsdAmount, const FInvoResultDelegate& OnDone);

    /** Spends the active player's balance on an item. Prices are decimal strings; TotalPrice must
     *  equal UnitPrice x Quantity. ClientRequestId comes from NewClientRequestId and is the
     *  idempotency key: reuse it when retrying the same purchase. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Store", meta = (AutoCreateRefTerm = "OnDone"))
    void PurchaseItem(const FString& ClientRequestId, const FString& ItemId, const FString& ItemName, int32 Quantity,
                      const FString& UnitPrice, const FString& TotalPrice, const FInvoResultDelegate& OnDone);

    // ---------------- send / transfer / collect ----------------

    /** Sends currency to another player's phone (this game or another). Approve it next with BeginPhoneApproval(flow "send"). */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Move", meta = (AutoCreateRefTerm = "OnDone"))
    void InitiateSend(const FString& ClientRequestId, const FString& ReceiverPhone, const FString& ReceivingGameId,
                      const FString& Amount, const FInvoResultDelegate& OnDone);

    /** Moves the active player's own balance to another INVO game. Approve it next with flow "transfer". */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Move", meta = (AutoCreateRefTerm = "OnDone"))
    void InitiateTransfer(const FString& ClientRequestId, const FString& TargetGameId, const FString& Amount,
                          const FInvoResultDelegate& OnDone);

    /** Claims a send with its claim code (for a receiver without phone approval). */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Move", meta = (AutoCreateRefTerm = "OnDone"))
    void ClaimCurrency(const FString& ClaimCode, const FInvoResultDelegate& OnDone);

    /** Claims a transfer with its claim code into TargetCurrencyId. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Move", meta = (AutoCreateRefTerm = "OnDone"))
    void ClaimTransfer(const FString& ClaimCode, const FString& TargetCurrencyId, const FInvoResultDelegate& OnDone);

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Move", meta = (AutoCreateRefTerm = "OnDone"))
    void GetSendStatus(const FString& TransactionId, const FInvoResultDelegate& OnDone);

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Move", meta = (AutoCreateRefTerm = "OnDone"))
    void GetTransferStatus(const FString& TransactionId, const FInvoResultDelegate& OnDone);

    // ---------------- phone ----------------

    /** Changes the phone that approves sends. A secured account answers PHONE_CHANGE_REQUIRES_VERIFICATION:
     *  call CreatePhoneChangeRequest instead, which confirms by email. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Phone", meta = (AutoCreateRefTerm = "OnDone"))
    void ChangePlayerPhone(const FString& NewPhone, const FInvoResultDelegate& OnDone);

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Phone", meta = (AutoCreateRefTerm = "OnDone"))
    void CreatePhoneChangeRequest(const FString& NewPhone, const FInvoResultDelegate& OnDone);

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Phone", meta = (AutoCreateRefTerm = "OnDone"))
    void GetPhoneChangeRequest(const FString& ChangeRequestId, const FInvoResultDelegate& OnDone);

    // ---------------- native ----------------

    /** C++ entry points take FInvoResultCallback; the Blueprint ones forward to these. */
    void GetBalanceNative(FInvoResultCallback Done);
    void GetGameItemsNative(FInvoResultCallback Done);
    void OpenCheckoutNative(const FString& UsdAmount, FInvoResultCallback Done);

    /** Replaces the network for tests and the offline mock (Development builds only). */
    void SetTransportForTesting(TFunction<void(const FString&, const FString&, const TMap<FString, FString>&, const FString&, TFunction<void(const FInvoRawResponse&)>)> Transport);

    /** Direct sandbox key resolution, exposed for tests: INVO_SANDBOX_SECRET, then the local setting. */
    static FString ResolveSandboxSecret();

private:
    EInvoEnvironment Environment = EInvoEnvironment::Sandbox;
    FString GameId;
    FString PlayerToken;
    FString PlayerEmail;
    FString PlayerPhone;
    FString PlayerName;
    /** Body re-posted to the token endpoint when a token expires; empty when not logged in via backend. */
    FString TokenEndpoint;
    FString TokenPayloadJson;
    bool bRefreshing = false;
    TArray<TFunction<void(bool)>> RefreshWaiters;

    TSharedPtr<FInvoHttp> Http;
    TSharedPtr<FInvoHttp> GameServerHttp;

    TMap<FString, FString> PlayerAuth() const;

    /** Player-token call with one re-mint + resend on SDK_TOKEN_EXPIRED (a token 401 is refused
     *  before the route runs, so that one resend is safe even for money calls). */
    void PlayerGet(const FString& Path, FInvoResultCallback Done);
    void PlayerPost(const FString& Path, TSharedPtr<FJsonObject> Body, FInvoResultCallback Done);
    void RefreshToken(TFunction<void(bool)> Done);

    /** Commerce routing: the sandbox key straight to INVO, or the game server with the player token. */
    bool CommerceRoute(TSharedPtr<FInvoHttp>& OutHttp, TMap<FString, FString>& OutAuth, FInvoError& OutError);
    void CommerceGet(const FString& Path, FInvoResultCallback Done);
    void CommercePost(const FString& Path, TSharedPtr<FJsonObject> Body, bool bIdempotent, FInvoResultCallback Done);

    /** Fires OnRequestFailed for failures, then the caller's delegate. */
    FInvoResultCallback Wrap(const FInvoResultDelegate& OnDone);
    FInvoResultCallback WrapNative(FInvoResultCallback Done);

    void OpenCheckoutUrl(const FInvoResult& Result, FInvoResultCallback Done);
    bool RequireActivePlayer(FInvoResultCallback& Done, bool bNeedPhone);
};
