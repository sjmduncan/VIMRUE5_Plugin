// Copyright Epic Games, Inc. All Rights Reserved.

#include "VIMRUE5.h"
#include "Core.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(VIMRLog);
DEFINE_LOG_CATEGORY(VIMRLogInternal);

#define LOCTEXT_NAMESPACE "FVIMRUE5Module"

void FVIMRUE5Module::StartupModule()
{
	const FString VIMR_ROOT_ENV_VAR = "VIMR_ROOT_09";

	FString BaseDir = IPluginManager::Get().FindPlugin("VIMRUE5")->GetBaseDir();
  const FString VIMR_ROOT = FPlatformMisc::GetEnvironmentVariable(*VIMR_ROOT_ENV_VAR);


	if(VIMR_ROOT.IsEmpty())
	{
	  UE_LOG(VIMRLog, Error, TEXT("Environment variable '%s' not found"), *VIMR_ROOT_ENV_VAR);
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "VIMR environment variable not found"));
		return;
	}

	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*VIMR_ROOT, TEXT("bin\\vimr.dll"));
#endif // PLATFORM_WINDOWS

	UE_LOG(VIMRLog, Log, TEXT("%s: %s"), *VIMR_ROOT_ENV_VAR, *VIMR_ROOT);

	if(!FPaths::FileExists(*LibraryPath))
	{
	  UE_LOG(VIMRLog, Error, TEXT("'%s' does not exist"), *LibraryPath);
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "VIMR library not found"));
		return;
	}

  UE_LOG(VIMRLog, Log, TEXT("VIMR library: %s"), *LibraryPath);

  VIMRLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!VIMRLibraryHandle)
	{
		UE_LOG(VIMRLog, Error, TEXT("Failed to load '%s'"), *LibraryPath);
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load VIMR library"));
		return;
	}
	UE_LOG(VIMRLog, Log, TEXT("Succesfully loaded VIMR"));
}

void FVIMRUE5Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(VIMRLibraryHandle);
	VIMRLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVIMRUE5Module, VIMRUE5)
