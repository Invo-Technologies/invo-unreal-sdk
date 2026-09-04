#include "InvoHostedApproval.h"
#include "InvoSDKSettings.h"
#include "SInvoEnrollmentPrompt.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CoreDelegates.h"

DEFINE_LOG_CATEGORY_STATIC(LogInvoHostedApproval, Log, All);

namespace
{
    const TCHAR* const SchemePrefix = TEXT("invo-sdk-");
    const TCHAR* const ReturnHost = TEXT("done");
    constexpr int32 MaxGameIdLength = 40;
    constexpr int32 MaxDisplayLength = 64;
    constexpr int32 MaxLabelLength = 32;

    // C0/C1 controls, DEL, line/paragraph separators, the bidi embedding/override range
    // (U+202A-U+202E) and the zero-width format characters. Compared as code units, which is
    // what FString holds.
    bool IsControlChar(TCHAR C)
    {
        const uint32 U = static_cast<uint32>(C);
        return U < 32 || U == 127 || (U >= 0x80 && U <= 0x9F) || U == 0x2028 || U == 0x2029
            || (U >= 0x202A && U <= 0x202E) || U == 0x200B || U == 0x200C || U == 0x200D
            || U == 0x2060 || U == 0xFEFF;
    }
}

// ======================================================================
// Library
// ======================================================================

bool UInvoHostedApprovalLibrary::IsValidGameId(const FString& GameId)
{
    if (GameId.IsEmpty() || GameId.Len() > MaxGameIdLength)
    {
        return false;
    }
    for (TCHAR C : GameId)
    {
        if (C < TEXT('0') || C > TEXT('9'))
        {
            return false;
        }
    }
    return true;
}

FString UInvoHostedApprovalLibrary::MakeReturnScheme(const FString& GameId)
{
    return IsValidGameId(GameId) ? FString(SchemePrefix) + GameId : FString();
}

FString UInvoHostedApprovalLibrary::MakeReturnUrl(const FString& GameId)
{
    const FString Scheme = MakeReturnScheme(GameId);
    return Scheme.IsEmpty() ? FString() : Scheme + TEXT("://") + ReturnHost;
}

FString UInvoHostedApprovalLibrary::ExtractScheme(const FString& Url)
{
    int32 Colon = INDEX_NONE;
    if (!Url.FindChar(TEXT(':'), Colon) || Colon <= 0)
    {
        return FString();
    }
    const FString Scheme = Url.Left(Colon);
    for (TCHAR C : Scheme)
    {
        const bool bOk = (C >= TEXT('a') && C <= TEXT('z')) || (C >= TEXT('A') && C <= TEXT('Z'))
                      || (C >= TEXT('0') && C <= TEXT('9')) || C == TEXT('-') || C == TEXT('+') || C == TEXT('.');
        if (!bOk)
        {
            return FString();
        }
    }
    return Scheme;
}

bool UInvoHostedApprovalLibrary::IsHostedApprovalUrl(const FString& Url)
{
    if (Url.IsEmpty())
    {
        return false;
    }
    for (TCHAR C : Url)
    {
        if (FChar::IsWhitespace(C) || IsControlChar(C))
        {
            return false;
        }
    }
    const FString Scheme = ExtractScheme(Url);
    if (!Scheme.Equals(TEXT("https"), ESearchCase::IgnoreCase))
    {
        return false;
    }
    // "https://" then a non-empty host before the next '/', '?' or '#'.
    const FString Rest = Url.Mid(Scheme.Len());
    if (!Rest.StartsWith(TEXT("://")))
    {
        return false;
    }
    const FString Authority = Rest.Mid(3);
    int32 End = Authority.Len();
    for (int32 i = 0; i < Authority.Len(); ++i)
    {
        const TCHAR C = Authority[i];
        if (C == TEXT('/') || C == TEXT('?') || C == TEXT('#'))
        {
            End = i;
            break;
        }
    }
    return End > 0;
}

bool UInvoHostedApprovalLibrary::IsReturnUrl(const FString& Url, const FString& GameId)
{
    const FString Expected = MakeReturnScheme(GameId);
    if (Expected.IsEmpty())
    {
        return false;
    }
    const FString Scheme = ExtractScheme(Url);
    return !Scheme.IsEmpty() && Scheme.Equals(Expected, ESearchCase::IgnoreCase);
}

bool UInvoHostedApprovalLibrary::IsAnyReturnUrl(const FString& Url)
{
    const FString Scheme = ExtractScheme(Url);
    const int32 PrefixLen = FCString::Strlen(SchemePrefix);
    if (Scheme.Len() <= PrefixLen || !Scheme.StartsWith(SchemePrefix, ESearchCase::IgnoreCase))
    {
        return false;
    }
    return IsValidGameId(Scheme.Mid(PrefixLen));
}

FString UInvoHostedApprovalLibrary::SanitizeForDisplay(const FString& Value, const FString& Fallback)
{
    return SanitizeForDisplayCapped(Value, Fallback, MaxDisplayLength);
}

FString UInvoHostedApprovalLibrary::QuotedLabel(const FString& DeviceLabel)
{
    // Capped tighter than other values and quoted, so a label that reads like a sentence
    // cannot rewrite the prompt.
    return FString(TEXT("\"")) + SanitizeForDisplayCapped(DeviceLabel, TEXT("this phone"), MaxLabelLength) + TEXT("\"");
}

FString UInvoHostedApprovalLibrary::SanitizeForDisplayCapped(const FString& Value, const FString& Fallback, int32 MaxLength)
{
    if (Value.IsEmpty())
    {
        return Fallback;
    }
    // Whole <...> spans are removed (Slate/UMG rich text would otherwise honour tags; an
    // unclosed '<' drops the rest), control characters are dropped, and the value is capped.
    FString Cleaned;
    Cleaned.Reserve(Value.Len());
    bool bInTag = false;
    for (TCHAR C : Value)
    {
        if (Cleaned.Len() >= MaxLength)
        {
            break;
        }
        if (bInTag)
        {
            if (C == TEXT('>'))
            {
                bInTag = false;
            }
            continue;
        }
        if (C == TEXT('<'))
        {
            bInTag = true;
            continue;
        }
        if (IsControlChar(C) || C == TEXT('>'))
        {
            continue;
        }
        Cleaned.AppendChar(C);
    }
    Cleaned.TrimStartAndEndInline();
    return Cleaned.IsEmpty() ? Fallback : Cleaned;
}

FString UInvoHostedApprovalLibrary::MakeEnrollmentPromptText(const FString& DeviceLabel, const FString& MatchCode)
{
    return FString::Printf(TEXT("Set up INVO on %s? Code %s. Say Yes only if the phone you just opened shows this code."),
                           *QuotedLabel(DeviceLabel),
                           *SanitizeForDisplay(MatchCode, TEXT("unknown")));
}

FString UInvoHostedApprovalLibrary::MakeEnrollmentFinishingText(const FString& DeviceLabel)
{
    // \u2026 is the ellipsis; escaped so the source carries no non-ASCII literal.
    return FString::Printf(TEXT("Finishing on %s\u2026"), *QuotedLabel(DeviceLabel));
}

FString UInvoHostedApprovalLibrary::DecisionToWire(EInvoEnrollmentDecision Decision)
{
    return Decision == EInvoEnrollmentDecision::Approve ? TEXT("approve") : TEXT("deny");
}

FString UInvoHostedApprovalLibrary::YesLabel()  { return TEXT("Yes"); }
FString UInvoHostedApprovalLibrary::NoLabel()   { return TEXT("No"); }
FString UInvoHostedApprovalLibrary::StopLabel() { return TEXT("Stop it (wrong code)"); }

bool UInvoHostedApprovalLibrary::ParseEnrollmentState(const FJsonObjectWrapper& Json, FInvoEnrollmentState& OutState)
{
    OutState = FInvoEnrollmentState();
    TSharedPtr<FJsonObject> Obj = Json.JsonObject;
    if (!Obj.IsValid())
    {
        return false;
    }
    const TSharedPtr<FJsonObject>* Nested = nullptr;
    if (Obj->TryGetObjectField(TEXT("enrollment"), Nested) && Nested != nullptr && Nested->IsValid())
    {
        Obj = *Nested;
    }
    if (!Obj->TryGetStringField(TEXT("state"), OutState.State) || OutState.State.IsEmpty())
    {
        return false;
    }
    Obj->TryGetStringField(TEXT("device_label"), OutState.DeviceLabel);
    Obj->TryGetStringField(TEXT("match_code"), OutState.MatchCode);
    Obj->TryGetStringField(TEXT("requested_at"), OutState.RequestedAt);
    return true;
}

bool UInvoHostedApprovalLibrary::ParseHostedApprovalHandoff(const FJsonObjectWrapper& Json, FInvoHostedApprovalHandoff& OutHandoff)
{
    OutHandoff = FInvoHostedApprovalHandoff();
    const TSharedPtr<FJsonObject> Obj = Json.JsonObject;
    if (!Obj.IsValid())
    {
        return false;
    }
    if (!Obj->TryGetStringField(TEXT("verification_uri_complete"), OutHandoff.VerificationUriComplete))
    {
        return false;
    }
    Obj->TryGetStringField(TEXT("user_code"), OutHandoff.UserCode);
    double Number = 0.0;
    if (Obj->TryGetNumberField(TEXT("expires_in"), Number))
    {
        OutHandoff.ExpiresIn = static_cast<int32>(Number);
    }
    if (Obj->TryGetNumberField(TEXT("interval"), Number))
    {
        OutHandoff.Interval = static_cast<int32>(Number);
    }
    // device_code, if a server relays it by mistake, is ignored on purpose.
    return IsHostedApprovalUrl(OutHandoff.VerificationUriComplete);
}

// ======================================================================
// Subsystem
// ======================================================================

void UInvoHostedApprovalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // The custom-scheme return brings the game back to the foreground on iOS and Android. That
    // is the whole signal: the URL carries nothing, so "we are back" is "poll now".
    ForegroundHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(
        this, &UInvoHostedApprovalSubsystem::OnApplicationEnteredForeground);
}

void UInvoHostedApprovalSubsystem::Deinitialize()
{
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(ForegroundHandle);
    ForegroundHandle.Reset();
    PendingDecision = nullptr;
    RemovePromptWidget();
    Super::Deinitialize();
}

FString UInvoHostedApprovalSubsystem::ResolveConfiguredGameId()
{
    const UInvoSDKSettings* Settings = GetDefault<UInvoSDKSettings>();
    return Settings ? Settings->GameId : FString();
}

bool UInvoHostedApprovalSubsystem::OpenHostedApproval(const FString& Url)
{
    return OpenHostedApprovalForGame(Url, ResolveConfiguredGameId());
}

bool UInvoHostedApprovalSubsystem::OpenHostedApprovalForGame(const FString& Url, const FString& GameId)
{
    if (!UInvoHostedApprovalLibrary::IsHostedApprovalUrl(Url))
    {
        UE_LOG(LogInvoHostedApproval, Error, TEXT("Hosted approval URL must be an absolute https URL."));
        return false;
    }
    if (!UInvoHostedApprovalLibrary::IsValidGameId(GameId))
    {
        UE_LOG(LogInvoHostedApproval, Error, TEXT("Hosted approval needs a numeric game id (Project Settings > Plugins > Invo SDK) to derive the return scheme."));
        return false;
    }

    ActiveGameId = GameId;
    bAwaitingReturn = true;

    // System browser, never an embedded WebView: WebAuthn does not run in a WebView, and the
    // hosted page refuses to be framed. On iOS this is Safari; on Android the default browser.
    // An ASWebAuthenticationSession / Custom Tabs bridge is a native step described in README.md.
    FString Error;
    FPlatformProcess::LaunchURL(*Url, nullptr, &Error);
    if (!Error.IsEmpty())
    {
        UE_LOG(LogInvoHostedApproval, Error, TEXT("LaunchURL failed: %s"), *Error);
        bAwaitingReturn = false;
        return false;
    }
    return true;
}

void UInvoHostedApprovalSubsystem::NotifyApprovalPageFinished()
{
    bAwaitingReturn = false;
    OnApprovalPageFinished.Broadcast();
}

bool UInvoHostedApprovalSubsystem::HandleDeepLink(const FString& Url)
{
    FString GameId = ActiveGameId;
    if (GameId.IsEmpty())
    {
        GameId = ResolveConfiguredGameId();
    }
    const bool bMatches = GameId.IsEmpty()
        ? UInvoHostedApprovalLibrary::IsAnyReturnUrl(Url)
        : UInvoHostedApprovalLibrary::IsReturnUrl(Url, GameId);
    if (!bMatches)
    {
        return false;
    }
    NotifyApprovalPageFinished();
    return true;
}

void UInvoHostedApprovalSubsystem::OnApplicationEnteredForeground()
{
    if (!bAwaitingReturn)
    {
        return;
    }
    if (IsInGameThread())
    {
        NotifyApprovalPageFinished();
        return;
    }
    TWeakObjectPtr<UInvoHostedApprovalSubsystem> WeakThis(this);
    AsyncTask(ENamedThreads::GameThread, [WeakThis]()
    {
        if (WeakThis.IsValid() && WeakThis->bAwaitingReturn)
        {
            WeakThis->NotifyApprovalPageFinished();
        }
    });
}

// ---------------- prompt ----------------

void UInvoHostedApprovalSubsystem::ShowEnrollmentPrompt(const FString& DeviceLabel, const FString& MatchCode, FInvoEnrollmentDecisionDelegate OnDecision)
{
    ShowEnrollmentPromptNative(DeviceLabel, MatchCode, [OnDecision](EInvoEnrollmentDecision Decision)
    {
        OnDecision.ExecuteIfBound(Decision);
    });
}

void UInvoHostedApprovalSubsystem::ShowEnrollmentFinishing(const FString& DeviceLabel, FInvoEnrollmentDecisionDelegate OnStop)
{
    ShowEnrollmentFinishingNative(DeviceLabel, [OnStop](EInvoEnrollmentDecision Decision)
    {
        OnStop.ExecuteIfBound(Decision);
    });
}

bool UInvoHostedApprovalSubsystem::ApplyEnrollmentState(const FInvoEnrollmentState& Enrollment, FInvoEnrollmentDecisionDelegate OnDecision)
{
    return ApplyEnrollmentStateNative(Enrollment, [OnDecision](EInvoEnrollmentDecision Decision)
    {
        OnDecision.ExecuteIfBound(Decision);
    });
}

void UInvoHostedApprovalSubsystem::ShowEnrollmentPromptNative(const FString& DeviceLabel, const FString& MatchCode, TFunction<void(EInvoEnrollmentDecision)> OnDecision)
{
    PendingDecision = MoveTemp(OnDecision);
    bPromptShowing = true;
    bFinishingMode = false;
    PresentPrompt(UInvoHostedApprovalLibrary::MakeEnrollmentPromptText(DeviceLabel, MatchCode), false);
}

void UInvoHostedApprovalSubsystem::ShowEnrollmentFinishingNative(const FString& DeviceLabel, TFunction<void(EInvoEnrollmentDecision)> OnStop)
{
    PendingDecision = MoveTemp(OnStop);
    bPromptShowing = true;
    bFinishingMode = true;
    PresentPrompt(UInvoHostedApprovalLibrary::MakeEnrollmentFinishingText(DeviceLabel), true);
}

void UInvoHostedApprovalSubsystem::HideEnrollmentPrompt()
{
    LastAppliedEnrollmentKey.Empty();
    PendingDecision = nullptr;
    HideView();
}

bool UInvoHostedApprovalSubsystem::ApplyEnrollmentStateNative(const FInvoEnrollmentState& Enrollment, TFunction<void(EInvoEnrollmentDecision)> OnDecision)
{
    if (Enrollment.State.IsEmpty())
    {
        const bool bWasShowing = bPromptShowing;
        HideEnrollmentPrompt();
        return bWasShowing;
    }

    const FString Key = Enrollment.State + TEXT("|") + Enrollment.MatchCode + TEXT("|") + Enrollment.DeviceLabel;
    if (Key == LastAppliedEnrollmentKey)
    {
        return false;
    }
    LastAppliedEnrollmentKey = Key;

    if (Enrollment.State == TEXT("awaiting_screen"))
    {
        ShowEnrollmentPromptNative(Enrollment.DeviceLabel, Enrollment.MatchCode, MoveTemp(OnDecision));
        return true;
    }
    if (Enrollment.State == TEXT("confirmed"))
    {
        // Stop means deny; the server lets a deny override a pending confirm.
        ShowEnrollmentFinishingNative(Enrollment.DeviceLabel, [OnDecision](EInvoEnrollmentDecision)
        {
            if (OnDecision)
            {
                OnDecision(EInvoEnrollmentDecision::Deny);
            }
        });
        return true;
    }
    // denied, or a state this SDK does not know: nothing to ask.
    PendingDecision = nullptr;
    HideView();
    return true;
}

void UInvoHostedApprovalSubsystem::SubmitEnrollmentDecision(EInvoEnrollmentDecision Decision)
{
    // The only answer the "Finishing on..." state can give is Stop, and Stop means Deny;
    // a custom widget that sends Approve here by mistake must not confirm anything.
    if (bFinishingMode)
    {
        Decision = EInvoEnrollmentDecision::Deny;
    }
    TFunction<void(EInvoEnrollmentDecision)> Callback = MoveTemp(PendingDecision);
    PendingDecision = nullptr;
    HideView();
    if (Callback)
    {
        Callback(Decision);
    }
}

void UInvoHostedApprovalSubsystem::PresentPrompt(const FString& Message, bool bFinishing)
{
    if (!bUseBuiltInPrompt)
    {
        bRoutedToDelegate = true;
        OnEnrollmentPromptRequested.Broadcast(Message, bFinishing);
        return;
    }

    RemovePromptWidget();
    bRoutedToDelegate = false;

    UGameInstance* GameInstance = GetGameInstance();
    UGameViewportClient* Viewport = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
    if (!Viewport)
    {
        UE_LOG(LogInvoHostedApproval, Warning, TEXT("No game viewport; the enrollment prompt was not drawn. Bind OnEnrollmentPromptRequested instead."));
        bRoutedToDelegate = true;
        OnEnrollmentPromptRequested.Broadcast(Message, bFinishing);
        return;
    }

    TWeakObjectPtr<UInvoHostedApprovalSubsystem> WeakThis(this);
    PromptWidget = SNew(SInvoEnrollmentPrompt)
        .Message(FText::FromString(Message))
        .PrimaryLabel(bFinishing ? FText::GetEmpty() : FText::FromString(UInvoHostedApprovalLibrary::YesLabel()))
        .SecondaryLabel(FText::FromString(bFinishing ? UInvoHostedApprovalLibrary::StopLabel() : UInvoHostedApprovalLibrary::NoLabel()))
        .OnPrimary(FSimpleDelegate::CreateLambda([WeakThis]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->SubmitEnrollmentDecision(EInvoEnrollmentDecision::Approve);
            }
        }))
        .OnSecondary(FSimpleDelegate::CreateLambda([WeakThis]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->SubmitEnrollmentDecision(EInvoEnrollmentDecision::Deny);
            }
        }));

    Viewport->AddViewportWidgetContent(PromptWidget.ToSharedRef(), 10000);
}

void UInvoHostedApprovalSubsystem::HideView()
{
    const bool bWasShowing = bPromptShowing;
    bPromptShowing = false;
    bFinishingMode = false;
    RemovePromptWidget();
    // Whoever drew the prompt is told to hide it: the route is remembered per presentation,
    // so flipping bUseBuiltInPrompt mid-flight (or the no-viewport fallback) cannot strand a
    // custom widget.
    if (bRoutedToDelegate && bWasShowing)
    {
        OnEnrollmentPromptHidden.Broadcast();
    }
    bRoutedToDelegate = false;
}

void UInvoHostedApprovalSubsystem::RemovePromptWidget()
{
    if (!PromptWidget.IsValid())
    {
        return;
    }
    UGameInstance* GameInstance = GetGameInstance();
    UGameViewportClient* Viewport = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
    if (Viewport)
    {
        Viewport->RemoveViewportWidgetContent(PromptWidget.ToSharedRef());
    }
    PromptWidget.Reset();
}
