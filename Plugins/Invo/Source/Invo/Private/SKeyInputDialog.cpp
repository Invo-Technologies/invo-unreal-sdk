// Fill out your copyright notice in the Description page of Project Settings

#include "SKeyInputDialog.h"

#include "InvoFunctions.h"
#include "InvoHttpManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"

#include "Runtime/Online/HTTP/Public/Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpRequest.h"
#include "Runtime/Online/HTTP/Public/Interfaces/IHttpResponse.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/Misc/MessageDialog.h"
#include "Runtime/SlateCore/Public/Widgets/SWidget.h"


void SKeyInputDialog::Construct(const FArguments& InArgs)
{
    ChildSlot
        [
            SNew(SBox)

                [
                    SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        .HAlign(HAlign_Left)
                        .Padding(2.0f)
                        [
                            SNew(SHorizontalBox)
                                // Group for "Game ID"
                                + SHorizontalBox::Slot()
                                .FillWidth(0.25f)  // This ensures each group takes up 25% of the available width
                                .Padding(5.0f)
                                [
                                    SNew(SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Enter S Key Pin:"))
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SAssignNew(PinTextBox, SEditableTextBox)
                                                .HintText(FText::FromString("Enter 6 Digit Pin"))
                                                .MinDesiredWidth(200.0f) // This sets the minimum width
                                                .IsPassword(true)


                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(5.0f)
                                        [
                                            SNew(SButton)
                                                .Text(this, &SKeyInputDialog::GetPinMaskButtonText)
                                                .OnClicked(this, &SKeyInputDialog::OnTogglePinMask)

                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(SButton)
                                                .Text(FText::FromString(TEXT("OK")))
                                                .OnClicked(this, &SKeyInputDialog::OnOkClicked)
                                        ]

                                ]
                        ]
                ]
        ];
}


FReply SKeyInputDialog:: OnOkClicked()
{
    FString Key = PinTextBox->GetText().ToString();
    // Use the key as needed
    // ...

    FString EncryptDataAuthCode = UInvoFunctions::EncryptData("SomeKey", Key);
    UE_LOG(LogTemp,Warning,TEXT("EncryptDataAuthCode is  %s"), *EncryptDataAuthCode)
    UInvoFunctions::UpdateSecretsIni("SKeyCode", EncryptDataAuthCode);
    // FString DecryptDataAuthCode = UInvoFunctions::DecryptData(EncryptDataAuthCode, Key,"SKeyCode");
    //UE_LOG(LogTemp, Warning, TEXT("DecryptDataAuthCode is ---  %s"), *DecryptDataAuthCode)

    CloseSKeyInputDialogWidget();
    
    // Close the dialog
    return FReply::Handled();
}

void SKeyInputDialog::CloseSKeyInputDialogWidget() const
{
    // Find and close the parent window of this widget
    TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
    if (ParentWindow.IsValid())
    {
        ParentWindow->RequestDestroyWindow();
    }
}

bool SKeyInputDialog::isPinedMasked() const
{
    return bIsPinMasked;
}

FText SKeyInputDialog::GetPinMaskButtonText() const
{
    return bIsPinMasked ? FText::FromString("Unmask") : FText::FromString("Mask");
}


FReply SKeyInputDialog::OnTogglePinMask()
{
    bIsPinMasked = !bIsPinMasked;
    if (PinTextBox.IsValid())
    {
        PinTextBox->SetIsPassword(bIsPinMasked);
    }
    return FReply::Handled();
}