#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Built-in match-code prompt: a dimmed full-viewport overlay with the copy and one or two
 * buttons. Slate-only on purpose, so it works with no UMG asset. Replace it by setting
 * UInvoHostedApprovalSubsystem::bUseBuiltInPrompt = false and binding OnEnrollmentPromptRequested.
 */
class SInvoEnrollmentPrompt : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SInvoEnrollmentPrompt) {}
        SLATE_ARGUMENT(FText, Message)
        /** Empty = single-button mode (the "Finishing on…" state). */
        SLATE_ARGUMENT(FText, PrimaryLabel)
        SLATE_ARGUMENT(FText, SecondaryLabel)
        SLATE_EVENT(FSimpleDelegate, OnPrimary)
        SLATE_EVENT(FSimpleDelegate, OnSecondary)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    FSimpleDelegate OnPrimary;
    FSimpleDelegate OnSecondary;

    FReply HandlePrimary();
    FReply HandleSecondary();
};
