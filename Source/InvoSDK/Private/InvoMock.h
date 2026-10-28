#pragma once

#include "CoreMinimal.h"
#include "InvoHttp.h"

/**
 * Offline mock transport: canned sandbox-shaped responses so the wallet demo and UI work run with
 * no network and no credentials (the Unreal twin of the Godot SDK's InvoMock). Refused in Shipping
 * builds by UInvoSubsystem::SetTransportForTesting.
 *
 * Stateful where a screen needs a sequence:
 *  - Device approval: begin -> first poll authorization_pending -> second poll approved -> settle
 *    (a second settle answers TRANSACTION_NOT_PENDING). Collecting a pending item removes it from
 *    /api/sdk/transfers/pending and credits the balance.
 *  - Phone change: with bPhoneSecured the direct change answers 409 PHONE_CHANGE_REQUIRES_VERIFICATION;
 *    a change request then reads pending once and confirmed on the next poll.
 *  - Item purchases spend the balance and answer INSUFFICIENT_BALANCE when it runs out.
 */
class FInvoMock
{
public:
    static constexpr const TCHAR* Phone = TEXT("+15555550111");
    static constexpr const TCHAR* Email = TEXT("player@example.com");
    static constexpr const TCHAR* DisplayName = TEXT("Demo Player");

    /** true: the direct phone change asks for the player's confirmation (a secured account). */
    static bool bPhoneSecured;
    /** true: a grant's pending poll carries a first-time-phone `enrollment` block. */
    static bool bEnrollmentPrompt;
    /** Simulated latency, so loading states show. */
    static float LatencySeconds;

    static void Reset();
    static FInvoTransport Transport();
    static FString CurrentPhone();

    /** One request, answered synchronously (Transport() defers it by LatencySeconds). */
    static FInvoRawResponse Handle(const FString& Verb, const FString& Url, const FString& Body);
};
