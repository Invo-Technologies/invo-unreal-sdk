#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectWrapper.h"
#include "Templates/Function.h"
#include "InvoHostedApproval.generated.h"

class SInvoEnrollmentPrompt;

/**
 * The two answers the game screen can give to "Set up INVO on <label>?". The game forwards the
 * answer to ITS OWN server, which posts
 * `POST /api/sdk/approvals/device/confirm-enrollment {device_code, decision}`.
 * The SDK never holds `device_code`; it stays on the game server.
 */
UENUM(BlueprintType)
enum class EInvoEnrollmentDecision : uint8
{
    Approve UMETA(DisplayName = "Approve"),
    Deny    UMETA(DisplayName = "Deny")
};

/**
 * What the game SERVER hands the client after it called
 * `POST /api/sdk/approvals/device/begin {transaction_id, flow, "channel": "app_browser"}`.
 * The RFC 8628 `device_code` is deliberately absent: it stays on the game server, which is the
 * only party that polls and the only party that can settle money with it.
 */
USTRUCT(BlueprintType)
struct INVOSDK_API FInvoHostedApprovalHandoff
{
    GENERATED_BODY()

    /** The hosted page on INVO's platform domain. Open with OpenHostedApproval; never a WebView. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    FString VerificationUriComplete;

    /** Human-readable code for the "type it instead" fallback. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    FString UserCode;

    /** Seconds until the grant expires (10 minutes today). */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    int32 ExpiresIn = 0;

    /** Minimum seconds between server polls. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    int32 Interval = 0;
};

/**
 * The `enrollment` block the server's poll carries while a first-time phone waits for the game
 * screen to confirm the match code. Absent entirely for an already-enrolled phone. The game
 * server relays it to the client; feed it to ApplyEnrollmentState.
 */
USTRUCT(BlueprintType)
struct INVOSDK_API FInvoEnrollmentState
{
    GENERATED_BODY()

    /** "awaiting_screen" | "confirmed" | "denied". Empty means no block. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    FString State;

    /** The phone as the server describes it, e.g. "iPhone (Safari)". */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    FString DeviceLabel;

    /** The code the phone page is showing. The player confirms a MATCH. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    FString MatchCode;

    /** ISO-8601 timestamp of the enrolment request. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    FString RequestedAt;
};

/** Wake-up only: the hosted page finished, or the player came back. Poll your server now. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvoApprovalPageFinished);

/** The prompt's answer. Forward DecisionToWire(Decision) to your server. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FInvoEnrollmentDecisionDelegate, EInvoEnrollmentDecision, Decision);

/** Fired instead of the built-in Slate prompt when bUseBuiltInPrompt is false. Render Message
 *  with Yes/No (bFinishing == false) or the single Stop button (bFinishing == true), then call
 *  SubmitEnrollmentDecision. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInvoEnrollmentPromptRequested, const FString&, Message, bool, bFinishing);

/** Hide your own prompt (only used when bUseBuiltInPrompt is false). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvoEnrollmentPromptHidden);

/**
 * Pure helpers behind the hosted-approval flow: scheme derivation, URL checks, the prompt copy
 * and JSON parsing. No state, so they are covered by the automation tests in
 * Private/Tests/InvoHostedApprovalTests.cpp.
 */
UCLASS()
class INVOSDK_API UInvoHostedApprovalLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Game ids are numeric on the platform. Anything else cannot form a scheme. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static bool IsValidGameId(const FString& GameId);

    /** `invo-sdk-<GameId>`, or empty when the game id is invalid. Derived from the game id and
     *  nothing else; the server derives the same value and it is never caller-supplied. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString MakeReturnScheme(const FString& GameId);

    /** `invo-sdk-<GameId>://done` - the only URL the page ever returns on. It carries NOTHING:
     *  approved, denied and expired all use this same bare URL; the truth is the server's poll. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString MakeReturnUrl(const FString& GameId);

    /** True only for an absolute https URL with a host and no whitespace. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static bool IsHostedApprovalUrl(const FString& Url);

    /** True when Url uses this game's return scheme. Only the scheme is compared (case-insensitively);
     *  host, path, query and fragment are never read, so nothing smuggled onto the URL can tell
     *  the game anything. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static bool IsReturnUrl(const FString& Url, const FString& GameId);

    /** True when Url uses ANY `invo-sdk-<digits>` scheme (cold start, game id not yet known). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static bool IsAnyReturnUrl(const FString& Url);

    /** The scheme before the first colon, or empty. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString ExtractScheme(const FString& Url);

    /** "Set up INVO on "<label>"? Code <code>. Say Yes only if the phone you just opened shows this code." */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString MakeEnrollmentPromptText(const FString& DeviceLabel, const FString& MatchCode);

    /** "Finishing on "<label>"..." (with a real ellipsis) */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString MakeEnrollmentFinishingText(const FString& DeviceLabel);

    /** Drops control/format characters and whole <...> spans, caps at 64 characters, falls back when empty. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString SanitizeForDisplay(const FString& Value, const FString& Fallback);

    /** Same, with a caller-chosen cap. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString SanitizeForDisplayCapped(const FString& Value, const FString& Fallback, int32 MaxLength);

    /** The device label as it appears in the prompt copy: capped at 32 characters and quoted. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString QuotedLabel(const FString& DeviceLabel);

    /** "approve" | "deny" - the wire value for confirm-enrollment. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString DecisionToWire(EInvoEnrollmentDecision Decision);

    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString YesLabel();

    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString NoLabel();

    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static FString StopLabel();

    /** Reads `enrollment` from a relayed poll body (or a bare enrollment object). False when absent. */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static bool ParseEnrollmentState(const FJsonObjectWrapper& Json, FInvoEnrollmentState& OutState);

    /** Reads the begin response your server relays (device_code is ignored even if present). */
    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    static bool ParseHostedApprovalHandoff(const FJsonObjectWrapper& Json, FInvoHostedApprovalHandoff& OutHandoff);
};

/**
 * Hosted approval on mobile: open INVO's hosted approval page in the SYSTEM browser and learn
 * when it finishes, then show the match-code prompt for a first-time phone.
 *
 * Two halves. The game SERVER calls `begin` with channel "app_browser", keeps `device_code` and
 * polls; it hands the client only FInvoHostedApprovalHandoff. The client half is this subsystem:
 *  1. OpenHostedApproval opens `VerificationUriComplete` with FPlatformProcess::LaunchURL - the
 *     system browser. NEVER an embedded WebView (WebAuthn does not run there, so the passkey
 *     ceremony would fail before it started).
 *  2. When the page is done it navigates to `invo-sdk-<GameId>://done`, which brings the game
 *     back to the foreground. That URL carries NOTHING, so OnApprovalPageFinished means exactly
 *     "poll now", never "approved". Games with their own deep-link plumbing can also call
 *     HandleDeepLink / NotifyApprovalPageFinished.
 *  3. While a first-time phone waits, feed the poll's `enrollment` block to ApplyEnrollmentState
 *     and forward the decision to your server.
 * Money moves only when the game server calls the approve endpoint with `device_code`.
 */
UCLASS()
class INVOSDK_API UInvoHostedApprovalSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Wake-up only. Poll your server. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK|HostedApproval")
    FInvoApprovalPageFinished OnApprovalPageFinished;

    /** Only when bUseBuiltInPrompt is false: render the prompt yourself. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK|HostedApproval")
    FInvoEnrollmentPromptRequested OnEnrollmentPromptRequested;

    /** Only when bUseBuiltInPrompt is false: hide your prompt. */
    UPROPERTY(BlueprintAssignable, Category = "InvoSDK|HostedApproval")
    FInvoEnrollmentPromptHidden OnEnrollmentPromptHidden;

    /** True: a Slate overlay draws the prompt on the game viewport (no UMG asset needed).
     *  False: OnEnrollmentPromptRequested / OnEnrollmentPromptHidden fire and you call
     *  SubmitEnrollmentDecision from your own widget. */
    UPROPERTY(BlueprintReadWrite, Category = "InvoSDK|HostedApproval")
    bool bUseBuiltInPrompt = true;

    // ---------------- open ----------------

    /** Opens Url for the GameId in Project Settings > Plugins > Invo SDK. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    bool OpenHostedApproval(const FString& Url);

    /** Opens Url (must be https) in the system browser. Returns false, and opens nothing, when
     *  the URL is not https or GameId is not numeric. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    bool OpenHostedApprovalForGame(const FString& Url, const FString& GameId);

    /** For games with their own deep-link plumbing. Fires OnApprovalPageFinished. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    void NotifyApprovalPageFinished();

    /** Handles a deep link: a matching `invo-sdk-<GameId>` scheme fires OnApprovalPageFinished and
     *  returns true; anything else is ignored. Nothing beyond the scheme is read. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    bool HandleDeepLink(const FString& Url);

    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    bool IsAwaitingReturn() const { return bAwaitingReturn; }

    UFUNCTION(BlueprintPure, Category = "InvoSDK|HostedApproval")
    FString GetActiveGameId() const { return ActiveGameId; }

    // ---------------- match-code prompt ----------------

    /** Yes / No prompt with the canonical copy. OnDecision fires once. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    void ShowEnrollmentPrompt(const FString& DeviceLabel, const FString& MatchCode, FInvoEnrollmentDecisionDelegate OnDecision);

    /** "Finishing on <label>…" with one "Stop it (wrong code)" button. Stop delivers Deny: the
     *  server lets a deny override a pending confirm. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    void ShowEnrollmentFinishing(const FString& DeviceLabel, FInvoEnrollmentDecisionDelegate OnStop);

    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    void HideEnrollmentPrompt();

    /** Drive the prompt straight from the poll: pass the relayed `enrollment` block every tick
     *  (an empty State means absent). Unchanged blocks are a no-op. Returns true when the prompt
     *  changed. OnDecision receives Approve/Deny from the prompt and Deny from Stop. */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    bool ApplyEnrollmentState(const FInvoEnrollmentState& Enrollment, FInvoEnrollmentDecisionDelegate OnDecision);

    /** Delivers the pending prompt's answer (the built-in prompt calls this; so does your widget). */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|HostedApproval")
    void SubmitEnrollmentDecision(EInvoEnrollmentDecision Decision);

    // Native equivalents for C++ callers.
    void ShowEnrollmentPromptNative(const FString& DeviceLabel, const FString& MatchCode, TFunction<void(EInvoEnrollmentDecision)> OnDecision);
    void ShowEnrollmentFinishingNative(const FString& DeviceLabel, TFunction<void(EInvoEnrollmentDecision)> OnStop);
    bool ApplyEnrollmentStateNative(const FInvoEnrollmentState& Enrollment, TFunction<void(EInvoEnrollmentDecision)> OnDecision);

    bool IsPromptShowing() const { return bPromptShowing; }

private:
    bool bAwaitingReturn = false;
    bool bPromptShowing = false;
    bool bFinishingMode = false;
    bool bRoutedToDelegate = false;
    FString ActiveGameId;
    FString LastAppliedEnrollmentKey;
    TFunction<void(EInvoEnrollmentDecision)> PendingDecision;
    FDelegateHandle ForegroundHandle;
    TSharedPtr<SInvoEnrollmentPrompt> PromptWidget;

    void OnApplicationEnteredForeground();
    void PresentPrompt(const FString& Message, bool bFinishing);
    void HideView();
    void RemovePromptWidget();
    static FString ResolveConfiguredGameId();
};
