<p align="center">
  <img src="https://console.invo.network/assets/logo-e0324218.svg" alt="InvoSDK Logo" width="200"/>
</p>

<h1 align="center">InvoSDK for Unreal Engine</h1>

<p align="center">
  <strong>The official Unreal Engine plugin for integrating with the Invo platform</strong>
</p>

<p align="center">
  <a href="#installation">Installation</a> •
  <a href="#quick-start">Quick Start</a> •
  <a href="#features">Features</a> •
  <a href="#api-reference">API Reference</a> •
  <a href="#hosted-approval-on-mobile-system-browser">Hosted Approval</a> •
  <a href="#support">Support</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Unreal%20Engine-4.27%2B%20%7C%205.x-blue" alt="UE Version"/>
  <img src="https://img.shields.io/badge/Platform-Windows-brightgreen" alt="Platform"/>
  <img src="https://img.shields.io/badge/Version-1.0-orange" alt="Version"/>
  <img src="https://img.shields.io/badge/License-Proprietary-red" alt="License"/>
</p>

---

## Overview

InvoSDK is an enterprise-grade Unreal Engine plugin that seamlessly integrates your game with the **Invo platform** — enabling in-game currency management, item purchases, player-to-player transfers, and real-money transactions. Built with performance and ease-of-use in mind, the SDK provides both C++ and Blueprint interfaces for maximum flexibility.

### Why InvoSDK?

- **Production-Ready**: Battle-tested infrastructure handling millions of transactions
- **Dual Interface**: Full C++ API with complete Blueprint exposure
- **Async by Design**: Non-blocking HTTP operations for smooth gameplay
- **Pre-built UI**: Ready-to-use UMG widgets for common flows
- **Secure**: Industry-standard authentication and encryption

---

## Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              YOUR GAME                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌──────────────┐     ┌──────────────────┐     ┌─────────────────────┐    │
│   │  Blueprints  │────▶│  BP_InvoSDK      │────▶│  Pre-built UI       │    │
│   │  (Your Game) │     │  Manager         │     │  Widgets            │    │
│   └──────────────┘     └────────┬─────────┘     └─────────────────────┘    │
│                                 │                                           │
│                                 ▼                                           │
│   ┌──────────────┐     ┌──────────────────┐     ┌─────────────────────┐    │
│   │  C++ Game    │────▶│  UInvoSDKApi     │────▶│  UInvoSDKJson       │    │
│   │  Code        │     │  Manager         │     │  Helpers            │    │
│   └──────────────┘     └────────┬─────────┘     └─────────────────────┘    │
│                                 │                                           │
├─────────────────────────────────┼───────────────────────────────────────────┤
│                          INVOSDK PLUGIN                                      │
│                                 │                                           │
│                                 ▼                                           │
│                    ┌────────────────────────┐                               │
│                    │   Unreal HTTP Module   │                               │
│                    │   (Async Requests)     │                               │
│                    └────────────┬───────────┘                               │
│                                 │                                           │
└─────────────────────────────────┼───────────────────────────────────────────┘
                                  │
                                  ▼ HTTPS
                    ┌─────────────────────────┐
                    │     INVO PLATFORM       │
                    │  ┌───────────────────┐  │
                    │  │  Authentication   │  │
                    │  │  /auth/*          │  │
                    │  └───────────────────┘  │
                    │  ┌───────────────────┐  │
                    │  │  API Services     │  │
                    │  │  /api/*           │  │
                    │  └───────────────────┘  │
                    └─────────────────────────┘
```

### Request Flow

```
┌─────────┐    ┌──────────────┐    ┌─────────────┐    ┌───────────┐    ┌─────────┐
│  Game   │    │ InvoRequest  │    │  Activate   │    │  HTTP     │    │  Invo   │
│  Code   │    │  (Factory)   │    │  (Execute)  │    │  Module   │    │  API    │
└────┬────┘    └──────┬───────┘    └──────┬──────┘    └─────┬─────┘    └────┬────┘
     │                │                   │                 │               │
     │  Create Node   │                   │                 │               │
     │───────────────▶│                   │                 │               │
     │                │                   │                 │               │
     │                │  Configure        │                 │               │
     │                │──────────────────▶│                 │               │
     │                │                   │                 │               │
     │                │                   │  Send Request   │               │
     │                │                   │────────────────▶│               │
     │                │                   │                 │               │
     │                │                   │                 │  HTTPS POST   │
     │                │                   │                 │──────────────▶│
     │                │                   │                 │               │
     │                │                   │                 │  JSON Response│
     │                │                   │                 │◀──────────────│
     │                │                   │                 │               │
     │                │                   │  HandleResponse │               │
     │                │                   │◀────────────────│               │
     │                │                   │                 │               │
     │  OnSuccess/OnFail (Delegate)       │                 │               │
     │◀───────────────────────────────────│                 │               │
     │                │                   │                 │               │
```

### Module Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                       InvoSDK Module                         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Public Dependencies          Private Dependencies           │
│  ┌─────────────────┐         ┌─────────────────┐            │
│  │ Core            │         │ CoreUObject     │            │
│  │ HTTP            │         │ Engine          │            │
│  │ Json            │         │ Slate           │            │
│  │ JsonUtilities   │         │ SlateCore       │            │
│  │                 │         │ DeveloperSettings│           │
│  └─────────────────┘         └─────────────────┘            │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Installation

### Prerequisites

- Unreal Engine 4.27+ or Unreal Engine 5.x
- Windows 64-bit development environment
- Visual Studio 2019 or later (for C++ development)
- An Invo developer account ([Register here](https://console.invo.network))

### Step 1: Download the Plugin

Download the latest release from the [Invo Developer Portal](https://console.invo.network) or clone this repository.

### Step 2: Install the Plugin

Copy the `InvoSDK` folder into your project's `Plugins` directory:

```
YourProject/
├── Content/
├── Source/
└── Plugins/
    └── InvoSDK/           ← Place here
        ├── Source/
        ├── Content/
        ├── Binaries/
        └── InvoSDK.uplugin
```

### Step 3: Enable the Plugin

1. Open your project in Unreal Editor
2. Go to **Edit → Plugins**
3. Search for "InvoSDK"
4. Enable the plugin and restart the editor

### Step 4: Configure the SDK

The setup wizard will automatically create an `InvoSDKConfig` asset on first launch. Configure it with your credentials:

| Field | Description | Required |
|-------|-------------|----------|
| `sdkKey` | Your Invo API secret key | Yes |
| `gameId` | Your unique game identifier | Yes |
| `gameName` | Display name for your game | Yes |
| `playerEmail` | Current player's email | Yes |
| `gameIconUrl` | URL to your game icon | No |
| `gameCurrencyName` | Name of your in-game currency | No |
| `gameCurrencyUrl` | URL to currency icon | No |
| `gameVersion` | Your game version string | No |
| `useProduction` | Toggle production/sandbox | Yes |

> ⚠️ **Security Warning**: Never ship `sdkKey` in client builds. Use server-side authentication for production deployments.

---

## Environments

InvoSDK supports two environments for development and production use:

### Sandbox (Development & Testing)

```
Host:     https://sandbox.invo.network
API:      /sandbox/api/*
Auth:     /sandbox/auth/*
Console:  https://dev.console.invo.network
```

**Always use sandbox for development and testing.** Sandbox provides:
- Test accounts with unlimited currency
- No real money transactions
- Full API feature parity with production
- Isolated data that won't affect production

### Production (Live Games Only)

```
Host:     https://invo.network
API:      /api/*
Auth:     /auth/*
Console:  https://console.invo.network
```

> 🔒 **Important**: Only switch to production when your game is ready for release. All transactions in production involve real money.

---

## Quick Start

### Blueprint Quick Start

1. **Create an API Request Node**

   In your Blueprint, right-click and search for `Invo Request` under the **InvoSDK | API** category.

2. **Configure the Request**

   ```
   Base URL:  https://sandbox.invo.network
   Endpoint:  /sandbox/api/player/balance
   Method:    GET
   Headers:   Authorization: Bearer <your_token>
   Body:      (empty for GET)
   ```

3. **Handle the Response**

   Connect the `OnSuccess` and `OnFail` execution pins to your response handlers.

### C++ Quick Start

```cpp
#include "InvoSDKApiManager.h"
#include "InvoSDKJsonHelpers.h"

void AMyActor::FetchPlayerBalance()
{
    TMap<FString, FString> Headers;
    Headers.Add(TEXT("Authorization"), TEXT("Bearer ") + AuthToken);

    UInvoSDKApiManager* Request = UInvoSDKApiManager::InvoRequest(
        this,
        TEXT("https://sandbox.invo.network"),
        TEXT("/sandbox/api/player/balance"),
        Headers,
        TEXT(""),  // Empty body for GET
        EInvoHttpMethod::GET
    );

    Request->OnSuccess.AddDynamic(this, &AMyActor::OnBalanceReceived);
    Request->OnFail.AddDynamic(this, &AMyActor::OnRequestFailed);
    Request->Activate();
}

void AMyActor::OnBalanceReceived(const FString& Content, int32 StatusCode,
                                  bool bSuccess, const FJsonObjectWrapper& Json)
{
    int32 Balance = UInvoSDKJsonHelpers::GetIntegerField(Json, TEXT("balance"));
    UE_LOG(LogTemp, Log, TEXT("Player balance: %d"), Balance);
}

void AMyActor::OnRequestFailed(const FString& Content, int32 StatusCode,
                                bool bSuccess, const FJsonObjectWrapper& Json)
{
    UE_LOG(LogTemp, Error, TEXT("Request failed [%d]: %s"), StatusCode, *Content);
}
```

---

## Features

### Core Features

| Feature | Description | Blueprint | C++ |
|---------|-------------|:---------:|:---:|
| Player Balance | Get/update player currency balance | ✅ | ✅ |
| Item Catalog | Fetch available items for purchase | ✅ | ✅ |
| Item Purchase | Process item purchases | ✅ | ✅ |
| Currency Transfer | Player-to-player transfers | ✅ | ✅ |
| SMS Verification | Two-factor authentication | ✅ | ✅ |
| Transaction History | View past transactions | ✅ | ✅ |

### Pre-built UI Components

The SDK includes ready-to-use UMG widgets located in `Content/UI/`:

| Widget Category | Description |
|-----------------|-------------|
| `DailyDeals/` | Daily rotating offers display |
| `Featured/` | Featured items showcase |
| `FeaturedBundleBox/` | Bundle promotion displays |
| `Item/` | Individual item cards |
| `ItemPurchase/` | Purchase confirmation flow |
| `NavBar/` | Navigation components |
| `QuickSend/` | Quick currency send interface |
| `RecentPurchases/` | Purchase history display |
| `SendCurrency/` | Currency sending flow |
| `TransferCurrency/` | Full transfer interface |
| `WeeklyFeaturedBundle/` | Weekly bundle promotions |

---

## API Reference

### UInvoSDKApiManager

The core class for making HTTP requests to the Invo API.

#### InvoRequest (Static Factory)

Creates and configures a new API request node.

```cpp
UFUNCTION(BlueprintCallable, Category = "InvoSDK|API")
static UInvoSDKApiManager* InvoRequest(
    UObject* WorldContextObject,
    const FString& BaseUrl,
    const FString& Endpoint,
    const TMap<FString, FString>& Headers,
    const FString& Body,
    EInvoHttpMethod Method
);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `WorldContextObject` | `UObject*` | World context for the request |
| `BaseUrl` | `FString` | Base URL (e.g., `https://sandbox.invo.network`) |
| `Endpoint` | `FString` | API endpoint (e.g., `/sandbox/api/player/balance`) |
| `Headers` | `TMap<FString, FString>` | HTTP headers (Authorization, etc.) |
| `Body` | `FString` | Request body (JSON string for POST/PUT) |
| `Method` | `EInvoHttpMethod` | HTTP method (GET, POST, PUT, DELETE) |

#### Delegates

```cpp
// Called on successful response (2xx status codes)
UPROPERTY(BlueprintAssignable)
FInvoSDKOnResponse OnSuccess;

// Called on failed response (non-2xx or network error)
UPROPERTY(BlueprintAssignable)
FInvoSDKOnResponse OnFail;
```

#### Delegate Signature

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FInvoSDKOnResponse,
    const FString&, ResponseContent,    // Raw response body
    int32, StatusCode,                   // HTTP status code
    bool, bWasSuccessful,                // Network success flag
    const FJsonObjectWrapper&, JsonObject // Parsed JSON response
);
```

### EInvoHttpMethod

Enumeration of supported HTTP methods.

```cpp
UENUM(BlueprintType)
enum class EInvoHttpMethod : uint8
{
    GET     UMETA(DisplayName = "GET"),
    POST    UMETA(DisplayName = "POST"),
    PUT     UMETA(DisplayName = "PUT"),
    DELETE  UMETA(DisplayName = "DELETE")
};
```

### UInvoSDKJsonHelpers

Blueprint function library for parsing JSON responses.

#### GetStringField

```cpp
UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
static FString GetStringField(
    const FJsonObjectWrapper& JsonWrapper,
    const FString& FieldName
);
```

#### GetIntegerField

```cpp
UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
static int32 GetIntegerField(
    const FJsonObjectWrapper& JsonWrapper,
    const FString& FieldName
);
```

#### GetBoolField

```cpp
UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
static bool GetBoolField(
    const FJsonObjectWrapper& JsonWrapper,
    const FString& FieldName
);
```

#### GetObjectsArray

```cpp
UFUNCTION(BlueprintPure, Category = "InvoSDK|JSON")
static TArray<FJsonObjectWrapper> GetObjectsArray(
    const FJsonObjectWrapper& JsonWrapper,
    const FString& FieldName
);
```

---

## Hosted approval on mobile (system browser)

Transfers, sends and claims can be approved with a passkey on **Invo's hosted approval page** instead of an SMS PIN. On iOS and Android the plugin opens that page in the **system browser** and tells you when the player is back. Nothing in the plugin can move money: settlement happens on your server, with a value the client never sees.

### Two halves

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

### First-time phones: no prompt on mobile

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

### Registering the return scheme

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

### Native bridges (optional)

The plugin opens the page with `FPlatformProcess::LaunchURL`: Safari on iOS, the default browser on Android. Both run WebAuthn. An `ASWebAuthenticationSession` (iOS) or Custom Tabs (Android) bridge gives a sheet that closes itself on return; the plugin does not ship one. If you add one, keep the contract: open only `https`, read nothing from the callback URL, and call `NotifyApprovalPageFinished()` when the sheet closes.

### Tests

`Source/InvoSDK/Private/Tests/InvoHostedApprovalTests.cpp` — Session Frontend > Automation, filter `InvoSDK.HostedApproval`, or:

```
UnrealEditor-Cmd.exe <Project>.uproject -ExecCmds="Automation RunTests InvoSDK.HostedApproval" -unattended -nopause -NullRHI
```

---

## Common Integration Patterns

### Authentication

Your game authenticates with its **SDK key** (`sdkKey` in `InvoSDKConfig`), sent as the `X-Game-Secret-Key` header. Calls made for one player can instead carry a short-lived player token (`Authorization: Bearer <token>`) that your server mints with `POST /api/sdk/player-token`. There is no login step and no CSRF token: players never sign in to Invo from your game.

`/auth/login` is the Invo developer console's own sign-in. It needs a browser check and a second step, so a game or script cannot use it. Do not call it from your game.

Keep the SDK key on your server in production builds (see the security warning under **Configure the SDK**).

### Example: Item Purchase Flow (C++)

```cpp
void AStoreManager::PurchaseItem(const FString& ItemId, int32 Quantity)
{
    // Build request body
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("itemId"), ItemId);
    JsonBody->SetNumberField(TEXT("quantity"), Quantity);

    FString BodyString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

    // Configure headers
    TMap<FString, FString> Headers;
    Headers.Add(TEXT("X-Game-Secret-Key"), SdkKey);   // from your server, never shipped in the client

    // Make request
    UInvoSDKApiManager* Request = UInvoSDKApiManager::InvoRequest(
        this,
        TEXT("https://sandbox.invo.network"),
        TEXT("/sandbox/api/store/purchase"),
        Headers,
        BodyString,
        EInvoHttpMethod::POST
    );

    Request->OnSuccess.AddDynamic(this, &AStoreManager::OnPurchaseSuccess);
    Request->OnFail.AddDynamic(this, &AStoreManager::OnPurchaseFailed);
    Request->Activate();
}
```

---

## Troubleshooting

### Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| "Config not found!" | `InvoSDKConfig` asset missing or not loaded | Ensure the config asset exists in your project and is properly referenced |
| "Player email not set!" | Empty `playerEmail` in config | Set the player's email in `InvoSDKConfig` or pass it directly to API calls |
| Request timeout | Network issues or incorrect endpoint | Verify network connectivity and endpoint URLs |
| 401 Unauthorized | Missing or wrong SDK key, or an expired player token | Check the `X-Game-Secret-Key` header (a test key on sandbox, a live key in production); mint a new player token |
| 403 Forbidden | The game or account cannot use this call | Check the game is active and the account is in good standing in the console |
| JSON parse errors | Malformed response | Check API endpoint and request format |

### Debug Logging

InvoSDK logs all requests and responses. Enable verbose logging in your project settings:

1. Open `DefaultEngine.ini`
2. Add the following:

```ini
[Core.Log]
LogTemp=Verbose
```

Check the Output Log in Unreal Editor for `InvoSDK Request Success` and `InvoSDK Request Failed` messages.

### Network Diagnostics

To verify connectivity to Invo servers:

```cpp
// Test request to verify API connectivity
UInvoSDKApiManager* TestRequest = UInvoSDKApiManager::InvoRequest(
    this,
    TEXT("https://sandbox.invo.network"),
    TEXT("/sandbox/api/health"),
    TMap<FString, FString>(),
    TEXT(""),
    EInvoHttpMethod::GET
);
TestRequest->OnSuccess.AddDynamic(this, &AMyActor::OnConnected);
TestRequest->OnFail.AddDynamic(this, &AMyActor::OnConnectionFailed);
TestRequest->Activate();
```

---

## Best Practices

### Security

1. **Never expose API keys in client builds** — Use server-side authentication
2. **Always use HTTPS** — The SDK enforces this by default
3. **Implement token refresh** — Access tokens expire; implement automatic refresh
4. **Validate on server** — Never trust client-side validation alone

### Performance

1. **Cache responses** — Store frequently-accessed data (catalogs, player info)
2. **Batch requests** — Combine multiple API calls when possible
3. **Handle offline gracefully** — Implement retry logic and offline queuing

### User Experience

1. **Show loading states** — Indicate when API calls are in progress
2. **Handle errors gracefully** — Display user-friendly error messages
3. **Implement optimistic UI** — Update UI immediately, reconcile with server

---

## Migration Guide

### From Previous Versions

If upgrading from an earlier version of InvoSDK:

1. Back up your project
2. Remove the old `InvoSDK` plugin folder
3. Install the new version following the [Installation](#installation) steps
4. Update any deprecated API calls (see changelog)
5. Regenerate project files and rebuild

---

## Support

### Documentation

- [Invo Platform Documentation](https://docs.invo.network)
- [API Reference](https://docs.invo.network/api)
- [Blueprint Tutorials](https://docs.invo.network/unreal/blueprints)

### Developer Resources

- [Developer Console](https://console.invo.network)
- [Sandbox Console](https://dev.console.invo.network)
- [Sample Projects](https://github.com/invo-network/unreal-samples)

### Getting Help

- **Email**: support@invo.network
- **Discord**: [Invo Developer Community](https://discord.gg/TDRxuVTU)
- **GitHub Issues**: Report bugs and feature requests

---

## Changelog

### Unreleased

- **Hosted approval on mobile.** `UInvoHostedApprovalSubsystem` opens Invo's hosted approval page in the system browser (never a WebView), fires `OnApprovalPageFinished` when the game returns on `invo-sdk-<GameId>://done` (the URL carries nothing; the game polls its own server from `begin` onwards), and offers the match-code prompt on a Slate overlay or your own UMG for non-phone screens (mobile `app_browser` enrolment is auto-confirmed, no prompt). `UInvoHostedApprovalLibrary` has the pure helpers and JSON parsers; `UInvoSDKSettings` adds **Project Settings > Plugins > Invo SDK > GameId**; `InvoSDK_Android_UPL.xml` registers the Android intent-filter at package time. Automation tests under `InvoSDK.HostedApproval`. New module dependency: `DeveloperSettings`.

### Version 1.0.0

- Initial release
- Core API manager with async HTTP support
- Blueprint integration for all API calls
- JSON helper utilities
- Pre-built UI components
- Sandbox and production environment support

---

## License

This SDK is proprietary software. See [LICENSE](LICENSE) for terms and conditions.

---

<p align="center">
  <sub>Built with ❤️ by the Invo Team</sub>
</p>
