# Fab listing: INVO Network SDK (Unreal Engine)

Seller: Invonetwork (company account, not a personal name)
Product type: Code Plugin, Engine plugin
Price: Free
License: Standard Fab license

## Title
INVO Network SDK

## Short description (one line)
One virtual currency players buy once and spend across every INVO game: store, item purchases, sends, transfers and phone approval for Unreal Engine 5.

## Description
The official Unreal Engine plugin for INVO Network (https://invo.network). INVO gives your players one virtual currency they can buy once and use across every INVO game. Sell currency packs, sell in-game items, and let players send currency to friends or move it to another game, with every send approved on the player's phone.

**What's included**
- **A C++ and Blueprint API** (`Invo Subsystem`): sign-in, balances, hosted checkout, item purchases, sends, transfers, pending collect, claim codes and phone changes. Every call answers with an `FInvoResult` carrying INVO's stable error codes, plus Blueprint error checks such as Is Insufficient Balance and Is Rate Limited.
- **Phone approval.** Sends and transfers are approved on the player's phone. On desktop the player scans a QR code; on mobile the approval page opens in the system browser and returns to the game through the `invo-sdk-<GameId>` link. A built-in Slate prompt handles the first-time "is this your phone?" match code.
- **Project Settings > Plugins > Invo SDK**: sandbox and production game IDs, game server URL, token and checkout endpoints, and display settings.
- **A ready-made wallet**: Store, Send, Transfer, Collect and phone screens drawn in Slate, opened with one Blueprint node. The demo map runs every screen offline with demo data, no account needed.
- **Automation tests** under `InvoSDK.*` in the Session Frontend.

**Security**
Your Platform API key never ships in a build. In production the game holds only a short-lived player token, and anything that needs the key goes through your own game server, the same way INVO's Unity and Godot SDKs work. Direct sandbox mode reads the key from your local settings or an environment variable, refuses production keys, and is disabled in Shipping builds.

**Get started**
Get free sandbox keys in Test mode at console.invo.network/test, then follow the guide at docs.invo.network.

## Technical details
- Features: C++ and Blueprint API, hosted checkout, item purchases, player-to-player sends, cross-game transfers, phone (passkey) approval with QR code and mobile deep link, sandbox and production environments
- Code modules: InvoSDK (Runtime)
- Number of Blueprints: 0 (the wallet is C++/Slate; one demo map)
- Number of C++ classes: UInvoSubsystem, UInvoLibrary, UInvoHostedApprovalSubsystem, UInvoHostedApprovalLibrary, UInvoSDKSettings, UInvoSDKLocalSettings, UInvoWalletSubsystem, AInvoWalletDemoActor, UInvoQrCodeLibrary, UInvoSDKApiManager
- Network replicated: No (client and your own game server)
- Supported development platforms: Windows
- Supported target build platforms: Windows, Android, iOS
- Documentation: https://docs.invo.network
- Example project: Content/Map/InvoWalletDemo (works offline)
- Important notes: Requires an INVO account (free sandbox at console.invo.network/test). Real-money top-ups use INVO's hosted checkout in the system browser.

## Tags
currency, virtual currency, payments, monetization, in-game store, wallet, cross-game, blueprint, sdk, invo

## Media
- Thumbnail and gallery: reuse the INVO wallet screenshots (store, items, send, QR approval, collect) at 1920x1080
- Video: https://www.youtube.com/watch?v=ePrcYXZZG6k (same INVO demo as the Unity and Godot listings)
