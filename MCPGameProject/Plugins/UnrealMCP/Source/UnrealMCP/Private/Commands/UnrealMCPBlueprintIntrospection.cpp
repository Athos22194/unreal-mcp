#include "Commands/UnrealMCPBlueprintIntrospection.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/FieldPath.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_CustomEvent.h"
#include "EdGraphSchema_K2.h"

FUnrealMCPBlueprintIntrospection::FUnrealMCPBlueprintIntrospection()
{
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintIntrospection::HandleCommand(
    const FString& CommandType, 
    const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("get_blueprint_data"))
    {
        return HandleGetBlueprintData(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(
        FString::Printf(TEXT("Unknown blueprint introspection command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintIntrospection::HandleGetBlueprintData(
    const TSharedPtr<FJsonObject>& Params)
{
    // Get blueprint name parameter
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    
    UE_LOG(LogTemp, Display, TEXT("Getting blueprint data for: %s"), *BlueprintName);
    
    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }
    
    // Create result object
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    
    // Extract basic Blueprint info
    TSharedPtr<FJsonObject> InfoObj = ExtractBlueprintInfo(Blueprint);
    Result->SetObjectField(TEXT("blueprint_info"), InfoObj);
    
    // Extract components
    TArray<TSharedPtr<FJsonValue>> ComponentsArray = ExtractComponents(Blueprint);
    Result->SetArrayField(TEXT("components"), ComponentsArray);
    
    // Extract variables
    TArray<TSharedPtr<FJsonValue>> VariablesArray = ExtractVariables(Blueprint);
    Result->SetArrayField(TEXT("variables"), VariablesArray);
    
    // Extract functions (Phase 4)
    TArray<TSharedPtr<FJsonValue>> FunctionsArray = ExtractFunctions(Blueprint);
    Result->SetArrayField(TEXT("functions"), FunctionsArray);
    
    // Extract event graphs (Phase 5)
    TArray<TSharedPtr<FJsonValue>> EventGraphsArray;
    
    UE_LOG(LogTemp, Warning, TEXT("ExtractEventGraphs: Blueprint=%s, UbergraphPages=%d"), 
        *Blueprint->GetName(), Blueprint->UbergraphPages.Num());
    
    // Main event graph
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (!Graph) continue;
        
        UE_LOG(LogTemp, Warning, TEXT("  Event Graph: %s, NumNodes=%d"), 
            *Graph->GetName(), Graph->Nodes.Num());
        
        TSharedPtr<FJsonObject> EventGraphObj = MakeShared<FJsonObject>();
        EventGraphObj->SetStringField(TEXT("name"), Graph->GetName());
        EventGraphObj->SetStringField(TEXT("type"), TEXT("event_graph"));
        
        TSharedPtr<FJsonObject> GraphData = ExtractGraphData(Graph);
        if (GraphData.IsValid())
        {
            EventGraphObj->SetObjectField(TEXT("graph"), GraphData);
        }
        
        EventGraphsArray.Add(MakeShared<FJsonValueObject>(EventGraphObj));
    }
    
    // Construction script (if it exists)
    if (Blueprint->SimpleConstructionScript)
    {
        // Find the UserConstructionScript graph
        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph && Graph->GetName() == TEXT("UserConstructionScript"))
            {
                TSharedPtr<FJsonObject> ConstructionGraphObj = MakeShared<FJsonObject>();
                ConstructionGraphObj->SetStringField(TEXT("name"), TEXT("UserConstructionScript"));
                ConstructionGraphObj->SetStringField(TEXT("type"), TEXT("construction_script"));
                
                TSharedPtr<FJsonObject> GraphData = ExtractGraphData(Graph);
                if (GraphData.IsValid())
                {
                    ConstructionGraphObj->SetObjectField(TEXT("graph"), GraphData);
                }
                
                EventGraphsArray.Add(MakeShared<FJsonValueObject>(ConstructionGraphObj));
                break;
            }
        }
    }
    
    Result->SetArrayField(TEXT("event_graphs"), EventGraphsArray);
    
    UE_LOG(LogTemp, Display, TEXT("Successfully extracted blueprint data"));
    
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintIntrospection::ExtractBlueprintInfo(UBlueprint* Blueprint)
{
    TSharedPtr<FJsonObject> InfoObj = MakeShared<FJsonObject>();
    
    // Basic info
    InfoObj->SetStringField(TEXT("name"), Blueprint->GetName());
    InfoObj->SetStringField(TEXT("path"), Blueprint->GetPathName());
    
    // Parent class
    if (Blueprint->ParentClass)
    {
        InfoObj->SetStringField(TEXT("parent_class"), Blueprint->ParentClass->GetName());
    }
    else
    {
        InfoObj->SetStringField(TEXT("parent_class"), TEXT("None"));
    }
    
    // Blueprint type
    FString BlueprintType = TEXT("Normal");
    switch (Blueprint->BlueprintType)
    {
        case BPTYPE_Normal:
            BlueprintType = TEXT("Normal");
            break;
        case BPTYPE_Const:
            BlueprintType = TEXT("Const");
            break;
        case BPTYPE_MacroLibrary:
            BlueprintType = TEXT("MacroLibrary");
            break;
        case BPTYPE_Interface:
            BlueprintType = TEXT("Interface");
            break;
        case BPTYPE_LevelScript:
            BlueprintType = TEXT("LevelScript");
            break;
        case BPTYPE_FunctionLibrary:
            BlueprintType = TEXT("FunctionLibrary");
            break;
    }
    InfoObj->SetStringField(TEXT("blueprint_type"), BlueprintType);
    
    // Description
    InfoObj->SetStringField(TEXT("description"), Blueprint->BlueprintDescription);
    
    // Category
    InfoObj->SetStringField(TEXT("category"), Blueprint->BlueprintCategory);
    
    // Package path
    if (Blueprint->GetPackage())
    {
        InfoObj->SetStringField(TEXT("package"), Blueprint->GetPackage()->GetName());
    }
    
    return InfoObj;
}

TArray<TSharedPtr<FJsonValue>> FUnrealMCPBlueprintIntrospection::ExtractComponents(UBlueprint* Blueprint)
{
    TArray<TSharedPtr<FJsonValue>> ComponentsArray;
    
    // Get Simple Construction Script
    USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
    if (!SCS)
    {
        UE_LOG(LogTemp, Warning, TEXT("Blueprint has no SimpleConstructionScript"));
        return ComponentsArray;
    }
    
    // Iterate through all nodes
    const TArray<USCS_Node*>& AllNodes = SCS->GetAllNodes();
    UE_LOG(LogTemp, Display, TEXT("Found %d components in blueprint"), AllNodes.Num());
    
    for (USCS_Node* Node : AllNodes)
    {
        if (!Node || !Node->ComponentTemplate)
        {
            continue;
        }
        
        TSharedPtr<FJsonObject> CompObj = MakeShared<FJsonObject>();
        
        // Basic component info
        CompObj->SetStringField(TEXT("name"), Node->GetVariableName().ToString());
        CompObj->SetStringField(TEXT("type"), Node->ComponentTemplate->GetClass()->GetName());
        
        // Parent component
        if (Node->ParentComponentOrVariableName != NAME_None)
        {
            CompObj->SetStringField(TEXT("parent_component"), Node->ParentComponentOrVariableName.ToString());
        }
        else
        {
            CompObj->SetStringField(TEXT("parent_component"), TEXT("None"));
        }
        
        // Transform (for SceneComponents)
        USceneComponent* SceneComp = Cast<USceneComponent>(Node->ComponentTemplate);
        if (SceneComp)
        {
            TSharedPtr<FJsonObject> TransformObj = MakeShared<FJsonObject>();
            
            // Location
            FVector Location = SceneComp->GetRelativeLocation();
            TArray<TSharedPtr<FJsonValue>> LocationArray;
            LocationArray.Add(MakeShared<FJsonValueNumber>(Location.X));
            LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Y));
            LocationArray.Add(MakeShared<FJsonValueNumber>(Location.Z));
            TransformObj->SetArrayField(TEXT("location"), LocationArray);
            
            // Rotation
            FRotator Rotation = SceneComp->GetRelativeRotation();
            TArray<TSharedPtr<FJsonValue>> RotationArray;
            RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Pitch));
            RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Yaw));
            RotationArray.Add(MakeShared<FJsonValueNumber>(Rotation.Roll));
            TransformObj->SetArrayField(TEXT("rotation"), RotationArray);
            
            // Scale
            FVector Scale = SceneComp->GetRelativeScale3D();
            TArray<TSharedPtr<FJsonValue>> ScaleArray;
            ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.X));
            ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Y));
            ScaleArray.Add(MakeShared<FJsonValueNumber>(Scale.Z));
            TransformObj->SetArrayField(TEXT("scale"), ScaleArray);
            
            CompObj->SetObjectField(TEXT("transform"), TransformObj);
            
            // Phase 2: Enhanced properties
            CompObj->SetBoolField(TEXT("mobility"), SceneComp->Mobility == EComponentMobility::Movable);
            CompObj->SetBoolField(TEXT("visible"), SceneComp->IsVisible());
            CompObj->SetBoolField(TEXT("hidden_in_game"), SceneComp->bHiddenInGame);
        }
        
        // Phase 2: StaticMeshComponent-specific properties
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Node->ComponentTemplate);
        if (MeshComp)
        {
            TSharedPtr<FJsonObject> MeshPropsObj = MakeShared<FJsonObject>();
            
            // Static mesh asset
            if (MeshComp->GetStaticMesh())
            {
                MeshPropsObj->SetStringField(TEXT("static_mesh"), MeshComp->GetStaticMesh()->GetPathName());
            }
            else
            {
                MeshPropsObj->SetStringField(TEXT("static_mesh"), TEXT(""));
            }
            
            // Physics
            MeshPropsObj->SetBoolField(TEXT("simulate_physics"), MeshComp->IsSimulatingPhysics());
            MeshPropsObj->SetBoolField(TEXT("generate_overlap_events"), MeshComp->GetGenerateOverlapEvents());
            MeshPropsObj->SetNumberField(TEXT("mass"), MeshComp->GetMass());
            
            // Rendering
            MeshPropsObj->SetBoolField(TEXT("cast_shadow"), MeshComp->CastShadow);
            
            // Materials count
            MeshPropsObj->SetNumberField(TEXT("num_materials"), MeshComp->GetNumMaterials());
            
            CompObj->SetObjectField(TEXT("mesh_properties"), MeshPropsObj);
        }
        
        // Phase 2: Light component properties
        ULightComponent* LightComp = Cast<ULightComponent>(Node->ComponentTemplate);
        if (LightComp)
        {
            TSharedPtr<FJsonObject> LightPropsObj = MakeShared<FJsonObject>();
            
            LightPropsObj->SetNumberField(TEXT("intensity"), LightComp->Intensity);
            
            // Light color
            FLinearColor LightColor = LightComp->LightColor;
            TArray<TSharedPtr<FJsonValue>> ColorArray;
            ColorArray.Add(MakeShared<FJsonValueNumber>(LightColor.R));
            ColorArray.Add(MakeShared<FJsonValueNumber>(LightColor.G));
            ColorArray.Add(MakeShared<FJsonValueNumber>(LightColor.B));
            ColorArray.Add(MakeShared<FJsonValueNumber>(LightColor.A));
            LightPropsObj->SetArrayField(TEXT("light_color"), ColorArray);
            
            LightPropsObj->SetBoolField(TEXT("cast_shadows"), LightComp->CastShadows);
            
            CompObj->SetObjectField(TEXT("light_properties"), LightPropsObj);
        }
        
        ComponentsArray.Add(MakeShared<FJsonValueObject>(CompObj));
    }
    
    return ComponentsArray;
}

TArray<TSharedPtr<FJsonValue>> FUnrealMCPBlueprintIntrospection::ExtractVariables(UBlueprint* Blueprint)
{
    TArray<TSharedPtr<FJsonValue>> VariablesArray;
    
    UE_LOG(LogTemp, Display, TEXT("Found %d variables in blueprint"), Blueprint->NewVariables.Num());
    
    for (const FBPVariableDescription& VarDesc : Blueprint->NewVariables)
    {
        TSharedPtr<FJsonObject> VarObj = MakeShared<FJsonObject>();
        
        // Variable name
        VarObj->SetStringField(TEXT("name"), VarDesc.VarName.ToString());
        
        // Phase 3: Enhanced type information
        TSharedPtr<FJsonObject> TypeObj = MakeShared<FJsonObject>();
        TypeObj->SetStringField(TEXT("category"), VarDesc.VarType.PinCategory.ToString());
        TypeObj->SetStringField(TEXT("sub_category"), VarDesc.VarType.PinSubCategory.ToString());
        
        // Container type (none, array, set, map)
        FString ContainerType = TEXT("none");
        if (VarDesc.VarType.ContainerType == EPinContainerType::Array)
        {
            ContainerType = TEXT("array");
        }
        else if (VarDesc.VarType.ContainerType == EPinContainerType::Set)
        {
            ContainerType = TEXT("set");
        }
        else if (VarDesc.VarType.ContainerType == EPinContainerType::Map)
        {
            ContainerType = TEXT("map");
        }
        TypeObj->SetStringField(TEXT("container_type"), ContainerType);
        
        // Object/class reference
        if (VarDesc.VarType.PinSubCategoryObject.IsValid())
        {
            TypeObj->SetStringField(TEXT("object_type"), VarDesc.VarType.PinSubCategoryObject->GetName());
            TypeObj->SetStringField(TEXT("object_path"), VarDesc.VarType.PinSubCategoryObject->GetPathName());
        }
        
        // Is reference type
        TypeObj->SetBoolField(TEXT("is_reference"), VarDesc.VarType.bIsReference);
        TypeObj->SetBoolField(TEXT("is_const"), VarDesc.VarType.bIsConst);
        TypeObj->SetBoolField(TEXT("is_weak_pointer"), VarDesc.VarType.bIsWeakPointer);
        
        VarObj->SetObjectField(TEXT("type_info"), TypeObj);
        
        // Legacy simple type string for backward compatibility
        FString TypeStr = VarDesc.VarType.PinCategory.ToString();
        if (VarDesc.VarType.PinSubCategoryObject.IsValid())
        {
            TypeStr += TEXT(":") + VarDesc.VarType.PinSubCategoryObject->GetName();
        }
        VarObj->SetStringField(TEXT("type"), TypeStr);
        
        // Category
        VarObj->SetStringField(TEXT("category"), VarDesc.Category.IsEmpty() ? TEXT("") : VarDesc.Category.ToString());
        
        // Friendly name / tooltip  
        VarObj->SetStringField(TEXT("friendly_name"), VarDesc.FriendlyName);
        
        // Phase 3: Extract metadata entries
        if (VarDesc.MetaDataArray.Num() > 0)
        {
            TSharedPtr<FJsonObject> MetadataObj = MakeShared<FJsonObject>();
            for (const FBPVariableMetaDataEntry& MetaEntry : VarDesc.MetaDataArray)
            {
                MetadataObj->SetStringField(MetaEntry.DataKey.ToString(), MetaEntry.DataValue);
            }
            VarObj->SetObjectField(TEXT("metadata"), MetadataObj);
        }
        
        // Flags
        VarObj->SetBoolField(TEXT("is_exposed"), (VarDesc.PropertyFlags & CPF_ExposeOnSpawn) != 0);
        VarObj->SetBoolField(TEXT("is_blueprint_read_only"), (VarDesc.PropertyFlags & CPF_BlueprintReadOnly) != 0);
        VarObj->SetBoolField(TEXT("is_editable"), (VarDesc.PropertyFlags & CPF_Edit) != 0);
        VarObj->SetBoolField(TEXT("is_blueprint_visible"), (VarDesc.PropertyFlags & CPF_BlueprintVisible) != 0);
        VarObj->SetBoolField(TEXT("is_transient"), (VarDesc.PropertyFlags & CPF_Transient) != 0);
        VarObj->SetBoolField(TEXT("is_config"), (VarDesc.PropertyFlags & CPF_Config) != 0);
        
        // Replication
        FString ReplicationType = TEXT("None");
        if (VarDesc.PropertyFlags & CPF_Net)
        {
            ReplicationType = TEXT("Replicated");
            if (VarDesc.RepNotifyFunc != NAME_None)
            {
                ReplicationType = TEXT("RepNotify");
                VarObj->SetStringField(TEXT("rep_notify_function"), VarDesc.RepNotifyFunc.ToString());
            }
        }
        VarObj->SetStringField(TEXT("replication"), ReplicationType);
        
        // Replication condition
        if (VarDesc.PropertyFlags & CPF_Net)
        {
            FString RepCondition = TEXT("None");
            switch (VarDesc.ReplicationCondition)
            {
                case COND_InitialOnly: RepCondition = TEXT("InitialOnly"); break;
                case COND_OwnerOnly: RepCondition = TEXT("OwnerOnly"); break;
                case COND_SkipOwner: RepCondition = TEXT("SkipOwner"); break;
                case COND_SimulatedOnly: RepCondition = TEXT("SimulatedOnly"); break;
                case COND_AutonomousOnly: RepCondition = TEXT("AutonomousOnly"); break;
                case COND_SimulatedOrPhysics: RepCondition = TEXT("SimulatedOrPhysics"); break;
                case COND_InitialOrOwner: RepCondition = TEXT("InitialOrOwner"); break;
                case COND_Custom: RepCondition = TEXT("Custom"); break;
                case COND_ReplayOrOwner: RepCondition = TEXT("ReplayOrOwner"); break;
                case COND_ReplayOnly: RepCondition = TEXT("ReplayOnly"); break;
                case COND_SimulatedOnlyNoReplay: RepCondition = TEXT("SimulatedOnlyNoReplay"); break;
                case COND_SimulatedOrPhysicsNoReplay: RepCondition = TEXT("SimulatedOrPhysicsNoReplay"); break;
                case COND_SkipReplay: RepCondition = TEXT("SkipReplay"); break;
                default: RepCondition = TEXT("None");
            }
            VarObj->SetStringField(TEXT("replication_condition"), RepCondition);
        }
        
        // Default value (basic string representation)
        VarObj->SetStringField(TEXT("default_value"), VarDesc.DefaultValue);
        
        // Variable GUID (unique identifier)
        VarObj->SetStringField(TEXT("guid"), VarDesc.VarGuid.ToString());
        
        VariablesArray.Add(MakeShared<FJsonValueObject>(VarObj));
    }
    
    return VariablesArray;
}

TArray<TSharedPtr<FJsonValue>> FUnrealMCPBlueprintIntrospection::ExtractFunctions(UBlueprint* Blueprint)
{
    TArray<TSharedPtr<FJsonValue>> FunctionsArray;
    
    if (!Blueprint)
    {
        return FunctionsArray;
    }
    
    // Get all graphs in the Blueprint
    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);
    
    UE_LOG(LogTemp, Warning, TEXT("ExtractFunctions: Blueprint=%s, Total Graphs=%d"), 
        *Blueprint->GetName(), AllGraphs.Num());
    
    for (UEdGraph* Graph : AllGraphs)
    {
        if (!Graph)
        {
            continue;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  Processing Graph: %s, Schema=%s, NumNodes=%d, Outer=%s"), 
            *Graph->GetName(), 
            Graph->Schema ? *Graph->Schema->GetName() : TEXT("NULL"),
            Graph->Nodes.Num(),
            Graph->GetOuter() ? *Graph->GetOuter()->GetName() : TEXT("NULL"));
        
        // Skip event graphs (UbergraphPages)
        bool bIsEventGraph = false;
        for (UEdGraph* UberGraph : Blueprint->UbergraphPages)
        {
            if (Graph == UberGraph)
            {
                UE_LOG(LogTemp, Warning, TEXT("    -> SKIPPING: This is an Event Graph (UbergraphPage)"));
                bIsEventGraph = true;
                break;
            }
        }
        
        if (bIsEventGraph)
        {
            continue;
        }
        
        // Look for function graphs (they have FunctionEntry nodes)
        UK2Node_FunctionEntry* EntryNode = nullptr;
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            EntryNode = Cast<UK2Node_FunctionEntry>(Node);
            if (EntryNode)
            {
                break;
            }
        }
        
        // Skip if not a function graph
        if (!EntryNode)
        {
            UE_LOG(LogTemp, Warning, TEXT("    -> SKIPPING: No FunctionEntry node found"));
            continue;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("    -> EXTRACTING: Function with %d nodes"), Graph->Nodes.Num());
        
        TSharedPtr<FJsonObject> FuncObj = MakeShared<FJsonObject>();
        
        // Function name
        FuncObj->SetStringField(TEXT("name"), Graph->GetName());
        
        // Function metadata from entry node
        FuncObj->SetStringField(TEXT("category"), EntryNode->MetaData.Category.ToString());
        FuncObj->SetStringField(TEXT("description"), EntryNode->MetaData.ToolTip.ToString());
        FuncObj->SetBoolField(TEXT("is_pure"), EntryNode->MetaData.bCallInEditor);
        
        // Access specifier
        FString AccessSpecifier = TEXT("public");
        if (EntryNode->GetFunctionFlags() & FUNC_Private)
        {
            AccessSpecifier = TEXT("private");
        }
        else if (EntryNode->GetFunctionFlags() & FUNC_Protected)
        {
            AccessSpecifier = TEXT("protected");
        }
        FuncObj->SetStringField(TEXT("access_specifier"), AccessSpecifier);
        
        // Extract input parameters
        TArray<TSharedPtr<FJsonValue>> InputsArray;
        for (UEdGraphPin* Pin : EntryNode->Pins)
        {
            // Output pins on entry node = function inputs
            if (Pin && Pin->Direction == EGPD_Output && 
                Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                TSharedPtr<FJsonObject> ParamObj = MakeShared<FJsonObject>();
                ParamObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
                ParamObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
                
                // Sub-category (for object/enum types)
                if (Pin->PinType.PinSubCategory != NAME_None)
                {
                    ParamObj->SetStringField(TEXT("sub_type"), Pin->PinType.PinSubCategory.ToString());
                }
                
                // Object type
                if (Pin->PinType.PinSubCategoryObject.IsValid())
                {
                    ParamObj->SetStringField(TEXT("object_type"), 
                        Pin->PinType.PinSubCategoryObject->GetName());
                }
                
                // Default value
                if (!Pin->DefaultValue.IsEmpty())
                {
                    ParamObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);
                }
                
                // Is reference/const
                ParamObj->SetBoolField(TEXT("is_reference"), Pin->PinType.bIsReference);
                ParamObj->SetBoolField(TEXT("is_const"), Pin->PinType.bIsConst);
                
                InputsArray.Add(MakeShared<FJsonValueObject>(ParamObj));
            }
        }
        FuncObj->SetArrayField(TEXT("inputs"), InputsArray);
        
        // Extract output parameters (from FunctionResult node)
        TArray<TSharedPtr<FJsonValue>> OutputsArray;
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            UK2Node_FunctionResult* ResultNode = Cast<UK2Node_FunctionResult>(Node);
            if (ResultNode)
            {
                for (UEdGraphPin* Pin : ResultNode->Pins)
                {
                    // Input pins on result node = function outputs
                    if (Pin && Pin->Direction == EGPD_Input && 
                        Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
                    {
                        TSharedPtr<FJsonObject> ParamObj = MakeShared<FJsonObject>();
                        ParamObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
                        ParamObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
                        
                        // Sub-category
                        if (Pin->PinType.PinSubCategory != NAME_None)
                        {
                            ParamObj->SetStringField(TEXT("sub_type"), 
                                Pin->PinType.PinSubCategory.ToString());
                        }
                        
                        // Object type
                        if (Pin->PinType.PinSubCategoryObject.IsValid())
                        {
                            ParamObj->SetStringField(TEXT("object_type"), 
                                Pin->PinType.PinSubCategoryObject->GetName());
                        }
                        
                        // Is reference/const
                        ParamObj->SetBoolField(TEXT("is_reference"), Pin->PinType.bIsReference);
                        ParamObj->SetBoolField(TEXT("is_const"), Pin->PinType.bIsConst);
                        
                        OutputsArray.Add(MakeShared<FJsonValueObject>(ParamObj));
                    }
                }
                break; // Only one result node per function
            }
        }
        FuncObj->SetArrayField(TEXT("outputs"), OutputsArray);
        
        // Extract local variables
        TArray<TSharedPtr<FJsonValue>> LocalVarsArray;
        for (const FBPVariableDescription& LocalVar : EntryNode->LocalVariables)
        {
            TSharedPtr<FJsonObject> LocalVarObj = MakeShared<FJsonObject>();
            LocalVarObj->SetStringField(TEXT("name"), LocalVar.VarName.ToString());
            LocalVarObj->SetStringField(TEXT("type"), LocalVar.VarType.PinCategory.ToString());
            
            // Sub-category
            if (LocalVar.VarType.PinSubCategory != NAME_None)
            {
                LocalVarObj->SetStringField(TEXT("sub_type"), 
                    LocalVar.VarType.PinSubCategory.ToString());
            }
            
            LocalVarsArray.Add(MakeShared<FJsonValueObject>(LocalVarObj));
        }
        FuncObj->SetArrayField(TEXT("local_variables"), LocalVarsArray);
        
        // Extract full graph data (nodes and connections)
        TSharedPtr<FJsonObject> GraphData = ExtractGraphData(Graph);
        if (GraphData.IsValid())
        {
            FuncObj->SetObjectField(TEXT("graph"), GraphData);
        }
        
        FunctionsArray.Add(MakeShared<FJsonValueObject>(FuncObj));
    }
    
    UE_LOG(LogTemp, Display, TEXT("Extracted %d functions"), FunctionsArray.Num());
    
    return FunctionsArray;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintIntrospection::ExtractGraphData(UEdGraph* Graph)
{
    TSharedPtr<FJsonObject> GraphObj = MakeShared<FJsonObject>();
    
    if (!Graph)
    {
        return GraphObj;
    }
    
    // Extract all nodes
    TArray<TSharedPtr<FJsonValue>> NodesArray;
    TMap<FGuid, int32> NodeGuidToIndex;  // For connection references
    int32 NodeIndex = 0;
    
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (!Node) continue;
        
        TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
        NodeObj->SetStringField(TEXT("id"), Node->NodeGuid.ToString());
        NodeObj->SetStringField(TEXT("type"), Node->GetClass()->GetName());
        NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        NodeObj->SetNumberField(TEXT("pos_x"), Node->NodePosX);
        NodeObj->SetNumberField(TEXT("pos_y"), Node->NodePosY);
        
        // Node-specific data extraction
        if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
        {
            NodeObj->SetStringField(TEXT("node_category"), TEXT("event"));
            if (EventNode->EventReference.GetMemberName().IsValid())
            {
                NodeObj->SetStringField(TEXT("event_name"), 
                    EventNode->EventReference.GetMemberName().ToString());
            }
        }
        else if (UK2Node_CallFunction* CallNode = Cast<UK2Node_CallFunction>(Node))
        {
            NodeObj->SetStringField(TEXT("node_category"), TEXT("function_call"));
            if (CallNode->FunctionReference.GetMemberName().IsValid())
            {
                NodeObj->SetStringField(TEXT("function_name"), 
                    CallNode->FunctionReference.GetMemberName().ToString());
            }
        }
        else if (UK2Node_VariableGet* VarGetNode = Cast<UK2Node_VariableGet>(Node))
        {
            NodeObj->SetStringField(TEXT("node_category"), TEXT("variable_get"));
            if (VarGetNode->VariableReference.GetMemberName().IsValid())
            {
                NodeObj->SetStringField(TEXT("variable_name"), 
                    VarGetNode->VariableReference.GetMemberName().ToString());
            }
        }
        else if (UK2Node_VariableSet* VarSetNode = Cast<UK2Node_VariableSet>(Node))
        {
            NodeObj->SetStringField(TEXT("node_category"), TEXT("variable_set"));
            if (VarSetNode->VariableReference.GetMemberName().IsValid())
            {
                NodeObj->SetStringField(TEXT("variable_name"), 
                    VarSetNode->VariableReference.GetMemberName().ToString());
            }
        }
        else if (UK2Node_CustomEvent* CustomEventNode = Cast<UK2Node_CustomEvent>(Node))
        {
            NodeObj->SetStringField(TEXT("node_category"), TEXT("custom_event"));
            NodeObj->SetStringField(TEXT("event_name"), 
                CustomEventNode->CustomFunctionName.ToString());
        }
        else
        {
            NodeObj->SetStringField(TEXT("node_category"), TEXT("other"));
        }
        
        // Extract pins
        TArray<TSharedPtr<FJsonValue>> PinsArray;
        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (!Pin) continue;
            
            TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
            PinObj->SetStringField(TEXT("id"), Pin->PinId.ToString());
            PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
            PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
            PinObj->SetStringField(TEXT("direction"), 
                Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
            
            // Sub-category (for object/enum types)
            if (Pin->PinType.PinSubCategory != NAME_None)
            {
                PinObj->SetStringField(TEXT("sub_type"), 
                    Pin->PinType.PinSubCategory.ToString());
            }
            
            // Object type
            if (Pin->PinType.PinSubCategoryObject.IsValid())
            {
                PinObj->SetStringField(TEXT("object_type"), 
                    Pin->PinType.PinSubCategoryObject->GetName());
            }
            
            // Default value for input pins
            if (!Pin->DefaultValue.IsEmpty())
            {
                PinObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);
            }
            
            // Pin flags
            PinObj->SetBoolField(TEXT("is_reference"), Pin->PinType.bIsReference);
            PinObj->SetBoolField(TEXT("is_const"), Pin->PinType.bIsConst);
            
            // Connection count
            PinObj->SetNumberField(TEXT("connection_count"), Pin->LinkedTo.Num());
            
            PinsArray.Add(MakeShared<FJsonValueObject>(PinObj));
        }
        NodeObj->SetArrayField(TEXT("pins"), PinsArray);
        
        NodesArray.Add(MakeShared<FJsonValueObject>(NodeObj));
        NodeGuidToIndex.Add(Node->NodeGuid, NodeIndex++);
    }
    
    GraphObj->SetArrayField(TEXT("nodes"), NodesArray);
    
    // Extract connections
    TArray<TSharedPtr<FJsonValue>> ConnectionsArray;
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (!Node) continue;
        
        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (!Pin) continue;
            
            // Only process output pins to avoid duplicates
            if (Pin->Direction == EGPD_Output)
            {
                for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                {
                    if (!LinkedPin || !LinkedPin->GetOwningNode()) continue;
                    
                    TSharedPtr<FJsonObject> ConnObj = MakeShared<FJsonObject>();
                    ConnObj->SetStringField(TEXT("from_node"), Node->NodeGuid.ToString());
                    ConnObj->SetStringField(TEXT("from_pin"), Pin->PinId.ToString());
                    ConnObj->SetStringField(TEXT("from_pin_name"), Pin->PinName.ToString());
                    ConnObj->SetStringField(TEXT("to_node"), 
                        LinkedPin->GetOwningNode()->NodeGuid.ToString());
                    ConnObj->SetStringField(TEXT("to_pin"), LinkedPin->PinId.ToString());
                    ConnObj->SetStringField(TEXT("to_pin_name"), LinkedPin->PinName.ToString());
                    
                    ConnectionsArray.Add(MakeShared<FJsonValueObject>(ConnObj));
                }
            }
        }
    }
    
    GraphObj->SetArrayField(TEXT("connections"), ConnectionsArray);
    
    // Graph statistics
    GraphObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
    GraphObj->SetNumberField(TEXT("connection_count"), ConnectionsArray.Num());
    
    return GraphObj;
}
