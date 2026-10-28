// Automation tests for the INVO core: validation, URL and key rules, error parsing, the HTTP
// retry rules (through a fake transport, no network) and the phone-approval helpers. Run from
// Session Frontend > Automation (filter "InvoSDK") or:
//   UnrealEditor-Cmd.exe <Project>.uproject -ExecCmds="Automation RunTests InvoSDK.Core;Quit" -unattended -nopause -NullRHI

#include "InvoLibrary.h"
#include "InvoHttp.h"
#include "InvoSubsystem.h"
#include "InvoTypes.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#define INVO_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

static TSharedPtr<FJsonObject> ParseJson(const FString& Text)
{
    TSharedPtr<FJsonObject> Object;
    TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Text);
    FJsonSerializer::Deserialize(Reader, Object);
    return Object;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoMoneyTest, "InvoSDK.Core.Money", INVO_TEST_FLAGS)
bool FInvoMoneyTest::RunTest(const FString& Parameters)
{
    for (const TCHAR* Good : { TEXT("5"), TEXT("4.99"), TEXT("0.5"), TEXT("999.99") })
    {
        TestTrue(FString::Printf(TEXT("accepts %s"), Good), UInvoLibrary::IsValidMoney(Good, UInvoLibrary::MaxUsdAmount));
    }
    for (const TCHAR* Bad : { TEXT(""), TEXT("0"), TEXT("0.00"), TEXT("-1"), TEXT("+1"), TEXT("1.999"), TEXT("1."), TEXT(".5"),
                             TEXT("1e2"), TEXT("0x10"), TEXT(" 5"), TEXT("5\n"), TEXT("1000"), TEXT("1.2.3"), TEXT("abc") })
    {
        TestFalse(FString::Printf(TEXT("rejects '%s'"), Bad), UInvoLibrary::IsValidMoney(Bad, UInvoLibrary::MaxUsdAmount));
    }
    TestTrue(TEXT("item max"), UInvoLibrary::IsValidMoney(TEXT("999999.99"), UInvoLibrary::MaxItemPrice));
    TestEqual(TEXT("format whole"), UInvoLibrary::FormatMoney(5.0), FString(TEXT("5")));
    TestEqual(TEXT("format cents"), UInvoLibrary::FormatMoney(4.99), FString(TEXT("4.99")));
    TestEqual(TEXT("format tenths"), UInvoLibrary::FormatMoney(2.5), FString(TEXT("2.5")));
    TestEqual(TEXT("format leading zero cents"), UInvoLibrary::FormatMoney(1.05), FString(TEXT("1.05")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoPhoneTest, "InvoSDK.Core.Phone", INVO_TEST_FLAGS)
bool FInvoPhoneTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("us"), UInvoLibrary::IsE164Phone(TEXT("+15555550100")));
    TestTrue(TEXT("uk"), UInvoLibrary::IsE164Phone(TEXT("+447700900123")));
    for (const TCHAR* Bad : { TEXT(""), TEXT("5555550100"), TEXT("+05555550100"), TEXT("+1 555 555 0100"), TEXT("+1555555010012345"), TEXT("+1555-555"), TEXT("+") })
    {
        TestFalse(FString::Printf(TEXT("rejects '%s'"), Bad), UInvoLibrary::IsE164Phone(Bad));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoUrlTest, "InvoSDK.Core.Urls", INVO_TEST_FLAGS)
bool FInvoUrlTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("https"), UInvoLibrary::SecureBaseUrlProblem(TEXT("https://invo.network")).IsEmpty());
    TestTrue(TEXT("https path"), UInvoLibrary::SecureBaseUrlProblem(TEXT("https://sandbox.invo.network/sandbox")).IsEmpty());
    TestTrue(TEXT("localhost"), UInvoLibrary::SecureBaseUrlProblem(TEXT("http://localhost:8080")).IsEmpty());
    TestTrue(TEXT("loopback"), UInvoLibrary::SecureBaseUrlProblem(TEXT("http://127.0.0.1:3000/api")).IsEmpty());
    TestTrue(TEXT("ipv6 loopback"), UInvoLibrary::SecureBaseUrlProblem(TEXT("http://[::1]:3000")).IsEmpty());
    TestFalse(TEXT("plain http"), UInvoLibrary::SecureBaseUrlProblem(TEXT("http://invo.network")).IsEmpty());
    TestFalse(TEXT("localhost lookalike"), UInvoLibrary::SecureBaseUrlProblem(TEXT("http://localhost.evil.com")).IsEmpty());
    TestFalse(TEXT("no scheme"), UInvoLibrary::SecureBaseUrlProblem(TEXT("invo.network")).IsEmpty());
    TestFalse(TEXT("file"), UInvoLibrary::SecureBaseUrlProblem(TEXT("file:///etc/passwd")).IsEmpty());

    TestEqual(TEXT("query stripped"), UInvoLibrary::RedactUrl(TEXT("https://x/api/a?email=p@x.com")), FString(TEXT("https://x/api/a")));
    TestEqual(TEXT("by-email redacted"), UInvoLibrary::RedactUrl(TEXT("https://x/api/player-balances/player/by-email/p%40x.com")),
              FString(TEXT("https://x/api/player-balances/player/by-email/<redacted>")));

    TestEqual(TEXT("header CRLF"), UInvoLibrary::SanitizeHeaderValue(TEXT("Bearer abc\r\nX-Evil: 1")), FString(TEXT("Bearer abcX-Evil: 1")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoKeyTest, "InvoSDK.Core.Keys", INVO_TEST_FLAGS)
bool FInvoKeyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("test key"), UInvoLibrary::KeyEnvironment(TEXT("ivsdk_test_abc")), FString(TEXT("Sandbox")));
    TestEqual(TEXT("live key"), UInvoLibrary::KeyEnvironment(TEXT("ivsdk_live_abc")), FString(TEXT("Production")));
    TestTrue(TEXT("unknown"), UInvoLibrary::KeyEnvironment(TEXT("sk_abc")).IsEmpty());
    TestTrue(TEXT("case-sensitive"), UInvoLibrary::KeyEnvironment(TEXT("IVSDK_TEST_abc")).IsEmpty());
    TestTrue(TEXT("request ids are unique"), UInvoLibrary::NewClientRequestId() != UInvoLibrary::NewClientRequestId());
    TestTrue(TEXT("request id prefix"), UInvoLibrary::NewClientRequestId().StartsWith(TEXT("unreal-")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoErrorTest, "InvoSDK.Core.Errors", INVO_TEST_FLAGS)
bool FInvoErrorTest::RunTest(const FString& Parameters)
{
    FInvoError Coded = FInvoError::FromResponse(400, ParseJson(TEXT("{\"error\":\"Not enough\",\"code\":\"INSUFFICIENT_BALANCE\"}")), FString(), TEXT("req-1"));
    TestEqual(TEXT("code"), Coded.Code, FString(TEXT("INSUFFICIENT_BALANCE")));
    TestEqual(TEXT("message"), Coded.Message, FString(TEXT("Not enough")));
    TestEqual(TEXT("request id"), Coded.RequestId, FString(TEXT("req-1")));
    TestTrue(TEXT("insufficient"), UInvoLibrary::IsInsufficientBalance(Coded));

    FInvoError Throttled = FInvoError::FromResponse(429, ParseJson(TEXT("{\"error\":\"insufficient balance attempts\",\"retry_after\":7}")), FString(), FString());
    TestFalse(TEXT("429 is not a balance problem"), UInvoLibrary::IsInsufficientBalance(Throttled));
    TestTrue(TEXT("rate limited"), UInvoLibrary::IsRateLimited(Throttled));
    TestEqual(TEXT("retry after"), UInvoLibrary::GetRetryAfterSeconds(Throttled), 7.f);

    FInvoError Promoted = FInvoError::FromResponse(409, ParseJson(TEXT("{\"error\":\"receiver_not_enrolled_use_claim_code\"}")), FString(), FString());
    TestEqual(TEXT("promoted token"), Promoted.Code, FString(TEXT("receiver_not_enrolled_use_claim_code")));
    TestTrue(TEXT("not enrolled"), UInvoLibrary::IsReceiverNotEnrolled(Promoted));

    FInvoError Bare = FInvoError::FromResponse(400, ParseJson(TEXT("{\"error\":\"invalid_grant\"}")), FString(), FString());
    TestTrue(TEXT("bare token not promoted by default"), Bare.Code.IsEmpty());
    TestEqual(TEXT("bare token promoted on request"), Bare.PromoteBareErrorToken().Code, FString(TEXT("invalid_grant")));
    FInvoError Sentence = FInvoError::FromResponse(400, ParseJson(TEXT("{\"error\":\"Something went wrong\"}")), FString(), FString());
    TestTrue(TEXT("sentence never promoted"), Sentence.PromoteBareErrorToken().Code.IsEmpty());

    FInvoError Other = FInvoError::FromResponse(401, ParseJson(TEXT("{\"error\":\"x\",\"code\":\"KEY_FOR_OTHER_ENVIRONMENT\"}")), FString(), FString());
    TestTrue(TEXT("other env"), UInvoLibrary::IsKeyForOtherEnvironment(Other));
    TestTrue(TEXT("other env message names the prefixes"), Other.Message.Contains(TEXT("ivsdk_test_")) && Other.Message.Contains(TEXT("ivsdk_live_")));

    FInvoError Text = FInvoError::FromResponse(502, nullptr, TEXT("Bad Gateway"), FString());
    TestEqual(TEXT("text body"), Text.Message, FString(TEXT("Bad Gateway")));
    FInvoError Empty = FInvoError::FromResponse(500, nullptr, FString(), FString());
    TestEqual(TEXT("empty body"), Empty.Message, FString(TEXT("INVO request failed (HTTP 500)")));

    TestEqual(TEXT("header retry-after"), UInvoLibrary::RetryAfterSeconds(nullptr, TEXT("3")), 3.0);
    TestEqual(TEXT("no retry-after"), UInvoLibrary::RetryAfterSeconds(nullptr, FString()), -1.0);
    TestEqual(TEXT("iso retry-after is not seconds"), UInvoLibrary::RetryAfterSeconds(ParseJson(TEXT("{\"retry_after\":\"2026-10-29T00:00:00Z\"}")), FString()), -1.0);
    return true;
}

/** A fake transport that answers each call from a queue and records what was sent. */
struct FFakeTransport
{
    TArray<FInvoRawResponse> Answers;
    TArray<FString> Calls;
    TArray<TMap<FString, FString>> Headers;

    FInvoTransport Make()
    {
        return [this](const FString& Verb, const FString& Url, const TMap<FString, FString>& InHeaders, const FString&, TFunction<void(const FInvoRawResponse&)> Done)
        {
            Calls.Add(Verb + TEXT(" ") + Url);
            Headers.Add(InHeaders);
            FInvoRawResponse Raw;
            if (Answers.Num() > 0)
            {
                Raw = Answers[0];
                Answers.RemoveAt(0);
            }
            else
            {
                Raw.bNetworkError = true;
            }
            Done(Raw);
        };
    }

    static FInvoRawResponse Status(int32 Code, const FString& Body)
    {
        FInvoRawResponse Raw;
        Raw.Status = Code;
        Raw.Body = Body;
        return Raw;
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoHttpTest, "InvoSDK.Core.Http", INVO_TEST_FLAGS)
bool FInvoHttpTest::RunTest(const FString& Parameters)
{
    TSharedRef<FInvoHttp> Http = MakeShared<FInvoHttp>();
    TestFalse(TEXT("refuses plain http"), Http->Configure(TEXT("http://invo.network")).IsEmpty());
    TestTrue(TEXT("accepts https"), Http->Configure(TEXT("https://sandbox.invo.network/sandbox/")).IsEmpty());
    TestEqual(TEXT("trailing slash trimmed"), Http->GetBaseUrl(), FString(TEXT("https://sandbox.invo.network/sandbox")));

    // Success parses the JSON body and sends the agent and auth headers (sanitised).
    {
        FFakeTransport Fake;
        Fake.Answers.Add(FFakeTransport::Status(200, TEXT("{\"balances\":[]}")));
        Http->SetTransport(Fake.Make());
        FInvoResult Got;
        TMap<FString, FString> Auth;
        Auth.Add(TEXT("Authorization"), TEXT("Bearer t\r\nX: 1"));
        Http->Get(TEXT("/api/sdk/balance"), Auth, [&Got](const FInvoResult& R) { Got = R; });
        TestTrue(TEXT("ok"), Got.bOk);
        TestTrue(TEXT("parsed"), Got.Object()->HasField(TEXT("balances")));
        TestEqual(TEXT("url"), Fake.Calls[0], FString(TEXT("GET https://sandbox.invo.network/sandbox/api/sdk/balance")));
        TestEqual(TEXT("auth sanitised"), Fake.Headers[0].FindRef(TEXT("Authorization")), FString(TEXT("Bearer tX: 1")));
        TestTrue(TEXT("user agent"), Fake.Headers[0].FindRef(TEXT("User-Agent")).StartsWith(TEXT("invonetwork-unreal-sdk/")));
    }

    // A non-idempotent POST is never retried, even on 503 or a network error.
    {
        FFakeTransport Fake;
        Fake.Answers.Add(FFakeTransport::Status(503, TEXT("{\"error\":\"down\"}")));
        Http->SetTransport(Fake.Make());
        FInvoResult Got;
        Http->Post(TEXT("/api/checkout/sessions"), MakeShared<FJsonObject>(), {}, false, [&Got](const FInvoResult& R) { Got = R; });
        TestFalse(TEXT("failed"), Got.bOk);
        TestEqual(TEXT("one call only"), Fake.Calls.Num(), 1);
        TestEqual(TEXT("status"), Got.Error.Status, 503);
    }

    // A 4xx is final even when idempotent.
    {
        FFakeTransport Fake;
        Fake.Answers.Add(FFakeTransport::Status(400, TEXT("{\"error\":\"bad\",\"code\":\"INVALID_INPUT\"}")));
        Http->SetTransport(Fake.Make());
        FInvoResult Got;
        Http->Get(TEXT("/api/x"), {}, [&Got](const FInvoResult& R) { Got = R; });
        TestEqual(TEXT("one call only"), Fake.Calls.Num(), 1);
        TestEqual(TEXT("code"), Got.Error.Code, FString(TEXT("INVALID_INPUT")));
    }

    // A 3xx is never treated as success.
    {
        FFakeTransport Fake;
        Fake.Answers.Add(FFakeTransport::Status(302, FString()));
        Http->SetTransport(Fake.Make());
        FInvoResult Got;
        Http->Get(TEXT("/api/x"), {}, [&Got](const FInvoResult& R) { Got = R; });
        TestFalse(TEXT("redirect is an error"), Got.bOk);
    }

    // Absolute URLs must be https too.
    {
        FInvoResult Got;
        Http->PostUrl(TEXT("http://example.com/token"), nullptr, {}, [&Got](const FInvoResult& R) { Got = R; });
        TestEqual(TEXT("refused"), Got.Error.Code, FString(TEXT("INVALID_INPUT")));
    }

    Http->SetTransport(nullptr);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvoApprovalHelpersTest, "InvoSDK.Core.ApprovalHelpers", INVO_TEST_FLAGS)
bool FInvoApprovalHelpersTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("transfer"), UInvoSubsystem::SettlePath(TEXT("t1"), TEXT("transfer")), FString(TEXT("/api/sdk/transfers/t1/approve")));
    TestEqual(TEXT("send"), UInvoSubsystem::SettlePath(TEXT("t1"), TEXT("send")), FString(TEXT("/api/sdk/send/t1/approve")));
    TestEqual(TEXT("send receipt"), UInvoSubsystem::SettlePath(TEXT("t1"), TEXT("send_receipt")), FString(TEXT("/api/sdk/send/t1/confirm-receipt")));
    TestEqual(TEXT("transfer receipt"), UInvoSubsystem::SettlePath(TEXT("t1"), TEXT("transfer_receipt")), FString(TEXT("/api/sdk/transfers/t1/confirm-receipt")));
    TestFalse(TEXT("id is encoded"), UInvoSubsystem::SettlePath(TEXT("a/../b"), TEXT("send")).Contains(TEXT("/../")));

    TestTrue(TEXT("send past pending_claim"), UInvoSubsystem::IsPastStep(TEXT("send"), TEXT("pending_claim")));
    TestTrue(TEXT("transfer completed"), UInvoSubsystem::IsPastStep(TEXT("transfer"), TEXT("Completed")));
    TestFalse(TEXT("receipt not past pending_claim"), UInvoSubsystem::IsPastStep(TEXT("send_receipt"), TEXT("pending_claim")));
    TestFalse(TEXT("unknown status fails closed"), UInvoSubsystem::IsPastStep(TEXT("send"), TEXT("mystery")));
    TestFalse(TEXT("empty status"), UInvoSubsystem::IsPastStep(TEXT("send"), FString()));

    TestEqual(TEXT("first wait default"), UInvoSubsystem::NextPollInterval(0, FString(), 0), 5);
    TestEqual(TEXT("first wait stated"), UInvoSubsystem::NextPollInterval(0, FString(), 3), 3);
    TestEqual(TEXT("slow down"), UInvoSubsystem::NextPollInterval(5, TEXT("slow_down"), 5), 10);
    TestEqual(TEXT("pending keeps stated"), UInvoSubsystem::NextPollInterval(10, TEXT("authorization_pending"), 5), 5);
    return true;
}

#undef INVO_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
