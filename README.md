<p align="center">
  <img src="https://invo.network/logo.png" alt="InvoSDK Logo" width="200"/>
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

## Common Integration Patterns

### Authentication Flow

```
┌─────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  User   │     │  Your Game  │     │  InvoSDK    │     │  Invo API   │
└────┬────┘     └──────┬──────┘     └──────┬──────┘     └──────┬──────┘
     │                 │                   │                   │
     │  Login Request  │                   │                   │
     │────────────────▶│                   │                   │
     │                 │                   │                   │
     │                 │  POST /auth/login │                   │
     │                 │──────────────────▶│                   │
     │                 │                   │                   │
     │                 │                   │  HTTPS Request    │
     │                 │                   │──────────────────▶│
     │                 │                   │                   │
     │                 │                   │  Access Token     │
     │                 │                   │◀──────────────────│
     │                 │                   │                   │
     │                 │  OnSuccess        │                   │
     │                 │◀──────────────────│                   │
     │                 │                   │                   │
     │                 │  GET /auth/csrf   │                   │
     │                 │──────────────────▶│                   │
     │                 │                   │──────────────────▶│
     │                 │                   │◀──────────────────│
     │                 │◀──────────────────│                   │
     │                 │                   │                   │
     │  Login Success  │                   │                   │
     │◀────────────────│                   │                   │
     │                 │                   │                   │
```

### Example: Complete Login Flow (Blueprint)

1. **Login Request**
   - Endpoint: `POST /sandbox/auth/login`
   - Body: `{"email": "player@example.com", "password": "..."}`

2. **Get CSRF Token**
   - Endpoint: `GET /sandbox/auth/csrf-token`
   - Store the token for subsequent requests

3. **Make Authenticated Requests**
   - Include `Authorization: Bearer <access_token>` header
   - Include `X-CSRF-Token: <csrf_token>` header for mutations

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
    Headers.Add(TEXT("Authorization"), TEXT("Bearer ") + AccessToken);
    Headers.Add(TEXT("X-CSRF-Token"), CSRFToken);

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
| 401 Unauthorized | Invalid or expired token | Re-authenticate and obtain a fresh access token |
| 403 Forbidden | Missing CSRF token or permissions | Include CSRF token in headers; verify API permissions |
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
- **Discord**: [Invo Developer Community](https://discord.gg/invo)
- **GitHub Issues**: Report bugs and feature requests

---

## Changelog

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
