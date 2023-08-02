// Fill out your copyright notice in the Description page of Project Settings.


#include "InvoWebBrowserWidge.h"

//#include "WebBrowser.h"
void UInvoWebBrowserWidge::NativeConstruct()
{
    Super::NativeConstruct();

    UWebBrowser* WebBrowser = NewObject<UWebBrowser>(this);

    if (WebBrowser)
    {
        WebBrowser->LoadURL(FString("https://www.google.com"));
    }
}