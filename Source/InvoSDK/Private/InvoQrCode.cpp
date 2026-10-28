#include "InvoQrCode.h"
#include "InvoQrCodeCore.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"

bool UInvoQrCodeLibrary::EncodeQrModules(const FString& Text, int32& OutSize, TArray<bool>& OutModules)
{
    OutSize = 0;
    OutModules.Reset();
    if (Text.IsEmpty())
    {
        return false;
    }

    const FTCHARToUTF8 Utf8(*Text);
    const uint8* Bytes = reinterpret_cast<const uint8*>(Utf8.Get());
    const std::vector<uint8_t> Data(Bytes, Bytes + Utf8.Length());

    InvoQr::FQrCode Code;
    if (!InvoQr::FQrCode::EncodeBytes(Data, InvoQr::EEcc::Medium, -1, Code))
    {
        return false;
    }

    OutSize = Code.GetSize();
    OutModules.SetNumUninitialized(OutSize * OutSize);
    for (int32 Y = 0; Y < OutSize; ++Y)
    {
        for (int32 X = 0; X < OutSize; ++X)
        {
            OutModules[Y * OutSize + X] = Code.IsDark(X, Y);
        }
    }
    return true;
}

UTexture2D* UInvoQrCodeLibrary::MakeQrCodeTexture(const FString& Text, int32 PixelsPerModule, int32 QuietZoneModules,
                                                  FLinearColor DarkColor, FLinearColor LightColor)
{
    int32 Size = 0;
    TArray<bool> Modules;
    if (!EncodeQrModules(Text, Size, Modules))
    {
        return nullptr;
    }

    PixelsPerModule = FMath::Clamp(PixelsPerModule, 1, 32);
    QuietZoneModules = FMath::Clamp(QuietZoneModules, 0, 16);
    const int32 Side = (Size + QuietZoneModules * 2) * PixelsPerModule;

    UTexture2D* Texture = UTexture2D::CreateTransient(Side, Side, PF_B8G8R8A8);
    if (!Texture)
    {
        return nullptr;
    }
    Texture->Filter = TF_Nearest;
    Texture->SRGB = true;
    Texture->LODGroup = TEXTUREGROUP_UI;
    Texture->NeverStream = true;

    const FColor Dark = DarkColor.ToFColor(true);
    const FColor Light = LightColor.ToFColor(true);

    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
    FColor* Pixels = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
    for (int32 Py = 0; Py < Side; ++Py)
    {
        const int32 My = Py / PixelsPerModule - QuietZoneModules;
        for (int32 Px = 0; Px < Side; ++Px)
        {
            const int32 Mx = Px / PixelsPerModule - QuietZoneModules;
            const bool bDark = Mx >= 0 && Mx < Size && My >= 0 && My < Size && Modules[My * Size + Mx];
            Pixels[Py * Side + Px] = bDark ? Dark : Light;
        }
    }
    Mip.BulkData.Unlock();
    Texture->UpdateResource();
    return Texture;
}
