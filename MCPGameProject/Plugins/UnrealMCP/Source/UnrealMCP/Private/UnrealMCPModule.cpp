#include "UnrealMCPModule.h"
#include "UnrealMCPBridge.h"
#include "MCPLogCaptureDevice.h"
#include "Modules/ModuleManager.h"
#include "EditorSubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FUnrealMCPModule"

void FUnrealMCPModule::StartupModule()
{
	UE_LOG(LogTemp, Display, TEXT("Unreal MCP Module has started"));

	// Create and register the log capture device
	LogCaptureDevice = MakeUnique<FMCPLogCaptureDevice>(1000); // Keep last 1000 log entries
	
	if (GLog && LogCaptureDevice.IsValid())
	{
		GLog->AddOutputDevice(LogCaptureDevice.Get());
		UE_LOG(LogTemp, Display, TEXT("MCP Log Capture Device registered - capturing console output"));
	}
}

void FUnrealMCPModule::ShutdownModule()
{
	// Unregister the log capture device
	if (GLog && LogCaptureDevice.IsValid())
	{
		GLog->RemoveOutputDevice(LogCaptureDevice.Get());
		UE_LOG(LogTemp, Display, TEXT("MCP Log Capture Device unregistered"));
	}
	
	LogCaptureDevice.Reset();
	
	UE_LOG(LogTemp, Display, TEXT("Unreal MCP Module has shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealMCPModule, UnrealMCP) 