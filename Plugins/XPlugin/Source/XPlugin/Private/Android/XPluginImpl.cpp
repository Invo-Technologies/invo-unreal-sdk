#include "XPluginImpl.h"
#include "Engine/Engine.h"


#if PLATFORM_ANDROID


//#include "Android/AndroidJNI.h"
//#include "AndroidApplication.h"

#endif

void FXPluginImpl::InitJavaFunctions()
{
    
    GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, TEXT("Java Android"));
}

void FXPluginImpl::TestXPlugin()
{
    
	GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, TEXT("TextPlugin launched from Android Platform"));
    InitJavaFunctions();
}
