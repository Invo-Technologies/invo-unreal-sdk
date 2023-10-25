// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"

class SEditableTextBox;
/**
 * 
 */
class  SKeyInputDialog : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SKeyInputDialog) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnOkClicked();
	FReply OnTogglePinMask();

	void CloseSKeyInputDialogWidget() const;
   

private:
    TSharedPtr<SEditableTextBox> TextBox;

	FText GetPinMaskButtonText() const;
	bool isPinedMasked() const;
	TSharedPtr<SEditableTextBox> PinTextBox;
	bool bIsPinMasked = true;


	
};
