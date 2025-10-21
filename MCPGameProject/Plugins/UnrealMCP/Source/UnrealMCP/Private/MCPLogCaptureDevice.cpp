// Copyright Epic Games, Inc. All Rights Reserved.

#include "MCPLogCaptureDevice.h"

FMCPLogCaptureDevice::FMCPLogCaptureDevice(int32 InMaxEntries)
    : MaxEntries(InMaxEntries)
    , WriteIndex(0)
    , bHasWrapped(false)
{
    LogEntries.Reserve(MaxEntries);
}

FMCPLogCaptureDevice::~FMCPLogCaptureDevice()
{
}

void FMCPLogCaptureDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
    Serialize(V, Verbosity, Category, -1.0);
}

void FMCPLogCaptureDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, const double Time)
{
    FScopeLock Lock(&CriticalSection);

    // Create new log entry
    FMCPLogEntry Entry(V, Category, Verbosity, Time);

    // Add to circular buffer
    if (LogEntries.Num() < MaxEntries)
    {
        // Still filling the initial buffer
        LogEntries.Add(Entry);
    }
    else
    {
        // Buffer is full, overwrite oldest entry
        LogEntries[WriteIndex] = Entry;
        bHasWrapped = true;
    }

    // Move write index forward (circular)
    WriteIndex = (WriteIndex + 1) % MaxEntries;
}

void FMCPLogCaptureDevice::GetLogEntries(TArray<FMCPLogEntry>& OutEntries, int32 MaxEntriesToReturn, const FString& SeverityFilter, const FString& CategoryFilter) const
{
    FScopeLock Lock(&CriticalSection);

    OutEntries.Empty();

    if (LogEntries.Num() == 0)
    {
        return;
    }

    // Determine the reading order (oldest to newest)
    int32 StartIndex = 0;
    int32 NumEntries = LogEntries.Num();

    if (bHasWrapped)
    {
        // Buffer has wrapped, oldest entry is at WriteIndex
        StartIndex = WriteIndex;
    }

    // Read entries in chronological order
    int32 EntriesReturned = 0;
    for (int32 i = 0; i < NumEntries && EntriesReturned < MaxEntriesToReturn; ++i)
    {
        int32 Index = (StartIndex + i) % NumEntries;
        const FMCPLogEntry& Entry = LogEntries[Index];

        // Apply category filter
        if (!CategoryFilter.IsEmpty() && !Entry.Category.Contains(CategoryFilter))
        {
            continue;
        }

        // Apply severity filter
        if (SeverityFilter != TEXT("All"))
        {
            if (SeverityFilter == TEXT("Error"))
            {
                if (Entry.Verbosity != ELogVerbosity::Error && Entry.Verbosity != ELogVerbosity::Fatal)
                {
                    continue;
                }
            }
            else if (SeverityFilter == TEXT("Warning"))
            {
                if (Entry.Verbosity != ELogVerbosity::Error && 
                    Entry.Verbosity != ELogVerbosity::Fatal && 
                    Entry.Verbosity != ELogVerbosity::Warning)
                {
                    continue;
                }
            }
            else if (SeverityFilter == TEXT("Display"))
            {
                if (Entry.Verbosity != ELogVerbosity::Error && 
                    Entry.Verbosity != ELogVerbosity::Fatal && 
                    Entry.Verbosity != ELogVerbosity::Warning &&
                    Entry.Verbosity != ELogVerbosity::Display &&
                    Entry.Verbosity != ELogVerbosity::Log)
                {
                    continue;
                }
            }
        }

        OutEntries.Add(Entry);
        EntriesReturned++;
    }
}
