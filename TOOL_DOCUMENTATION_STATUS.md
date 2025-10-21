# Tool Documentation Status

## Overview
This document tracks the documentation quality of UnrealMCP tools and identifies areas for improvement.

## Current State

### ✅ What We Have
- All tools have basic docstrings
- Function signatures with type hints
- Basic parameter descriptions in some tools

### ❌ What's Missing
Many tools need improvement to follow MCP best practices:

1. **Detailed Descriptions**
   - What the tool does
   - When to use it
   - What it affects in the engine

2. **Comprehensive Parameter Documentation**
   - Detailed explanation of each parameter
   - Valid value ranges and formats
   - Examples of valid inputs
   - Default behavior explanation

3. **Return Value Documentation**
   - Structure of returned data
   - What each field means
   - Success/error cases

4. **Usage Examples**
   - Concrete examples with actual values
   - Common use cases
   - Edge cases

## Example: Good vs. Bad Documentation

### ❌ Bad (Before)
```python
@mcp.tool()
def find_actors_by_name(ctx: Context, pattern: str) -> List[str]:
    """Find actors by name pattern."""
```

**Problems:**
- Too brief, AI doesn't know what "pattern" means
- No examples of valid patterns
- Return type unclear
- No information about case sensitivity

### ✅ Good (After)
```python
@mcp.tool()
def find_actors_by_name(ctx: Context, pattern: str) -> List[Dict[str, Any]]:
    """Find actors in the level by name pattern (case-sensitive substring match).
    
    Searches through all actors in the current level and returns those whose names
    contain the specified pattern. This is useful for quickly locating specific
    actors or groups of actors with similar naming conventions.
    
    Args:
        pattern: The text pattern to search for in actor names. This performs a
                case-sensitive substring match. Examples:
                - "Light" finds "DirectionalLight", "PointLight_1", "SpotLight"
                - "BP_" finds all Blueprint actors like "BP_Player", "BP_Enemy_2"
                - "Player" finds "PlayerStart", "BP_PlayerCharacter", etc.
    
    Returns:
        List of actor dictionaries, each containing:
        - name: The actor's unique name in the level
        - class: The actor's class type (e.g., "StaticMeshActor", "PointLight")
        - location: World position as [X, Y, Z] in centimeters
        - rotation: Rotation as [Pitch, Yaw, Roll] in degrees
        - scale: Scale as [X, Y, Z] multiplier
        
        Returns empty list if no actors match the pattern.
    
    Examples:
        >>> find_actors_by_name(pattern="Light")
        [{"name": "PointLight_1", "class": "PointLight", ...},
         {"name": "DirectionalLight_Sun", "class": "DirectionalLight", ...}]
        
        >>> find_actors_by_name(pattern="BP_Enemy")
        [{"name": "BP_Enemy_0", "class": "BP_Enemy_C", ...}]
    """
```

**Benefits:**
- AI knows exactly what to pass as `pattern`
- Clear examples help AI make correct function calls
- Return structure documented (AI knows what data to expect)
- Case sensitivity explicitly stated

## Guidelines from `.github/copilot-instructions.md`

According to the project guidelines, MCP tools must:

1. **Avoid problematic types:**
   - ❌ `Any`, `object`, `Optional[T]`, `Union[T]`
   - ✅ Use concrete types or `T = None` with default handling

2. **Handle None defaults properly:**
   - ❌ `x: T | None = None`
   - ✅ `x: T = None` (handle None in function body)

3. **Include comprehensive docstrings:**
   - Detailed description
   - Parameter explanations with examples
   - Return value structure
   - Usage examples

## Tools Documentation Status

### Editor Tools
- [x] `get_actors_in_level` - ✅ GOOD (detailed docs, examples)
- [x] `find_actors_by_name` - ✅ GOOD (just improved)
- [ ] `spawn_actor` - ⚠️ NEEDS IMPROVEMENT
- [ ] `delete_actor` - ⚠️ NEEDS IMPROVEMENT
- [ ] `set_actor_transform` - ⚠️ NEEDS IMPROVEMENT
- [ ] `get_actor_properties` - ⚠️ NEEDS IMPROVEMENT
- [ ] `set_actor_property` - ⚠️ NEEDS IMPROVEMENT
- [ ] `spawn_blueprint_actor` - ⚠️ NEEDS IMPROVEMENT

### Blueprint Tools
- [ ] All tools need improvement

### Node Tools
- [ ] All tools need improvement

### UMG Tools
- [ ] All tools need improvement

### Project Tools
- [ ] All tools need improvement

## Action Items

### High Priority
1. **Document spawn_actor** - Most commonly used tool
2. **Document create_blueprint** - Core Blueprint workflow
3. **Document add_component_to_blueprint** - Essential for Blueprint creation

### Medium Priority
4. Document transform tools
5. Document node tools
6. Document widget tools

### Low Priority
7. Document advanced tools
8. Add more usage examples
9. Create tutorial documentation

## Template for Good Documentation

Use this template when documenting tools:

```python
@mcp.tool()
def tool_name(
    ctx: Context,
    required_param: str,
    optional_param: int = 100
) -> Dict[str, Any]:
    """[One-line summary of what the tool does]
    
    [Longer description explaining:]
    - What this tool does in detail
    - When you should use it
    - What it affects in Unreal Engine
    - Any important caveats or limitations
    
    Args:
        required_param: [Detailed explanation]
                       - What it controls
                       - Valid formats/ranges
                       - Examples: "value1", "value2", "value3"
        
        optional_param: [Detailed explanation]
                       - What it controls
                       - What happens if not provided (default: 100)
                       - Valid range: 0-1000
                       - Examples: 50, 150, 500
    
    Returns:
        Dictionary containing:
        - field1: Description of field1
        - field2: Description of field2
        - success: Boolean indicating operation success
        
        Returns {"success": False, "message": "..."} on error.
    
    Examples:
        >>> # Example 1: Basic usage
        >>> tool_name(required_param="value1")
        {"field1": "...", "success": True}
        
        >>> # Example 2: With optional parameter
        >>> tool_name(required_param="value2", optional_param=200)
        {"field1": "...", "field2": "...", "success": True}
        
        >>> # Example 3: Error case
        >>> tool_name(required_param="invalid")
        {"success": False, "message": "Invalid parameter"}
    
    Raises:
        Does not raise exceptions - returns error dict instead.
    """
    # Implementation...
```

## Benefits of Good Documentation

1. **For AI Assistants:**
   - Can call tools correctly without trial and error
   - Understand parameter formats and constraints
   - Know what to expect in return values
   - Can suggest tools for user's needs

2. **For Developers:**
   - Understand tool capabilities quickly
   - Know how to use tools without reading code
   - Can debug issues faster
   - Easier to maintain and extend

3. **For Users:**
   - Better AI assistance (AI makes fewer mistakes)
   - Faster problem resolution
   - More reliable automation

## Next Steps

1. Review `TOOL_CHECKLIST.md` for complete testing plan
2. Improve documentation for high-priority tools
3. Test each tool and record results in checklist
4. Create example scripts demonstrating common workflows
5. Update README with link to tool documentation
