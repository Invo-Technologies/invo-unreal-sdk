#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvoQrCode.generated.h"

class UTexture2D;

/**
 * QR codes for phone approval on desktop: the player scans VerificationUriComplete with their phone.
 * Encoding is done in-process by InvoQrCodeCore.h (byte mode, medium error correction), so the
 * approval URL never goes to a third-party QR service.
 */
UCLASS()
class INVOSDK_API UInvoQrCodeLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** A transient texture of Text as a QR code (nearest filtering, so it stays sharp when scaled up).
     *  PixelsPerModule is clamped to 1-32 and QuietZoneModules to 0-16; scanners expect a quiet zone
     *  of at least 4. Returns null when Text is empty or longer than a QR code holds (about 2300 bytes). */
    UFUNCTION(BlueprintCallable, Category = "InvoSDK|Approval", meta = (AdvancedDisplay = "PixelsPerModule,QuietZoneModules,DarkColor,LightColor"))
    static UTexture2D* MakeQrCodeTexture(const FString& Text, int32 PixelsPerModule = 8, int32 QuietZoneModules = 4,
                                         FLinearColor DarkColor = FLinearColor::Black, FLinearColor LightColor = FLinearColor::White);

    /** The module grid of Text (UTF-8), row-major, true = dark, without a quiet zone. False when Text
     *  is empty or too long. For drawing the code yourself, e.g. in Slate or a material. */
    static bool EncodeQrModules(const FString& Text, int32& OutSize, TArray<bool>& OutModules);
};
