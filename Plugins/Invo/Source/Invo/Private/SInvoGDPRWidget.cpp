// Fill out your copyright notice in the Description page of Project Settings.

#include "SInvoGDPRWidget.h"
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


void SInvoGDPRWidget::Construct(const FArguments& InArgs)
{
    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString("GDPR Compliance Actions"))
                        .WrapTextAt(800)
                        .AutoWrapText(true)
                        .Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 32))
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .Padding(10)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString("You have the right to request or delete your personal data. Use the buttons below to perform these actions."))
                        .WrapTextAt(800)
                        .AutoWrapText(true)
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .VAlign(VAlign_Bottom)
                .HAlign(HAlign_Center)
                .Padding(10.0f)
                [
                    SNew(SHorizontalBox)

                        // Submit Button
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(5.0f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString("Request Data"))
                                .OnClicked(this, &SInvoGDPRWidget::OnRequestDataClicked)
                        ]

                        // Close Button
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(5.0f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString("Delete Data"))
                                .OnClicked(this, &SInvoGDPRWidget::OnDeleteDataClicked)
                        ]
                ]
                
        ];
}

FReply SInvoGDPRWidget::OnRequestDataClicked()
{
    // Handle the request data logic here
    return FReply::Handled();
}

FReply SInvoGDPRWidget::OnDeleteDataClicked()
{
    // Handle the delete data logic here
    return FReply::Handled();
}
