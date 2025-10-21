#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMCPLogCaptureDevice;

class FUnrealMCPModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static inline FUnrealMCPModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FUnrealMCPModule>("UnrealMCP");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UnrealMCP");
	}

	/** Get the log capture device */
	FMCPLogCaptureDevice* GetLogCaptureDevice() const { return LogCaptureDevice.Get(); }

private:
	/** Log capture device for console output */
	TUniquePtr<FMCPLogCaptureDevice> LogCaptureDevice;
}; 