#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


class XPLUGIN_API FXPluginImplBase
{
public:
	virtual ~FXPluginImplBase() {}
	virtual void TestXPlugin();
};
