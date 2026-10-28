#include "InvoMock.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

bool FInvoMock::bPhoneSecured = true;
bool FInvoMock::bEnrollmentPrompt = false;
float FInvoMock::LatencySeconds = 0.25f;

namespace
{
    constexpr int32 MockInterval = 2;

    struct FGrant
    {
        FString TransactionId;
        FString Flow;
        int32 Polls = 0;
        bool bDenied = false;
        bool bEnrolDecided = false;
        bool bSettled = false;
    };

    struct FPhoneRequest
    {
        FString Phone;
        int32 Polls = 0;
        FString Status;
    };

    struct FState
    {
        double Balance = 120.0;
        int32 Purchases = 0;
        int32 GrantSeq = 0;
        TMap<FString, FGrant> Grants;
        /** transaction id -> "approved" | "completed" after a settle. */
        TMap<FString, FString> Settled;
        TSet<FString> Collected;
        int32 PhoneSeq = 0;
        TMap<FString, FPhoneRequest> PhoneRequests;
        FString Phone = FInvoMock::Phone;
    };

    FState& State()
    {
        static FState S;
        return S;
    }

    using FObj = TSharedRef<FJsonObject>;

    FObj Obj() { return MakeShared<FJsonObject>(); }

    FString Money(double Value) { return FString::Printf(TEXT("%.2f"), Value); }

    FString ToJson(const FObj& O)
    {
        FString Out;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
        FJsonSerializer::Serialize(O, Writer);
        return Out;
    }

    FInvoRawResponse Respond(int32 Status, const FObj& Body)
    {
        FInvoRawResponse R;
        R.Status = Status;
        R.Headers.Add(TEXT("x-invo-request-id"), TEXT("mock-req"));
        R.Body = ToJson(Body);
        return R;
    }

    FInvoRawResponse Ok(const FObj& Body, int32 Status = 200) { return Respond(Status, Body); }

    FInvoRawResponse Err(int32 Status, const FString& Error, const FString& Code = FString())
    {
        FObj O = Obj();
        O->SetStringField(TEXT("error"), Error);
        if (!Code.IsEmpty())
        {
            O->SetStringField(TEXT("code"), Code);
        }
        return Respond(Status, O);
    }

    FObj ParseBody(const FString& Text)
    {
        TSharedPtr<FJsonObject> Parsed;
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Text);
        if (FJsonSerializer::Deserialize(Reader, Parsed) && Parsed.IsValid())
        {
            return Parsed.ToSharedRef();
        }
        return Obj();
    }

    FString Str(const FObj& O, const TCHAR* Field)
    {
        FString V;
        if (!O->TryGetStringField(Field, V))
        {
            double N = 0;
            if (O->TryGetNumberField(Field, N))
            {
                V = FString::SanitizeFloat(N);
            }
        }
        return V.TrimStartAndEnd();
    }

    // ---------------------------------------------------------------- data

    struct FPending
    {
        const TCHAR* Id;
        const TCHAR* Flow;
        const TCHAR* Amount;
        const TCHAR* Game;
        bool bHeld;
    };

    const FPending PendingRows[] = {
        { TEXT("mock-txn-9001"), TEXT("send"), TEXT("25.00"), TEXT("Starfall Arena"), false },
        { TEXT("mock-txn-9002"), TEXT("transfer"), TEXT("40.00"), TEXT("Harvest Lane"), false },
        { TEXT("mock-txn-9003"), TEXT("send"), TEXT("10.00"), TEXT("Starfall Arena"), true },
    };

    FObj BalanceBody()
    {
        const FString B = Money(State().Balance);
        FObj Row = Obj();
        Row->SetNumberField(TEXT("currency_id"), 1);
        Row->SetStringField(TEXT("currency_name"), TEXT("Gold Crowns"));
        Row->SetStringField(TEXT("currency_symbol"), TEXT("GC"));
        Row->SetStringField(TEXT("available_balance"), B);
        Row->SetStringField(TEXT("reserved_balance"), TEXT("0.00"));
        Row->SetStringField(TEXT("total_balance"), B);
        FObj O = Obj();
        O->SetStringField(TEXT("game_id"), TEXT("mock-game"));
        O->SetStringField(TEXT("game_name"), TEXT("Demo Game"));
        O->SetArrayField(TEXT("balances"), { MakeShared<FJsonValueObject>(Row) });
        return O;
    }

    FObj Destination(int32 GameId, const TCHAR* Name, const TCHAR* Currency, const TCHAR* Symbol, const TCHAR* Min, const TCHAR* Max)
    {
        FObj G = Obj();
        G->SetNumberField(TEXT("game_id"), GameId);
        G->SetStringField(TEXT("game_name"), Name);
        G->SetStringField(TEXT("currency_name"), Currency);
        G->SetStringField(TEXT("currency_symbol"), Symbol);
        G->SetStringField(TEXT("minimum_transfer"), Min);
        G->SetStringField(TEXT("maximum_transfer"), Max);
        return G;
    }

    FObj DestinationsBody(const FString& Direction)
    {
        FObj O = Obj();
        O->SetStringField(TEXT("status"), TEXT("ok"));
        O->SetStringField(TEXT("source_game_name"), TEXT("Demo Game"));
        O->SetStringField(TEXT("source_currency_name"), TEXT("Gold Crowns"));
        O->SetArrayField(TEXT("available_games"), {
            MakeShared<FJsonValueObject>(Destination(7, TEXT("Starfall Arena"), TEXT("Shards"), TEXT("SH"), TEXT("1.00"), TEXT("500.00"))),
            MakeShared<FJsonValueObject>(Destination(12, TEXT("Harvest Lane"), TEXT("Acorns"), TEXT("AC"), TEXT("5.00"), TEXT("250.00"))),
        });
        O->SetNumberField(TEXT("total_destinations"), 2);
        O->SetStringField(TEXT("direction"), Direction.IsEmpty() ? TEXT("transfer") : Direction);
        return O;
    }

    FObj PendingBody()
    {
        TArray<TSharedPtr<FJsonValue>> Rows;
        for (const FPending& P : PendingRows)
        {
            if (State().Collected.Contains(P.Id))
            {
                continue;
            }
            FObj R = Obj();
            R->SetStringField(TEXT("transfer_id"), P.Id);
            R->SetStringField(TEXT("transaction_id"), P.Id);
            R->SetStringField(TEXT("kind"), TEXT("receiving_confirm"));
            R->SetStringField(TEXT("flow"), P.Flow);
            R->SetStringField(TEXT("amount"), P.Amount);
            R->SetStringField(TEXT("currency"), TEXT("Gold Crowns"));
            R->SetStringField(TEXT("counterparty_game"), P.Game);
            R->SetStringField(TEXT("expires_at"), TEXT("2999-01-01T00:00:00Z"));
            R->SetBoolField(TEXT("step_up_required"), P.bHeld);
            R->SetBoolField(TEXT("held"), P.bHeld);
            if (P.bHeld)
            {
                R->SetStringField(TEXT("hold_reason"), TEXT("device_requires_elevation"));
            }
            Rows.Add(MakeShared<FJsonValueObject>(R));
        }
        FObj O = Obj();
        O->SetArrayField(TEXT("pending"), Rows);
        return O;
    }

    FString PendingAmount(const FString& TransactionId)
    {
        for (const FPending& P : PendingRows)
        {
            if (TransactionId == P.Id)
            {
                return P.Amount;
            }
        }
        return TEXT("25.00");
    }

    FObj Item(const TCHAR* Id, const TCHAR* Name, const TCHAR* Category, const TCHAR* Type, double Price, double Original,
              int32 Discount, double Amount, double Bonus, bool bFeatured, const TCHAR* Tag, const TCHAR* Getting)
    {
        FObj I = Obj();
        I->SetStringField(TEXT("item_id"), Id);
        I->SetStringField(TEXT("item_name"), Name);
        I->SetStringField(TEXT("display_name"), Name);
        I->SetStringField(TEXT("display_subtitle"), Getting);
        I->SetStringField(TEXT("display_tag"), Tag);
        I->SetStringField(TEXT("item_category"), Category);
        I->SetStringField(TEXT("item_type"), Type);
        I->SetStringField(TEXT("image_url"), TEXT(""));
        I->SetBoolField(TEXT("is_featured"), bFeatured);
        I->SetBoolField(TEXT("is_active"), true);
        I->SetNumberField(TEXT("price_usd"), Price);
        if (Original > 0)
        {
            I->SetNumberField(TEXT("original_price_usd"), Original);
            I->SetNumberField(TEXT("discount_percentage"), Discount);
        }
        else
        {
            I->SetField(TEXT("original_price_usd"), MakeShared<FJsonValueNull>());
            I->SetField(TEXT("discount_percentage"), MakeShared<FJsonValueNull>());
        }
        I->SetNumberField(TEXT("currency_amount"), Amount);
        I->SetNumberField(TEXT("bonus_amount"), Bonus);
        return I;
    }

    FObj GameItemsBody()
    {
        TArray<TSharedPtr<FJsonValue>> Items = {
            MakeShared<FJsonValueObject>(Item(TEXT("mock-bundle-hero"), TEXT("Dragon Hoard Bundle"), TEXT("weekly"), TEXT("bundle"), 49.99, 79.99, 38, 2000, 500, true, TEXT("BEST VALUE"), TEXT("2,000 + 500 bonus"))),
            MakeShared<FJsonValueObject>(Item(TEXT("mock-pack-weekly-1"), TEXT("Knight's Coffer"), TEXT("weekly"), TEXT("currency_pack"), 19.99, 0, 0, 750, 0, false, TEXT(""), TEXT("750"))),
            MakeShared<FJsonValueObject>(Item(TEXT("mock-pack-weekly-2"), TEXT("Ranger's Satchel"), TEXT("weekly"), TEXT("currency_pack"), 24.99, 29.99, 17, 1000, 100, false, TEXT("POPULAR"), TEXT("1,000 + 100 bonus"))),
            MakeShared<FJsonValueObject>(Item(TEXT("mock-daily-1"), TEXT("Daily Gem Drop"), TEXT("daily"), TEXT("daily_deal"), 4.99, 9.99, 50, 150, 0, false, TEXT("-50%"), TEXT("150"))),
            MakeShared<FJsonValueObject>(Item(TEXT("mock-daily-2"), TEXT("Morning Potion Pack"), TEXT("daily"), TEXT("daily_deal"), 2.49, 4.99, 50, 60, 0, false, TEXT("-50%"), TEXT("60"))),
            MakeShared<FJsonValueObject>(Item(TEXT("mock-standard-1"), TEXT("Starter Pouch"), TEXT("standard"), TEXT("currency_pack"), 0.99, 0, 0, 20, 0, false, TEXT(""), TEXT("20"))),
        };
        FObj O = Obj();
        O->SetStringField(TEXT("status"), TEXT("success"));
        O->SetArrayField(TEXT("items"), Items);
        O->SetNumberField(TEXT("total_items"), Items.Num());
        return O;
    }

    // ---------------------------------------------------------------- device approval

    bool IsApprovalFlow(const FString& Flow)
    {
        return Flow == TEXT("send") || Flow == TEXT("transfer") || Flow == TEXT("send_receipt") || Flow == TEXT("transfer_receipt");
    }

    FInvoRawResponse DeviceBegin(const FObj& Body)
    {
        const FString Txn = Str(Body, TEXT("transaction_id"));
        const FString Flow = Str(Body, TEXT("flow"));
        if (Txn.IsEmpty())
        {
            return Err(400, TEXT("transaction_id is required"), TEXT("INVALID_INPUT"));
        }
        if (!IsApprovalFlow(Flow))
        {
            return Err(400, TEXT("unknown flow"), TEXT("INVALID_INPUT"));
        }
        FState& S = State();
        ++S.GrantSeq;
        const FString DeviceCode = FString::Printf(TEXT("mock-device-code-%d"), S.GrantSeq);
        const FString UserCode = FString::Printf(TEXT("MOCK-%04d"), S.GrantSeq);
        FGrant& G = S.Grants.Add(DeviceCode);
        G.TransactionId = Txn;
        G.Flow = Flow;
        FObj O = Obj();
        O->SetStringField(TEXT("device_code"), DeviceCode);
        O->SetStringField(TEXT("user_code"), UserCode);
        O->SetStringField(TEXT("verification_uri"), TEXT("https://sandbox.invo.network/device"));
        O->SetStringField(TEXT("verification_uri_complete"), TEXT("https://sandbox.invo.network/device?code=") + UserCode);
        O->SetNumberField(TEXT("expires_in"), 600);
        O->SetNumberField(TEXT("interval"), MockInterval);
        const FString Channel = Str(Body, TEXT("channel"));
        O->SetStringField(TEXT("channel"), Channel.IsEmpty() ? TEXT("qr") : Channel);
        return Ok(O);
    }

    FInvoRawResponse DevicePoll(const FObj& Body)
    {
        FGrant* G = State().Grants.Find(Str(Body, TEXT("device_code")));
        if (!G)
        {
            return Err(400, TEXT("invalid_grant"));
        }
        if (G->bDenied)
        {
            return Err(400, TEXT("access_denied"));
        }
        if (++G->Polls == 1)
        {
            FObj Pending = Obj();
            Pending->SetStringField(TEXT("error"), TEXT("authorization_pending"));
            Pending->SetNumberField(TEXT("interval"), MockInterval);
            if (FInvoMock::bEnrollmentPrompt && !G->bEnrolDecided)
            {
                FObj E = Obj();
                E->SetStringField(TEXT("state"), TEXT("awaiting_screen"));
                E->SetStringField(TEXT("device_label"), TEXT("iPhone (Safari)"));
                E->SetStringField(TEXT("match_code"), TEXT("4821"));
                E->SetStringField(TEXT("requested_at"), TEXT("2026-01-01T00:00:00+00:00"));
                Pending->SetObjectField(TEXT("enrollment"), E);
            }
            return Respond(400, Pending);
        }
        FObj O = Obj();
        O->SetStringField(TEXT("status"), TEXT("approved"));
        O->SetStringField(TEXT("transaction_id"), G->TransactionId);
        O->SetStringField(TEXT("flow"), G->Flow);
        return Ok(O);
    }

    FInvoRawResponse DeviceConfirmEnrollment(const FObj& Body)
    {
        const FString Decision = Str(Body, TEXT("decision")).ToLower();
        if (Decision != TEXT("approve") && Decision != TEXT("deny"))
        {
            return Err(400, TEXT("decision must be approve or deny"), TEXT("INVALID_INPUT"));
        }
        FGrant* G = State().Grants.Find(Str(Body, TEXT("device_code")));
        if (!G)
        {
            return Err(400, TEXT("invalid_grant"));
        }
        if (!FInvoMock::bEnrollmentPrompt)
        {
            return Err(409, TEXT("nothing to confirm: no phone has asked to enrol on this approval"), TEXT("DEVICE_APPROVAL_NO_ENROLLMENT_PENDING"));
        }
        if (G->bEnrolDecided)
        {
            return Err(409, TEXT("this enrolment was already decided"), TEXT("DEVICE_APPROVAL_ENROLLMENT_ALREADY_DECIDED"));
        }
        G->bEnrolDecided = true;
        G->bDenied = Decision == TEXT("deny");
        FObj O = Obj();
        O->SetStringField(TEXT("status"), G->bDenied ? TEXT("denied") : TEXT("confirmed"));
        return Ok(O);
    }

    FInvoRawResponse DeviceSettle(const FString& Path, const FObj& Body)
    {
        const bool bReceipt = Path.EndsWith(TEXT("/confirm-receipt"));
        const bool bSend = Path.Contains(TEXT("/api/sdk/send/"));
        const FString Flow = bSend ? (bReceipt ? TEXT("send_receipt") : TEXT("send")) : (bReceipt ? TEXT("transfer_receipt") : TEXT("transfer"));
        TArray<FString> Parts;
        Path.ParseIntoArray(Parts, TEXT("/"));
        const FString Txn = Parts.Num() >= 2 ? FGenericPlatformHttp::UrlDecode(Parts[Parts.Num() - 2]) : FString();

        FState& S = State();
        FGrant* G = S.Grants.Find(Str(Body, TEXT("device_code")));
        if (!G || G->TransactionId != Txn || G->Flow != Flow || G->Polls < 2)
        {
            return Err(400, TEXT("invalid_grant"));
        }
        if (G->bSettled)
        {
            FObj O = Obj();
            O->SetStringField(TEXT("status"), TEXT("error"));
            O->SetStringField(TEXT("code"), TEXT("TRANSACTION_NOT_PENDING"));
            O->SetStringField(TEXT("current_status"), bReceipt ? TEXT("completed") : TEXT("pending_claim"));
            O->SetStringField(TEXT("message"), TEXT("Transaction not pending"));
            return Respond(400, O);
        }
        G->bSettled = true;
        FObj O = Obj();
        O->SetStringField(TEXT("transaction_id"), Txn);
        if (bReceipt)
        {
            const FString Amount = PendingAmount(Txn);
            S.Collected.Add(Txn);
            S.Balance += FCString::Atod(*Amount);
            S.Settled.Add(Txn, TEXT("completed"));
            O->SetStringField(TEXT("status"), TEXT("completed"));
            O->SetStringField(TEXT("amount_received"), Amount);
            return Ok(O);
        }
        S.Settled.Add(Txn, TEXT("approved"));
        O->SetStringField(TEXT("status"), TEXT("approved"));
        O->SetStringField(TEXT("next"), TEXT("pending_claim"));
        if (Flow == TEXT("transfer"))
        {
            O->SetStringField(TEXT("claim_code"), TEXT("MOCKCLAIM1"));
        }
        return Ok(O);
    }

    FInvoRawResponse TransactionStatus(const FString& Path)
    {
        TArray<FString> Parts;
        Path.ParseIntoArray(Parts, TEXT("/"));
        const FString Txn = Parts.Num() >= 2 ? FGenericPlatformHttp::UrlDecode(Parts[Parts.Num() - 2]) : FString();
        FObj O = Obj();
        O->SetStringField(TEXT("transaction_id"), Txn);
        if (const FString* SettledState = State().Settled.Find(Txn))
        {
            O->SetStringField(TEXT("transaction_status"), *SettledState == TEXT("completed") ? TEXT("completed") : TEXT("pending_claim"));
            O->SetStringField(TEXT("verification_state"), *SettledState);
        }
        else
        {
            O->SetStringField(TEXT("transaction_status"), TEXT("pending_verification"));
            O->SetStringField(TEXT("verification_state"), TEXT("pending"));
        }
        return Ok(O);
    }

    // ---------------------------------------------------------------- phone

    FInvoRawResponse PhoneChange(const FObj& Body)
    {
        FState& S = State();
        const FString NewPhone = Str(Body, TEXT("new_phone"));
        if (NewPhone.IsEmpty())
        {
            return Err(400, TEXT("new_phone is not a valid phone number"), TEXT("INVALID_PHONE"));
        }
        FObj O = Obj();
        if (NewPhone == S.Phone)
        {
            O->SetStringField(TEXT("status"), TEXT("unchanged"));
            return Ok(O);
        }
        if (FInvoMock::bPhoneSecured)
        {
            return Err(409, TEXT("This player's phone is verified, so the change needs the player's own confirmation."), TEXT("PHONE_CHANGE_REQUIRES_VERIFICATION"));
        }
        S.Phone = NewPhone;
        O->SetStringField(TEXT("status"), TEXT("updated"));
        O->SetStringField(TEXT("player_token"), TEXT("mock-player-token-2"));
        return Ok(O);
    }

    FInvoRawResponse PhoneChangeRequestCreate(const FObj& Body)
    {
        FState& S = State();
        const FString NewPhone = Str(Body, TEXT("new_phone"));
        if (NewPhone.IsEmpty())
        {
            return Err(400, TEXT("new_phone is not a valid phone number"), TEXT("INVALID_PHONE"));
        }
        if (NewPhone == S.Phone)
        {
            return Err(409, TEXT("That is already the player's number."), TEXT("PHONE_SAME"));
        }
        for (TPair<FString, FPhoneRequest>& Other : S.PhoneRequests)
        {
            if (Other.Value.Status == TEXT("pending"))
            {
                Other.Value.Status = TEXT("cancelled");
            }
        }
        const FString Id = FString::Printf(TEXT("mock-pcr-%d"), ++S.PhoneSeq);
        FPhoneRequest& R = S.PhoneRequests.Add(Id);
        R.Phone = NewPhone;
        R.Status = TEXT("pending");
        FObj O = Obj();
        O->SetStringField(TEXT("change_request_id"), Id);
        O->SetStringField(TEXT("status"), TEXT("pending_confirmation"));
        O->SetStringField(TEXT("delivery"), TEXT("email"));
        return Ok(O, 202);
    }

    FInvoRawResponse PhoneChangeRequestStatus(const FString& Id)
    {
        FState& S = State();
        FPhoneRequest* R = S.PhoneRequests.Find(Id);
        if (!R)
        {
            return Err(404, TEXT("Change request not found"), TEXT("CHANGE_REQUEST_NOT_FOUND"));
        }
        if (++R->Polls >= 2 && R->Status == TEXT("pending"))
        {
            R->Status = TEXT("confirmed");
            S.Phone = R->Phone;
        }
        FObj O = Obj();
        O->SetStringField(TEXT("change_request_id"), Id);
        O->SetStringField(TEXT("status"), R->Status);
        O->SetStringField(TEXT("new_phone_masked"), TEXT("***-***-") + R->Phone.Right(4));
        return Ok(O);
    }
}

void FInvoMock::Reset()
{
    State() = FState();
    bPhoneSecured = true;
    bEnrollmentPrompt = false;
}

FString FInvoMock::CurrentPhone()
{
    return State().Phone;
}

FInvoTransport FInvoMock::Transport()
{
    return [](const FString& Verb, const FString& Url, const TMap<FString, FString>&, const FString& Body, TFunction<void(const FInvoRawResponse&)> Done)
    {
        const FInvoRawResponse Response = Handle(Verb, Url, Body);
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Response, Done = MoveTemp(Done)](float)
        {
            Done(Response);
            return false;
        }), LatencySeconds);
    };
}

FInvoRawResponse FInvoMock::Handle(const FString& Verb, const FString& Url, const FString& BodyText)
{
    FString Path = Url;
    FString Query;
    Url.Split(TEXT("?"), &Path, &Query);
    const FObj Body = ParseBody(BodyText);
    FState& S = State();

    if (Path.EndsWith(TEXT("/api/sdk/player-token")))
    {
        FObj O = Obj();
        O->SetStringField(TEXT("token"), TEXT("mock-player-token"));
        O->SetStringField(TEXT("expires_at"), TEXT("2999-01-01T00:00:00Z"));
        O->SetStringField(TEXT("identity_id"), TEXT("mock-identity-1"));
        return Ok(O);
    }
    if (Path.EndsWith(TEXT("/api/sdk/balance")))
    {
        return Ok(BalanceBody());
    }
    if (Path.EndsWith(TEXT("/api/sdk/destinations")))
    {
        FString Direction;
        Query.Split(TEXT("direction="), nullptr, &Direction);
        return Ok(DestinationsBody(Direction));
    }
    if (Path.EndsWith(TEXT("/api/sdk/transfers/pending")))
    {
        return Ok(PendingBody());
    }
    if (Path.EndsWith(TEXT("/api/sdk/approvals/device/begin")))
    {
        return DeviceBegin(Body);
    }
    if (Path.EndsWith(TEXT("/api/sdk/approvals/device/poll")))
    {
        return DevicePoll(Body);
    }
    if (Path.EndsWith(TEXT("/api/sdk/approvals/device/confirm-enrollment")))
    {
        return DeviceConfirmEnrollment(Body);
    }
    if (Verb == TEXT("POST") && (Path.Contains(TEXT("/api/sdk/send/")) || Path.Contains(TEXT("/api/sdk/transfers/")))
        && (Path.EndsWith(TEXT("/approve")) || Path.EndsWith(TEXT("/confirm-receipt"))))
    {
        return DeviceSettle(Path, Body);
    }
    if (Path.EndsWith(TEXT("/api/v1/game-items/list")))
    {
        return Ok(GameItemsBody());
    }
    if (Path.EndsWith(TEXT("/api/sdk/players/phone")))
    {
        return PhoneChange(Body);
    }
    if (Path.EndsWith(TEXT("/api/sdk/players/phone/change-requests")))
    {
        return PhoneChangeRequestCreate(Body);
    }
    if (Verb == TEXT("GET") && Path.Contains(TEXT("/api/sdk/players/phone/change-requests/")))
    {
        FString Id;
        Path.Split(TEXT("/"), nullptr, &Id, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        return PhoneChangeRequestStatus(FGenericPlatformHttp::UrlDecode(Id));
    }
    if (Path.EndsWith(TEXT("/api/checkout/sessions")))
    {
        FObj O = Obj();
        O->SetStringField(TEXT("session_id"), TEXT("mock-session-1"));
        O->SetStringField(TEXT("checkout_url"), TEXT("https://sandbox.invo.network/mock-checkout?usd=") + Str(Body, TEXT("usd_amount")));
        O->SetNumberField(TEXT("expires_in_seconds"), 900);
        return Ok(O);
    }
    if (Path.EndsWith(TEXT("/api/item-purchases/purchase-item")))
    {
        ++S.Purchases;
        const double Total = FCString::Atod(*Str(Body, TEXT("total_price")));
        if (Total > S.Balance)
        {
            FObj O = Obj();
            O->SetStringField(TEXT("error"), TEXT("insufficient_balance"));
            O->SetStringField(TEXT("code"), TEXT("INSUFFICIENT_BALANCE"));
            O->SetStringField(TEXT("required_amount"), Money(Total));
            O->SetStringField(TEXT("current_balance"), Money(S.Balance));
            return Respond(400, O);
        }
        const double Previous = S.Balance;
        S.Balance -= Total;
        FObj O = Obj();
        O->SetStringField(TEXT("status"), TEXT("success"));
        O->SetStringField(TEXT("transaction_id"), FString::Printf(TEXT("mock-item-txn-%d"), S.Purchases));
        O->SetStringField(TEXT("new_balance"), Money(S.Balance));
        O->SetStringField(TEXT("previous_balance"), Money(Previous));
        O->SetStringField(TEXT("currency_name"), TEXT("Gold Crowns"));
        return Ok(O);
    }
    if (Path.EndsWith(TEXT("/claim-currency")) || Path.EndsWith(TEXT("/claim-transfer")))
    {
        S.Balance += 25.0;
        FObj O = Obj();
        O->SetStringField(TEXT("status"), TEXT("claimed"));
        O->SetStringField(TEXT("new_balance"), Money(S.Balance));
        return Ok(O);
    }
    if (Path.EndsWith(TEXT("/initiate-send")) || Path.EndsWith(TEXT("/initiate-transfer")))
    {
        FObj O = Obj();
        O->SetStringField(TEXT("status"), TEXT("success"));
        O->SetStringField(TEXT("transaction_id"), FString::Printf(TEXT("mock-out-txn-%d"), S.GrantSeq + 1));
        return Ok(O);
    }
    if (Path.EndsWith(TEXT("/status")))
    {
        return TransactionStatus(Path);
    }
    return Err(404, TEXT("mock route not found: ") + Path, TEXT("NOT_FOUND"));
}
