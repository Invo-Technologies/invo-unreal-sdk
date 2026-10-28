<p align="center">
  <img src="https://console.invo.network/assets/logo-e0324218.svg" alt="INVO logo" width="200"/>
</p>

<h1 align="center">INVO Network SDK for Unreal Engine</h1>

<p align="center">
  <strong>One virtual currency players buy once and spend across every INVO game.</strong>
</p>

<p align="center">
  <a href="#installation">Installation</a> •
  <a href="#quick-start">Quick Start</a> •
  <a href="#phone-approval">Phone Approval</a> •
  <a href="#api-reference">API Reference</a> •
  <a href="#support">Support</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Unreal%20Engine-5.8-blue" alt="Unreal Engine 5.8"/>
  <img src="https://img.shields.io/badge/Platforms-Windows%20%7C%20Android%20%7C%20iOS-brightgreen" alt="Platforms"/>
  <img src="https://img.shields.io/badge/Version-2.0-orange" alt="Version 2.0"/>
</p>

---

## Overview

The official Unreal Engine plugin for [INVO Network](https://ourinvo.com). Sell currency packs through INVO's hosted checkout, sell in-game items, and let players send currency to friends or move it to another INVO game, with every send approved on the player's phone.

- **`UInvoSubsystem`**: the whole API as a Game Instance Subsystem, in C++ and Blueprint. Every call answers once with an `FInvoResult` (`bOk`, `Data`, `Error`) carrying INVO's stable error codes.
- **A ready-made wallet** (`UInvoWalletSubsystem`): Store, Send, Transfer, Collect and phone screens in Slate, the same screens and copy as INVO's Unity and Godot wallets. Open it with one node.
- **Phone approval**: a QR code on desktop (`UInvoQrCodeLibrary`, encoded in-process), the system browser on phones (`UInvoHostedApprovalSubsystem`).
- **Error checks** in `UInvoLibrary`: Is Insufficient Balance, Is Rate Limited, Is Receiver Not Enrolled and more, so your UI never parses strings.
- **Same rules as INVO's Unity and Godot SDKs**: https only, money calls never retried automatically, idempotency keys on every purchase and send, and no production key in a build.
- **Automation tests** under `InvoSDK.*`.

### How calls are routed

```
                        player token (safe to ship)
  Your game ──────────────────────────────────────────▶ INVO  /api/sdk/*   balance, destinations,
     │                                                                     pending collect, phone approval
     │  commerce calls (store, checkout, purchase, send, transfer, claim, phone change)
     │
     ├── Sandbox, Development builds:  ivsdk_test_ key from your local settings ─▶ INVO sandbox
     │
     └── Production:  player token ─▶ YOUR game server ─▶ adds the ivsdk_live_ key ─▶ INVO
```

Your Platform API key never ships in a build. Shipping builds refuse direct sandbox mode.

---

## Installation

### Prerequisites

- Unreal Engine 5.8
- Visual Studio 2022 with the "Game development with C++" workload (Windows)
- An INVO developer account: free sandbox keys in Test mode at [console.invo.network/test](https://console.invo.network/test)

### Install

Install from Fab, or copy this repository into your project's `Plugins/InvoSDK` folder:

```
YourProject/
└── Plugins/
    └── InvoSDK/
        ├── Content/
        ├── Resources/
        ├── Source/
        └── InvoSDK.uplugin
```

Enable **Edit > Plugins > INVO Network SDK** and restart the editor.

### Configure

**Project Settings > Plugins > Invo SDK** (saved to `DefaultEngine.ini`, safe to commit):

| Setting | What it is |
|---|---|
| Environment | Sandbox or Production |
| Sandbox Game ID / Production Game ID | Numeric game IDs from the console |
| Game ID | The ID behind the `invo-sdk-<GameId>` return link for phone approval |
| Game Server URL | Production: your server that forwards commerce calls with your `ivsdk_live_` key |
| Player Token Endpoint | Production: your endpoint that mints player tokens (`{"token", "identity_id"}`) |
| Checkout Session Endpoint | Production: your endpoint that creates hosted checkout sessions |
| Game Name, Currency Name | Display only |

**Project Settings > Plugins > Invo SDK (Local)** holds the **Sandbox Platform API key** (`ivsdk_test_…`). It is saved per user (`EditorPerProjectUserSettings`), never to a file you commit, and is not read in Shipping builds. On CI, set `INVO_SANDBOX_SECRET` instead.

---

## Quick Start

### Try the wallet in 30 seconds

Open `/InvoSDK/Map/InvoWalletDemo` and press Play. With nothing configured, the **INVO Wallet Demo** actor switches the SDK to its offline mock (Development builds only) and signs in a demo player, so every screen works with no network: buy a pack, send to a phone, approve with the QR code, collect. Press F8 to toggle the wallet. Add your sandbox key in Project Settings and the same map talks to the real sandbox.

In your own game: **Get Game Instance > Invo Wallet Subsystem > Open Wallet** (`Store`, `Send`, `Transfer`, `Collect` or `Phone`). Set the items it sells with **Set Item Catalog**, and listen to **On Wallet Changed** to refresh your HUD.

### Blueprint

1. **Get Game Instance > Invo Subsystem**.
2. Sandbox: **Login Sandbox** (email, phone, display name). Production: **Login Via Backend** with whatever JSON your token endpoint expects.
3. **Get Balance**, **Get Game Items**, **Open Checkout** ("4.99"), **Purchase Item**: each has an **On Done** event with an `Invo Result`. Break it: `Ok`, `Data` (JSON), `Error`.
4. Bind **On Request Failed** once for global error UI.

### C++

```cpp
#include "InvoSubsystem.h"
#include "InvoLibrary.h"

void AMyHUD::BeginPlay()
{
    Super::BeginPlay();
    UInvoSubsystem* Invo = GetGameInstance()->GetSubsystem<UInvoSubsystem>();

    Invo->GetBalanceNative([](const FInvoResult& Result)
    {
        if (!Result.bOk)
        {
            UE_LOG(LogTemp, Warning, TEXT("Balance failed: %s"), *Result.Error.ToString());
            return;
        }
        // Result.Object() is the parsed JSON: {"balances": [...]}. An empty array is a new player.
    });
}
```

### Buying an item

```cpp
const FString RequestId = UInvoLibrary::NewClientRequestId(); // keep it: reuse it to retry this purchase
FInvoResultDelegate OnDone;
OnDone.BindUFunction(this, FName("OnPurchased"));
Invo->PurchaseItem(RequestId, TEXT("sword_01"), TEXT("Iron Sword"), 1, TEXT("50"), TEXT("50"), OnDone);
```

`UFUNCTION() void OnPurchased(const FInvoResult& Result)` then checks `UInvoLibrary::IsInsufficientBalance(Result.Error)` to offer a top-up with `OpenCheckout`.

### Money calls are never retried for you

A failed send, transfer or purchase is ambiguous about the money. Before telling the player it failed, read `GetSendStatus` / `GetTransferStatus`, or retry with the **same** `ClientRequestId` (the backend answers `DUPLICATE_REQUEST` for one already done). Reads (balance, items, status) retry on 5xx, network errors and short 429s.

---

## Phone approval

Sends and transfers are approved on the player's phone. The flow is RFC 8628 (device grant):

1. `InitiateSend` / `InitiateTransfer` returns a `transaction_id`.
2. `BeginPhoneApproval(TransactionId, "send" | "transfer", DefaultApprovalChannel())` returns `device_code`, `verification_uri_complete` and `interval`.
3. Desktop (`"qr"`): show `UInvoQrCodeLibrary::MakeQrCodeTexture(verification_uri_complete)` in an Image widget. Phones (`"app_browser"`): open it with `UInvoHostedApprovalSubsystem`, below.
4. Poll `PollPhoneApproval(DeviceCode)` every `NextPollInterval(...)` seconds until `status` is `approved` (or `access_denied` / `expired_token`).
5. `SettlePhoneApproval(TransactionId, Flow, DeviceCode)` moves the money. It is single use and never retried; on an error read the status first.

For a first-time phone the poll carries an `enrollment` block with a match code; answer it with `ConfirmPhoneEnrollment(DeviceCode, bApprove)` (see the match-code prompt below).

```cpp
UTexture2D* Qr = UInvoQrCodeLibrary::MakeQrCodeTexture(VerificationUriComplete); // 8 px per module, 4-module quiet zone
QrImage->SetBrushFromTexture(Qr);
```

The QR encoder (`Private/InvoQrCodeCore.h`, ISO/IEC 18004 byte mode) runs in the game, so the approval link never goes to a third-party QR service. It is checked bit for bit against Project Nayuki's reference encoder.

### Hosted approval on mobile (system browser)

Transfers, sends and claims can be approved with a passkey on **Invo's hosted approval page** instead of an SMS PIN. On iOS and Android the plugin opens that page in the **system browser** and tells you when the player is back. Nothing in the plugin can move money: settlement happens on your server, with a value the client never sees.

#### Two halves

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                       HOSTED APPROVAL (MOBILE)                               │
├──────────────────────────────────────────────────────────────────────────────┤
│  Game client          Your Server                 Invo API        Browser    │
│    │                      │                          │               │       │
│    │──"approve tx 123"───▶│                          │               │       │
│    │                      │─POST /api/sdk/approvals/device/begin──▶  │       │
│    │                      │  {transaction_id, flow, channel:"app_browser"}   │
│    │                      │◀─{device_code, verification_uri_complete,        │
│    │                      │    user_code, expires_in, interval}      │       │
│    │◀─handoff (NO device_code)                       │               │       │
│    │                      │                          │               │       │
│    │──OpenHostedApproval(VerificationUriComplete)────────────────────▶│      │
│    │                      │                          │◀──passkey─────│       │
│    │                      │──poll (device_code)─────▶│               │       │
│    │◀─poll relay {enrollment?}                       │               │       │
│    │  match-code prompt → decision                   │               │       │
│    │──decision───────────▶│─POST .../confirm-enrollment {device_code,decision}│
│    │                      │                          │               │       │
│    │◀════ invo-sdk-<GameId>://done  (carries NOTHING) ══════════════│       │
│    │──"poll now"─────────▶│──poll → approved────────▶│               │       │
│    │                      │─POST /transfers/<id>/approve {device_code}──▶    │
└──────────────────────────────────────────────────────────────────────────────┘
```

**Server half (yours).** Call `POST /api/sdk/approvals/device/begin` with `{transaction_id, flow, "channel": "app_browser"}`. The response is RFC 8628-shaped: keep `device_code` on the server — it is what polls and what settles — and hand the client only `verification_uri_complete`, `user_code`, `expires_in` and `interval` (`FInvoHostedApprovalHandoff`; `ParseHostedApprovalHandoff` reads it and ignores a `device_code` relayed by mistake). **Start polling on `interval` the moment `begin` returns**, not when the client says the page is done: the page can finish while the game is backgrounded, and the return signal is only a hint to poll sooner. When the poll says approved, call the approve endpoint with `device_code`.

**Client half (this plugin).** `UInvoHostedApprovalSubsystem` (a `GameInstanceSubsystem`, Blueprint and C++) does two things and nothing else:

```cpp
#include "InvoHostedApproval.h"

UInvoHostedApprovalSubsystem* Hosted = GetGameInstance()->GetSubsystem<UInvoHostedApprovalSubsystem>();

// 1. Open the page in the system browser (FPlatformProcess::LaunchURL). Never an embedded
//    WebView: WebAuthn does not run in one, so the passkey ceremony would fail before it
//    started. Requires https and a numeric GameId in Project Settings > Plugins > Invo SDK.
Hosted->OnApprovalPageFinished.AddDynamic(this, &AMyPlayerController::PollSoonerPlease); // wake-up only
if (!Hosted->OpenHostedApproval(Handoff.VerificationUriComplete))
{
    ShowFallbackCode(Handoff.UserCode);
}

// 2. Your server was already polling since `begin`; OnApprovalPageFinished just asks it to
//    poll now. Remove the binding in EndPlay:
//    Hosted->OnApprovalPageFinished.RemoveDynamic(this, &AMyPlayerController::PollSoonerPlease);
```

Blueprint: `Get InvoHostedApprovalSubsystem` → `Open Hosted Approval` and bind `On Approval Page Finished`.

- `OnApprovalPageFinished` fires when the game returns to the foreground after `OpenHostedApproval` — on both platforms the `invo-sdk-<GameId>://done` navigation is what brings the game back. It means **"poll now"** — never "approved". Approved, denied and expired all return on the same bare URL; the truth is your server's poll. Nothing beyond the scheme is ever read, so a value smuggled onto the URL cannot tell the game anything.
- Games with their own deep-link plumbing can call `HandleDeepLink(Url)` (matches only the scheme) or `NotifyApprovalPageFinished()`.

#### First-time phones: no prompt on mobile

For `channel: "app_browser"` the phone running the page **is** the device that opened it, so Invo **auto-confirms** the enrolment: the poll goes straight from pending to approved and the game shows nothing. There is no "come back, answer on the game screen, reopen" dance on iOS/Android.

The match-code prompt API below exists for games that run the same flow on a screen that is *not* the phone (tablet/desktop builds of a mobile title, where the poll carries `enrollment: {state, device_label, match_code, requested_at}`). It is not required on mobile.

| `state` | The plugin shows | Player action |
|---|---|---|
| `awaiting_screen` | "Set up INVO on "`<device_label>`"? Code `<match_code>`. Say Yes only if the phone you just opened shows this code." with **Yes** / **No** | Decision goes to your server as `approve` / `deny` |
| `confirmed` | "Finishing on "`<device_label>`"…" with one button, **Stop it (wrong code)** | Stop sends `deny` — the server lets a deny override a pending confirm |
| `denied` / absent | nothing | — |

```cpp
// Each poll tick, relay the poll's `enrollment` block. ParseEnrollmentState returns an empty
// State when the block is absent; the subsystem shows, hides, or ignores an unchanged block.
FInvoEnrollmentState Enrollment;
UInvoHostedApprovalLibrary::ParseEnrollmentState(PollJson, Enrollment);
Hosted->ApplyEnrollmentStateNative(Enrollment, [this](EInvoEnrollmentDecision Decision)
{
    MyServer->ConfirmEnrollment(UInvoHostedApprovalLibrary::DecisionToWire(Decision)); // "approve" | "deny"
});
```

Blueprint: `Apply Enrollment State` with a `Decision` event each poll tick. `ShowEnrollmentPrompt` and `ShowEnrollmentFinishing` are also callable directly. The default surface is a Slate overlay on the game viewport (`SInvoEnrollmentPrompt`) that needs no UMG asset. To draw the prompt in your own UMG, set `bUseBuiltInPrompt = false`, bind `OnEnrollmentPromptRequested(Message, bFinishing)` / `OnEnrollmentPromptHidden`, and call `SubmitEnrollmentDecision` from your buttons — the copy is passed in, so your widget only lays it out. Labels and codes come from the server and are sanitised for display (tags, control and Unicode format characters removed; the label is capped at 32 characters and quoted, codes at 64). In the finishing state `SubmitEnrollmentDecision` always delivers `Deny`, whatever a custom widget passes.

#### Registering the return scheme

The return scheme is `invo-sdk-<GameId>`, derived from the numeric **GameId** in **Project Settings > Plugins > Invo SDK** (`DefaultEngine.ini`, `[/Script/InvoSDK.InvoSDKSettings]`, `GameId=12345`) and nothing else; the server derives the same value. The editor refuses a non-numeric value.

**Android** is automatic: `InvoSDK_Android_UPL.xml` adds the intent-filter to the **stock** game activity (`com.epicgames.unreal.GameActivity`, or `com.epicgames.ue4.GameActivity` on UE4) at package time, reading the same setting. If your project replaces the launch activity with its own, the UPL does nothing and you add the snippet by hand:

```xml
<intent-filter>
  <action android:name="android.intent.action.VIEW" />
  <category android:name="android.intent.category.DEFAULT" />
  <category android:name="android.intent.category.BROWSABLE" />
  <data android:scheme="invo-sdk-12345" />
</intent-filter>
```

**iOS** needs one manual step — one line in `DefaultEngine.ini` (or paste the XML into **Project Settings > Platforms > iOS > Extra PList Data**), replacing `12345` with your game id:

```ini
[/Script/IOSRuntimeSettings.IOSRuntimeSettings]
AdditionalPlistData=<key>CFBundleURLTypes</key><array><dict><key>CFBundleURLName</key><string>invo-sdk-12345</string><key>CFBundleURLSchemes</key><array><string>invo-sdk-12345</string></array></dict></array>
```

Without it, Safari cannot open `invo-sdk-12345://done` and the player returns to the game by hand; the flow still completes because the poll is the truth. A custom scheme can be claimed by another app on Android; that is why the callback carries nothing.

#### Native bridges (optional)

The plugin opens the page with `FPlatformProcess::LaunchURL`: Safari on iOS, the default browser on Android. Both run WebAuthn. An `ASWebAuthenticationSession` (iOS) or Custom Tabs (Android) bridge gives a sheet that closes itself on return; the plugin does not ship one. If you add one, keep the contract: open only `https`, read nothing from the callback URL, and call `NotifyApprovalPageFinished()` when the sheet closes.


---

## API Reference

### `UInvoSubsystem`

| Area | Calls |
|---|---|
| Config | `Setup(Environment, BaseUrlOverride)`, `DefaultBaseUrl`, `IsCommerceReady`, `CommerceProblem`, `GetEnvironment`, `GetGameId` |
| Player | `SetPlayerToken`, `LoginViaBackend`, `LoginSandbox`, `SetActivePlayer`, `GetBalance`, `GetDestinations("send" or "transfer")`, `GetPendingCollect` |
| Phone approval | `BeginPhoneApproval`, `PollPhoneApproval`, `ConfirmPhoneEnrollment`, `SettlePhoneApproval`, `IsPastStep`, `NextPollInterval`, `DefaultApprovalChannel` |
| Store | `GetGameItems`, `OpenCheckout(UsdAmount)`, `PurchaseItem` |
| Send, transfer, collect | `InitiateSend`, `InitiateTransfer`, `ClaimCurrency`, `ClaimTransfer`, `GetSendStatus`, `GetTransferStatus` |
| Phone number | `ChangePlayerPhone`, `CreatePhoneChangeRequest`, `GetPhoneChangeRequest` |
| Events | `OnLoggedIn`, `OnBalanceUpdated`, `OnCheckoutOpened`, `OnRequestFailed` |

A player token that expires mid-session (`SDK_TOKEN_EXPIRED`) is re-minted once through the Player Token Endpoint and the call is resent; concurrent calls share one refresh.

### `FInvoResult` and `FInvoError`

`FInvoResult { bOk, Data, Error }`. `FInvoError { Message, Code, Status, RequestId, Body }`: `Code` is INVO's stable code (`INSUFFICIENT_BALANCE`, `RECEIVER_NOT_ENROLLED`, `KEY_FOR_OTHER_ENVIRONMENT`, …). Quote `RequestId` when contacting support.

### `UInvoLibrary`

- Validation: `IsE164Phone`, `FormatMoney`, `NewClientRequestId`, `IsValidMoney` (C++)
- Security: `SecureBaseUrlProblem`, `KeyEnvironment`
- Error checks: `IsTokenExpired`, `IsKeyForOtherEnvironment`, `IsReceiverNotEnrolled`, `IsInsufficientBalance`, `IsPhoneShareApprovalRequired`, `IsPhoneChangeRequiresVerification`, `IsPasskeyRecoveryCooldown`, `IsDuplicateRequest`, `IsRateLimited`, `GetRetryAfterSeconds`, `GetErrorBodyString`

### `UInvoWalletSubsystem`

`OpenWallet(Panel)`, `CloseWallet`, `ToggleWallet`, `IsWalletOpen`, `SetItemCatalog`, `OnWalletVisibilityChanged`, `OnWalletChanged`. The wallet is drawn in Slate over the game viewport and uses Project Settings > Invo SDK > Game Name and Currency Name. `AInvoWalletDemoActor` opens it on BeginPlay and toggles it with F8.

`UInvoSubsystem::EnableOfflineMock()` (Development builds) answers every call locally with demo data: the wallet, your own UI and the automation tests can run with no network or credentials.

### `UInvoQrCodeLibrary`

`MakeQrCodeTexture(Text, PixelsPerModule, QuietZoneModules, DarkColor, LightColor)` and, in C++, `EncodeQrModules(Text, OutSize, OutModules)`.

### Low-level requests (1.x)

`UInvoSDKApiManager::InvoRequest` (an async Blueprint node with `OnSuccess` / `OnFail`) and `UInvoSDKJsonHelpers` are kept for 1.x projects. New code should use `UInvoSubsystem`, which adds the routing, validation, retries and error codes above.

---

## Tests

Session Frontend > Automation, filter `InvoSDK`, or:

```
UnrealEditor-Cmd.exe <Project>.uproject -ExecCmds="Automation RunTests InvoSDK;Quit" -unattended -nopause -NullRHI
```

| Suite | Covers |
|---|---|
| `InvoSDK.Core.*` | money and phone validation, URL and key rules, error parsing, HTTP retry rules (fake transport, no network), approval helpers, QR encoding |
| `InvoSDK.HostedApproval.*` | the hosted approval helpers, parsers and return-link handling |

---

## Support

- Documentation: [docs.invo.network](https://docs.invo.network)
- Developer console: [console.invo.network](https://console.invo.network) (sandbox keys in Test mode at [console.invo.network/test](https://console.invo.network/test))
- Email: support@ourinvo.com
- Website: [ourinvo.com](https://ourinvo.com)

---

## Changelog

### 2.0.0

- **`UInvoSubsystem`**: the full INVO API in C++ and Blueprint (player, store, checkout, item purchase, send, transfer, collect, claim, phone change, phone approval), at parity with INVO's Unity and Godot SDKs.
- **Commerce routing**: sandbox key straight to INVO in Development builds only; production through your game server with the player token. Shipping builds refuse direct mode.
- **`FInvoResult` / `FInvoError`** with INVO's stable error codes and the `UInvoLibrary` error checks.
- **HTTP rules**: https only (loopback exempt), retries only for reads, redacted URLs in logs, sanitised headers, single-flight token refresh.
- **Wallet**: `UInvoWalletSubsystem`, a Slate wallet with Store, Send, Transfer, Collect and phone screens, and `AInvoWalletDemoActor` with the `InvoWalletDemo` map. Replaces the 1.x UMG widgets and demo map.
- **Offline mock**: `EnableOfflineMock()` for demos and UI work without credentials (Development builds only).
- **QR codes**: `UInvoQrCodeLibrary` for desktop phone approval.
- **Hosted approval on mobile**: `UInvoHostedApprovalSubsystem` opens INVO's hosted approval page in the system browser and returns on `invo-sdk-<GameId>://done`.
- **Settings**: sandbox and production game IDs, game server and endpoint URLs; the sandbox key moved to per-user local settings.
- Automation tests under `InvoSDK.Core` and `InvoSDK.HostedApproval`.

### 1.0.0

- Initial release: `UInvoSDKApiManager`, JSON helpers, UMG widgets, sandbox and production.
