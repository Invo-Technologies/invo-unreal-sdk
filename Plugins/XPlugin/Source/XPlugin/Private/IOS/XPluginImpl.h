#pragma once

#include "XPluginImplBase.h"
#import <Foundation/Foundation.h>
#include "Misc/CoreDelegates.h"
#import <UIKit/UIkit.h>

class XPLUGIN_API FXPluginImpl : public FXPluginImplBase
{
public:
	~FXPluginImpl() override {}
	void TestXPlugin() override;
};
