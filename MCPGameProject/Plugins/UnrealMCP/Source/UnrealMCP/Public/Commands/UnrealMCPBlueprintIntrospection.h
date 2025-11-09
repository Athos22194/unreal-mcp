#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Command handler for Blueprint introspection operations.
 * Extracts complete Blueprint data including metadata, components, variables,
 * functions, event graphs, and node connections.
 */
class UNREALMCP_API FUnrealMCPBlueprintIntrospection
{
public:
    FUnrealMCPBlueprintIntrospection();
    
    /**
     * Main command dispatcher
     */
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);
    
private:
    /**
     * Get complete Blueprint data
     */
    TSharedPtr<FJsonObject> HandleGetBlueprintData(const TSharedPtr<FJsonObject>& Params);
    
    /**
     * Extract basic Blueprint information
     */
    TSharedPtr<FJsonObject> ExtractBlueprintInfo(class UBlueprint* Blueprint);
    
    /**
     * Extract component hierarchy
     */
    TArray<TSharedPtr<FJsonValue>> ExtractComponents(class UBlueprint* Blueprint);
    
    /**
     * Extract Blueprint variables
     */
    TArray<TSharedPtr<FJsonValue>> ExtractVariables(class UBlueprint* Blueprint);
    
    /**
     * Extract Blueprint functions (Phase 4)
     */
    TArray<TSharedPtr<FJsonValue>> ExtractFunctions(class UBlueprint* Blueprint);
    
    /**
     * Extract graph node and connection data (Phase 5)
     */
    TSharedPtr<FJsonObject> ExtractGraphData(class UEdGraph* Graph);
};
