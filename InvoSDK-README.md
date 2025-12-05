
# InvoSDK Unreal Plugin

## Overview

InvoSDK is an Unreal Engine plugin that integrates with the Invo platform for in-game currency management, item purchases, and player-to-player transfers. The plugin wraps the Invo HTTP API, making it easy to call API endpoints, handle responses, and interact with in-game currency and items. The SDK uses C++ for backend functionality, exposed to Unreal's Blueprint system for ease of integration.

This README covers the plugin setup, API usage, available features, and the configuration system, all tailored for Unreal Engine.

## Environments (Important)

Invo exposes distinct environments for production and sandbox testing. **Always use the sandbox environment for testing purposes**.

- **Production (Live games only)**:
    - Host: `https://invo.network`
    - Routes: `/api/*` and `/auth/*`
    - Console: `https://console.invo.network`

- **Sandbox (Testing)**:
    - Host: `https://sandbox.invo.network`
    - Routes: `/sandbox/api/*` and `/sandbox/auth/*`
    - Console: `https://dev.console.invo.network`

**Quick Sandbox Example:**
1. **Login**: `POST https://sandbox.invo.network/sandbox/auth/login` with JSON body `{ "email": "...", "password": "..." }`
2. **Get CSRF Token**: `GET https://sandbox.invo.network/sandbox/auth/csrf-token`
3. **Make API Call**: Use the access token and CSRF token when making protected API calls.

## Key Features

- **Player Balance Management**: Retrieve and manage player balances.
- **Game Item Catalog**: Fetch and display the in-game item catalog.
- **Item Purchases**: Use `PurchaseItemAsync` for item purchases.
- **Currency Transfers**: Initiate currency transfers and validate SMS for verification.
- **Sprite Loading**: Helper function for async loading of sprites from remote URLs.
- **Editor Setup Wizard**: Auto-creates necessary config assets upon first import.
- **UI Components**: Example UI panels to quickly integrate InvoSDK flows into your game.

## Installation

1. Copy the `InvoSDK` folder and all associated files into your Unreal project's `Plugins` directory.
2. The setup wizard will create a `InvoSDKConfig` asset during the first import and open the configuration panel.

## Runtime Configuration (`InvoSDKConfig`)

The plugin uses a `UDataAsset` or `UStruct` in Unreal to store configuration data (similar to Unity's `ScriptableObject` approach). The configuration holds sensitive information like your API key, game ID, and player details.

Key fields:
- **sdkKey**: Your Invo API secret key (do not ship in client builds).
- **gameId**: Unique identifier for your game.
- **gameName**: Your game’s name.
- **playerName**: Player's name.
- **gameIconUrl**: URL to your game's icon.
- **gameCurrencyName**: Name of the game currency.
- **gameCurrencyUrl**: URL for the game currency image.
- **gameVersion**: Game version.
- **playerEmail**: Player’s email address.
- **useProduction**: Boolean to toggle between production and sandbox environments.

## Blueprint Integration

The **API Manager** is exposed as a Blueprint class that acts as a bridge for calling Invo API endpoints. This is set up using a C++ `UInvoSDKApiManager` class that extends `UBlueprintAsyncActionBase`, allowing asynchronous HTTP requests from Blueprints.

Key API calls are made through Blueprint nodes, such as:
- `InvoRequest`: Handles HTTP requests with parameters like method type (GET, POST, etc.), endpoint, and headers.
- Blueprint events for success or failure (`OnSuccess`, `OnFail`), triggered based on HTTP request results.

## Example Blueprint Usage

1. **Player Balance Fetch**:
   - Call `GetPlayerBalanceAsync` from your API manager in Blueprints.
   - Bind the response to handle the player's balance.

2. **Item Purchase**:
   - Use `PurchaseItemAsync` in Blueprints to handle item purchases.

```cpp
UInvoSDKApiManager::InvoRequest(
    WorldContextObject,
    BaseUrl,
    Endpoint,
    Headers,
    Body,
    EInvoHttpMethod::POST
);
```

## C++ Integration (InvoSDKApiManager)

The core of the SDK's API functionality resides in the `InvoSDKApiManager` class, which makes HTTP requests using Unreal's HTTP module. Here's a quick breakdown:

- **InvoRequest**: A static function that sets up and returns an instance of the API manager to perform requests.
- **Activate**: This method handles the actual HTTP request and processes the response, triggering success or failure callbacks.

Example of an HTTP request handler:

```cpp
void UInvoSDKApiManager::Activate()
{
    // Set up the request
    const FString FullUrl = RequestBaseUrl + RequestEndpoint;
    // Call the API with method (POST/GET) and parameters
}
```

## Troubleshooting

- **"Config not found!"**: Ensure the `InvoSDKConfig` asset is correctly placed and loaded in your project.
- **"Player email not set!"**: Ensure player email is populated in the config or passed directly to API calls.
- **Missing Dependencies**: Make sure to include all necessary libraries, such as `Json` for serialization.

## Where to Look in the Repository

- **`InvoSDKApiManager.h` & `InvoSDKApiManager.cpp`**: Core API handling and request management.
- **`InvoSDKJsonHelpers.h`**: Helper functions for JSON serialization and deserialization in Blueprints.
- **`InvoSDKConfig` Asset**: Holds the configuration for your Invo API integration.

For detailed API references, consult the Invo platform documentation [here](https://docs.invo.network).
