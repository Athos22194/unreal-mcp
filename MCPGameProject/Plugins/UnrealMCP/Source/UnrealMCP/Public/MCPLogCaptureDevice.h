// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"
#include "HAL/CriticalSection.h"

/**
 * Log entry structure for captured logs
 */
struct FMCPLogEntry
{
    FString Timestamp;
    FString Category;
    FString Severity;
    FString Message;
    double Time;
    ELogVerbosity::Type Verbosity;

    FMCPLogEntry()
        : Time(0.0)
        , Verbosity(ELogVerbosity::Log)
    {
    }

    FMCPLogEntry(const FString& InMessage, const FName& InCategory, ELogVerbosity::Type InVerbosity, double InTime)
        : Category(InCategory.ToString())
        , Message(InMessage)
        , Time(InTime)
        , Verbosity(InVerbosity)
    {
        // Set timestamp
        if (InTime > 0)
        {
            Timestamp = FDateTime::FromUnixTimestamp((int64)InTime).ToString();
        }
        else
        {
            Timestamp = FDateTime::Now().ToString();
        }

        // Set severity string
        switch (InVerbosity)
        {
            case ELogVerbosity::Fatal:
            case ELogVerbosity::Error:
                Severity = TEXT("Error");
                break;
            case ELogVerbosity::Warning:
                Severity = TEXT("Warning");
                break;
            case ELogVerbosity::Display:
            case ELogVerbosity::Log:
                Severity = TEXT("Display");
                break;
            default:
                Severity = TEXT("Verbose");
                break;
        }
    }
};

/**
 * Custom output device that maintains a circular buffer of recent log messages
 * Registered with GLog to capture all log output during editor session
 */
class UNREALMCP_API FMCPLogCaptureDevice : public FOutputDevice
{
public:
    FMCPLogCaptureDevice(int32 InMaxEntries = 1000);
    virtual ~FMCPLogCaptureDevice();

    // FOutputDevice interface
    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override;
    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, const double Time) override;
    virtual bool CanBeUsedOnAnyThread() const override { return true; }
    virtual bool CanBeUsedOnPanicThread() const override { return false; }

    /**
     * Get captured log entries with optional filtering
     * @param OutEntries Output array to fill with log entries
     * @param MaxEntries Maximum number of entries to return
     * @param SeverityFilter Filter by severity ("All", "Error", "Warning", "Display")
     * @param CategoryFilter Filter by category name (empty string for all)
     */
    void GetLogEntries(TArray<FMCPLogEntry>& OutEntries, int32 MaxEntries, const FString& SeverityFilter, const FString& CategoryFilter) const;

    /** Get the total number of captured entries */
    int32 GetTotalEntries() const { return LogEntries.Num(); }

private:
    /** Circular buffer of log entries */
    TArray<FMCPLogEntry> LogEntries;

    /** Maximum number of entries to keep in the buffer */
    int32 MaxEntries;

    /** Current write position in the circular buffer */
    int32 WriteIndex;

    /** Whether the buffer has wrapped around */
    bool bHasWrapped;

    /** Critical section for thread-safe access */
    mutable FCriticalSection CriticalSection;
};
