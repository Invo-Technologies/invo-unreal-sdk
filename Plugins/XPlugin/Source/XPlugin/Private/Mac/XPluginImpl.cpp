#include "XPluginImpl.h"
#include "Engine/Engine.h"

void FXPluginImpl::TestXPlugin()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("TextPlugin launched from Mac Platform"));
}
