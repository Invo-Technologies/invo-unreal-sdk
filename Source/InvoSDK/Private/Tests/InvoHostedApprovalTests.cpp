// Automation tests for the hosted-approval helper. Run from Session Frontend > Automation
// (filter "InvoSDK") or:
//   UnrealEditor-Cmd.exe <Project>.uproject -ExecCmds="Automation RunTests InvoSDK.HostedApproval" -unattended -nopause -NullRHI
// The browser hand-off and the foreground return need a device; these cover the pure logic and
// the subsystem's state machine.

#include "InvoHostedApproval.h"
#include "InvoSDKSettings.h"
#include "Engine/GameInstance.h"
#include "UObject/Package.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// The flags are spelled out in each macro rather than held in a constant: before UE 5.5 they are
// plain enumerators (EAutomationTestFlags::Type, or'd to an int) and from 5.5 an enum class with
// flag operators. The bare expression compiles under both; a typed constant does not.
#define INVO_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoHostedApprovalSchemeTest, "InvoSDK.HostedApproval.ReturnScheme", INVO_TEST_FLAGS)
bool FInvoHostedApprovalSchemeTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("scheme"), UInvoHostedApprovalLibrary::MakeReturnScheme(TEXT("12345")), FString(TEXT("invo-sdk-12345")));
    TestEqual(TEXT("return url"), UInvoHostedApprovalLibrary::MakeReturnUrl(TEXT("12345")), FString(TEXT("invo-sdk-12345://done")));

    for (const TCHAR* Bad : { TEXT(""), TEXT("abc"), TEXT("12 34"), TEXT("12-34"), TEXT("1234567890123456789012345678901234567890123") })
    {
        TestTrue(FString::Printf(TEXT("rejects '%s'"), Bad), UInvoHostedApprovalLibrary::MakeReturnScheme(Bad).IsEmpty());
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoHostedApprovalUrlTest, "InvoSDK.HostedApproval.Urls", INVO_TEST_FLAGS)
bool FInvoHostedApprovalUrlTest::RunTest(const FString& Parameters)
{
    // Hosted URL: absolute https with a host, nothing else.
    TestTrue(TEXT("https"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("https://invo.network/device?code=ABCD-1234")));
    TestTrue(TEXT("sandbox"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("https://sandbox.invo.network/sandbox/device?code=ABCD")));
    TestFalse(TEXT("http"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("http://invo.network/device")));
    TestFalse(TEXT("javascript"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("javascript:alert(1)")));
    TestFalse(TEXT("custom scheme"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("invo-sdk-1://done")));
    TestFalse(TEXT("whitespace"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("https://invo.network/device?code=A B")));
    TestFalse(TEXT("no host"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("https:///device")));
    TestFalse(TEXT("relative"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("/device?code=ABCD")));
    TestFalse(TEXT("empty"), UInvoHostedApprovalLibrary::IsHostedApprovalUrl(TEXT("")));

    // Return URL: only the scheme is compared.
    TestTrue(TEXT("exact"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-42://done"), TEXT("42")));
    TestTrue(TEXT("case-insensitive"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("INVO-SDK-42://done"), TEXT("42")));
    TestTrue(TEXT("query ignored"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-42://done?status=approved"), TEXT("42")));
    TestTrue(TEXT("host ignored"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-42://approved"), TEXT("42")));
    TestFalse(TEXT("other game"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-43://done"), TEXT("42")));
    TestFalse(TEXT("prefix game"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-420://done"), TEXT("42")));
    TestFalse(TEXT("https"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("https://invo.network/done"), TEXT("42")));
    TestFalse(TEXT("no colon"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-42"), TEXT("42")));
    TestFalse(TEXT("bad game id"), UInvoHostedApprovalLibrary::IsReturnUrl(TEXT("invo-sdk-42://done"), TEXT("abc")));

    TestTrue(TEXT("any numeric"), UInvoHostedApprovalLibrary::IsAnyReturnUrl(TEXT("invo-sdk-7://done")));
    TestFalse(TEXT("any empty"), UInvoHostedApprovalLibrary::IsAnyReturnUrl(TEXT("invo-sdk-://done")));
    TestFalse(TEXT("any alpha"), UInvoHostedApprovalLibrary::IsAnyReturnUrl(TEXT("invo-sdk-x://done")));
    TestFalse(TEXT("any https"), UInvoHostedApprovalLibrary::IsAnyReturnUrl(TEXT("https://invo.network/")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoHostedApprovalCopyTest, "InvoSDK.HostedApproval.Copy", INVO_TEST_FLAGS)
bool FInvoHostedApprovalCopyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("prompt"),
        UInvoHostedApprovalLibrary::MakeEnrollmentPromptText(TEXT("iPhone (Safari)"), TEXT("4821")),
        FString(TEXT("Set up INVO on \"iPhone (Safari)\"? Code 4821. Say Yes only if the phone you just opened shows this code.")));
    TestEqual(TEXT("finishing"),
        UInvoHostedApprovalLibrary::MakeEnrollmentFinishingText(TEXT("iPhone (Safari)")),
        FString(TEXT("Finishing on \"iPhone (Safari)\"\u2026")));
    TestEqual(TEXT("yes"), UInvoHostedApprovalLibrary::YesLabel(), FString(TEXT("Yes")));
    TestEqual(TEXT("no"), UInvoHostedApprovalLibrary::NoLabel(), FString(TEXT("No")));
    TestEqual(TEXT("stop"), UInvoHostedApprovalLibrary::StopLabel(), FString(TEXT("Stop it (wrong code)")));
    TestEqual(TEXT("approve wire"), UInvoHostedApprovalLibrary::DecisionToWire(EInvoEnrollmentDecision::Approve), FString(TEXT("approve")));
    TestEqual(TEXT("deny wire"), UInvoHostedApprovalLibrary::DecisionToWire(EInvoEnrollmentDecision::Deny), FString(TEXT("deny")));

    TestEqual(TEXT("sanitize markup"), UInvoHostedApprovalLibrary::SanitizeForDisplay(TEXT("Pixel <b>b</b>\x01 8"), TEXT("x")), FString(TEXT("Pixel b 8")));
    TestEqual(TEXT("sanitize empty"), UInvoHostedApprovalLibrary::SanitizeForDisplay(TEXT("  <>  "), TEXT("this phone")), FString(TEXT("this phone")));
    TestEqual(TEXT("sanitize cap"), UInvoHostedApprovalLibrary::SanitizeForDisplay(FString::ChrN(200, TEXT('a')), TEXT("x")).Len(), 64);
    // Bidi override, zero-width space/joiner, C1 control, line separator: all dropped.
    TestEqual(TEXT("sanitize format chars"), UInvoHostedApprovalLibrary::SanitizeForDisplay(TEXT("a\u202Eb\u200Bc\u200D\u0085d\u2028"), TEXT("x")), FString(TEXT("abcd")));
    // The label is capped at 32 and quoted; a label that reads like a sentence stays inside its quotes.
    const FString LongLabel = FString::ChrN(40, TEXT('L'));
    const FString ExpectedLabel = FString(TEXT("\"")) + FString::ChrN(32, TEXT('L')) + TEXT("\"");
    TestTrue(TEXT("label cap+quote"), UInvoHostedApprovalLibrary::MakeEnrollmentPromptText(LongLabel, TEXT("1")).StartsWith(FString(TEXT("Set up INVO on ")) + ExpectedLabel + TEXT("? Code 1.")));
    TestEqual(TEXT("finishing cap"), UInvoHostedApprovalLibrary::MakeEnrollmentFinishingText(LongLabel), FString(TEXT("Finishing on ")) + ExpectedLabel + TEXT("\u2026"));
    TestTrue(TEXT("label quoted"), UInvoHostedApprovalLibrary::MakeEnrollmentPromptText(TEXT("Say Yes now"), TEXT("1")).Contains(TEXT("\"Say Yes now\"?")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoHostedApprovalJsonTest, "InvoSDK.HostedApproval.Json", INVO_TEST_FLAGS)
bool FInvoHostedApprovalJsonTest::RunTest(const FString& Parameters)
{
    FJsonObjectWrapper Poll;
    Poll.JsonObjectFromString(TEXT("{\"error\":\"authorization_pending\",\"interval\":5,\"enrollment\":{\"state\":\"awaiting_screen\",\"device_label\":\"Pixel 8\",\"match_code\":\"7310\",\"requested_at\":\"2026-09-03T10:00:00\"}}"));
    FInvoEnrollmentState State;
    TestTrue(TEXT("parses nested enrollment"), UInvoHostedApprovalLibrary::ParseEnrollmentState(Poll, State));
    TestEqual(TEXT("state"), State.State, FString(TEXT("awaiting_screen")));
    TestEqual(TEXT("label"), State.DeviceLabel, FString(TEXT("Pixel 8")));
    TestEqual(TEXT("code"), State.MatchCode, FString(TEXT("7310")));

    FJsonObjectWrapper NoBlock;
    NoBlock.JsonObjectFromString(TEXT("{\"error\":\"authorization_pending\",\"interval\":5}"));
    TestFalse(TEXT("absent block"), UInvoHostedApprovalLibrary::ParseEnrollmentState(NoBlock, State));
    TestTrue(TEXT("absent block clears"), State.State.IsEmpty());

    FJsonObjectWrapper Begin;
    Begin.JsonObjectFromString(TEXT("{\"verification_uri_complete\":\"https://invo.network/device?code=ABCD\",\"user_code\":\"ABCD-1234\",\"expires_in\":600,\"interval\":5,\"device_code\":\"should-not-be-here\"}"));
    FInvoHostedApprovalHandoff Handoff;
    TestTrue(TEXT("parses handoff"), UInvoHostedApprovalLibrary::ParseHostedApprovalHandoff(Begin, Handoff));
    TestEqual(TEXT("uri"), Handoff.VerificationUriComplete, FString(TEXT("https://invo.network/device?code=ABCD")));
    TestEqual(TEXT("user code"), Handoff.UserCode, FString(TEXT("ABCD-1234")));
    TestEqual(TEXT("expires"), Handoff.ExpiresIn, 600);
    TestEqual(TEXT("interval"), Handoff.Interval, 5);

    FJsonObjectWrapper BadBegin;
    BadBegin.JsonObjectFromString(TEXT("{\"verification_uri_complete\":\"http://invo.network/device\"}"));
    TestFalse(TEXT("refuses http handoff"), UInvoHostedApprovalLibrary::ParseHostedApprovalHandoff(BadBegin, Handoff));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoHostedApprovalSubsystemTest, "InvoSDK.HostedApproval.Subsystem", INVO_TEST_FLAGS)
bool FInvoHostedApprovalSubsystemTest::RunTest(const FString& Parameters)
{
    // Inside a bare game instance (subsystems must be outered to one) with no world or viewport,
    // so route the prompt through the custom-UI delegates and drive it with SubmitEnrollmentDecision.
    UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
    UInvoHostedApprovalSubsystem* Subsystem = NewObject<UInvoHostedApprovalSubsystem>(GameInstance);
    Subsystem->AddToRoot();
    Subsystem->bUseBuiltInPrompt = false;

    // Open refuses bad input and opens nothing.
    AddExpectedError(TEXT("Hosted approval URL must be an absolute https URL."), EAutomationExpectedErrorFlags::Contains, 1);
    TestFalse(TEXT("refuses http"), Subsystem->OpenHostedApprovalForGame(TEXT("http://invo.network/device"), TEXT("42")));
    AddExpectedError(TEXT("Hosted approval needs a numeric game id"), EAutomationExpectedErrorFlags::Contains, 1);
    TestFalse(TEXT("refuses bad game id"), Subsystem->OpenHostedApprovalForGame(TEXT("https://invo.network/device?code=X"), TEXT("not-a-game")));
    TestFalse(TEXT("not awaiting"), Subsystem->IsAwaitingReturn());

    // Deep link: only the return scheme wakes us. Built from the configured GameId (Project
    // Settings > Plugins > Invo SDK) so the test holds on a configured project; "99" otherwise.
    const UInvoSDKSettings* Settings = GetDefault<UInvoSDKSettings>();
    const bool bConfigured = Settings && UInvoHostedApprovalLibrary::IsValidGameId(Settings->GameId);
    const FString GameId = bConfigured ? Settings->GameId : FString(TEXT("99"));
    TestFalse(TEXT("https ignored"), Subsystem->HandleDeepLink(TEXT("https://invo.network/anything")));
    TestFalse(TEXT("other scheme ignored"), Subsystem->HandleDeepLink(TEXT("mygame://open")));
    TestFalse(TEXT("non-numeric invo scheme ignored"), Subsystem->HandleDeepLink(TEXT("invo-sdk-x://done")));
    TestTrue(TEXT("return scheme handled"), Subsystem->HandleDeepLink(UInvoHostedApprovalLibrary::MakeReturnUrl(GameId)));
    // Another game's scheme: refused once a GameId is configured. Unconfigured, any numeric
    // invo-sdk-<digits> scheme is accepted by design (cold start before the id is known).
    const FString OtherGameUrl = UInvoHostedApprovalLibrary::MakeReturnUrl(GameId + TEXT("0"));
    TestEqual(TEXT("another game's scheme"), Subsystem->HandleDeepLink(OtherGameUrl), !bConfigured);

    // Prompt state machine.
    TArray<EInvoEnrollmentDecision> Decisions;
    auto Record = [&Decisions](EInvoEnrollmentDecision D) { Decisions.Add(D); };

    FInvoEnrollmentState Awaiting;
    Awaiting.State = TEXT("awaiting_screen");
    Awaiting.DeviceLabel = TEXT("Pixel 8");
    Awaiting.MatchCode = TEXT("7310");

    TestTrue(TEXT("awaiting shows"), Subsystem->ApplyEnrollmentStateNative(Awaiting, Record));
    TestTrue(TEXT("prompt showing"), Subsystem->IsPromptShowing());
    TestFalse(TEXT("unchanged is no-op"), Subsystem->ApplyEnrollmentStateNative(Awaiting, Record));
    Subsystem->SubmitEnrollmentDecision(EInvoEnrollmentDecision::Approve);
    TestFalse(TEXT("prompt hidden after decision"), Subsystem->IsPromptShowing());
    TestEqual(TEXT("one decision"), Decisions.Num(), 1);
    if (Decisions.Num() == 1)
    {
        TestEqual(TEXT("approve forwarded"), static_cast<uint8>(Decisions[0]), static_cast<uint8>(EInvoEnrollmentDecision::Approve));
    }

    FInvoEnrollmentState Confirmed = Awaiting;
    Confirmed.State = TEXT("confirmed");
    TestTrue(TEXT("confirmed shows finishing"), Subsystem->ApplyEnrollmentStateNative(Confirmed, Record));
    Subsystem->SubmitEnrollmentDecision(EInvoEnrollmentDecision::Approve); // finishing mode: Submit always delivers Deny
    TestEqual(TEXT("two decisions"), Decisions.Num(), 2);
    if (Decisions.Num() == 2)
    {
        TestEqual(TEXT("stop means deny"), static_cast<uint8>(Decisions[1]), static_cast<uint8>(EInvoEnrollmentDecision::Deny));
    }

    FInvoEnrollmentState Denied;
    Denied.State = TEXT("denied");
    Subsystem->ApplyEnrollmentStateNative(Awaiting, Record);
    TestTrue(TEXT("denied hides"), Subsystem->ApplyEnrollmentStateNative(Denied, Record));
    TestFalse(TEXT("hidden after denied"), Subsystem->IsPromptShowing());

    Subsystem->ApplyEnrollmentStateNative(Awaiting, Record);
    TestTrue(TEXT("absent after prompt hides"), Subsystem->ApplyEnrollmentStateNative(FInvoEnrollmentState(), Record));
    TestFalse(TEXT("absent with nothing shown is no-op"), Subsystem->ApplyEnrollmentStateNative(FInvoEnrollmentState(), Record));

    Subsystem->RemoveFromRoot();
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
