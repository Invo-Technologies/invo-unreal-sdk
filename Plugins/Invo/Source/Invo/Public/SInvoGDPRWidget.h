// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "UObject/NoExportTypes.h"


class INVO_API SInvoGDPRWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoGDPRWidget){}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Callback when the "Delete Data" button is clicked
    FReply OnDeleteDataClicked();

    // Callback when the "Request Data" button is clicked
    FReply OnRequestDataClicked();
};
