# UnrealMCP Tool Testing Checklist

This document provides a comprehensive checklist for testing all MCP tools with expected results.

## Testing Status Legend
- ✅ **PASS** - Tool works as expected
- ❌ **FAIL** - Tool has issues/bugs
- ⏸️ **SKIP** - Tool disabled/commented out
- 🔄 **PENDING** - Not yet tested

---

## Editor Tools (editor_tools.py)

### 1. get_actors_in_level
**Status:** ✅ PASS (Tested 2024-10-21)

**Description:** Retrieves a list of actors currently in the level with optional limit.

**Parameters:**
- `max_actors` (int, default=100): Maximum number of actors to return. Set to 0 for unlimited (use with caution).

**Test Cases:**
```python
# Test 1: Default limit (100 actors)
get_actors_in_level()

# Test 2: Small limit
get_actors_in_level(max_actors=10)

# Test 3: Tiny limit
get_actors_in_level(max_actors=3)
```

**Expected Results:**
- Returns list of actor dictionaries with name, class, location, rotation, scale
- Respects max_actors limit
- Returns metadata: total_actors, returned_actors, truncated

**Actual Results:**
- ✅ Returns exactly the number of actors requested
- ✅ No editor hanging even with large levels
- ✅ Proper JSON structure

---

### 2. find_actors_by_name
**Status:** 🔄 PENDING

**Description:** Find actors matching a name pattern (supports wildcards).

**Parameters:**
- `pattern` (str, required): Name pattern to search for (e.g., "Light", "BP_*", "*Player*")

**Test Cases:**
```python
# Test 1: Find all lights
find_actors_by_name(pattern="Light")

# Test 2: Find specific actor
find_actors_by_name(pattern="PlayerStart")

# Test 3: Wildcard search
find_actors_by_name(pattern="BP_*")
```

**Expected Results:**
- Returns list of matching actors
- Empty list if no matches
- Case-sensitive/insensitive behavior documented

**Actual Results:**
- [ ] Test with simple pattern
- [ ] Test with wildcards
- [ ] Test with no matches

---

### 3. spawn_actor
**Status:** 🔄 PENDING

**Description:** Create a new actor in the level.

**Parameters:**
- `name` (str, required): Unique name for the actor
- `type` (str, required): Actor class type (e.g., "STATICMESHACTOR", "POINTLIGHT", "CAMERA")
- `location` (List[float], default=[0,0,0]): World position [X, Y, Z]
- `rotation` (List[float], default=[0,0,0]): Rotation [Pitch, Yaw, Roll] in degrees
- `scale` (List[float], optional): Scale [X, Y, Z]

**Test Cases:**
```python
# Test 1: Basic spawn at origin
spawn_actor(name="TestCube", type="STATICMESHACTOR")

# Test 2: Spawn with transform
spawn_actor(
    name="TestLight",
    type="POINTLIGHT",
    location=[100, 200, 300],
    rotation=[0, 45, 0]
)

# Test 3: Duplicate name (should fail)
spawn_actor(name="TestCube", type="STATICMESHACTOR")
```

**Expected Results:**
- Actor appears in level
- Correct transform applied
- Error on duplicate name
- Returns actor properties

**Actual Results:**
- [ ] Basic spawn works
- [ ] Transform applied correctly
- [ ] Proper error handling

---

### 4. delete_actor
**Status:** 🔄 PENDING

**Description:** Delete an actor from the level by name.

**Parameters:**
- `name` (str, required): Name of the actor to delete

**Test Cases:**
```python
# Test 1: Delete existing actor
delete_actor(name="TestCube")

# Test 2: Delete non-existent actor (should fail gracefully)
delete_actor(name="NonExistentActor")
```

**Expected Results:**
- Actor removed from level
- Graceful error if actor doesn't exist
- Success/failure status returned

**Actual Results:**
- [ ] Successful deletion
- [ ] Error handling for missing actor

---

### 5. set_actor_transform
**Status:** 🔄 PENDING

**Description:** Modify an actor's transform (position, rotation, scale).

**Parameters:**
- `name` (str, required): Name of the actor
- `location` (List[float], optional): New position [X, Y, Z]
- `rotation` (List[float], optional): New rotation [Pitch, Yaw, Roll]
- `scale` (List[float], optional): New scale [X, Y, Z]

**Test Cases:**
```python
# Test 1: Change location only
set_actor_transform(name="TestCube", location=[500, 0, 0])

# Test 2: Change rotation only
set_actor_transform(name="TestCube", rotation=[0, 90, 0])

# Test 3: Change all components
set_actor_transform(
    name="TestCube",
    location=[1000, 1000, 100],
    rotation=[45, 45, 0],
    scale=[2, 2, 2]
)
```

**Expected Results:**
- Transform updates visible in level
- Partial updates work (only specified params change)
- Error on invalid actor name

**Actual Results:**
- [ ] Location update works
- [ ] Rotation update works
- [ ] Scale update works
- [ ] Partial updates work

---

### 6. get_actor_properties
**Status:** 🔄 PENDING

**Description:** Retrieve all properties of an actor.

**Parameters:**
- `name` (str, required): Name of the actor

**Test Cases:**
```python
# Test 1: Get properties of existing actor
get_actor_properties(name="TestCube")

# Test 2: Get properties of non-existent actor
get_actor_properties(name="NonExistent")
```

**Expected Results:**
- Returns dictionary of actor properties
- Includes transform, class, components
- Error on invalid actor

**Actual Results:**
- [ ] Properties returned correctly
- [ ] Error handling works

---

### 7. set_actor_property
**Status:** 🔄 PENDING

**Description:** Set a specific property value on an actor.

**Parameters:**
- `name` (str, required): Name of the actor
- `property_name` (str, required): Name of the property to set
- `property_value` (any, required): Value to set

**Test Cases:**
```python
# Test 1: Set visibility
set_actor_property(name="TestCube", property_name="bHidden", property_value=False)

# Test 2: Set mobility
set_actor_property(name="TestCube", property_name="Mobility", property_value="Movable")
```

**Expected Results:**
- Property value changes
- Error on invalid property name
- Type validation

**Actual Results:**
- [ ] Property set successfully
- [ ] Error handling works

---

### 8. focus_viewport
**Status:** ⏸️ SKIP (Commented out - buggy)

**Description:** Focus the viewport camera on an actor or location.

**Parameters:**
- `target` (str, optional): Actor name to focus on
- `location` (List[float], optional): World position to focus on
- `distance` (float, default=1000): Distance from target
- `orientation` (List[float], optional): Camera rotation

**Test Cases:**
```python
# Test 1: Focus on actor
focus_viewport(target="TestCube")

# Test 2: Focus on location
focus_viewport(location=[0, 0, 0], distance=500)
```

**Expected Results:**
- Viewport camera moves
- Actor/location centered in view

**Actual Results:**
- ⏸️ Tool disabled due to bugs

---

### 9. spawn_blueprint_actor
**Status:** 🔄 PENDING

**Description:** Spawn an instance of a Blueprint class.

**Parameters:**
- `blueprint_name` (str, required): Name of the Blueprint class
- `actor_name` (str, required): Name for the spawned instance
- `location` (List[float], default=[0,0,0]): World position
- `rotation` (List[float], default=[0,0,0]): Rotation in degrees

**Test Cases:**
```python
# Test 1: Spawn blueprint at origin
spawn_blueprint_actor(
    blueprint_name="BP_MyBlueprint",
    actor_name="BP_MyBlueprint_Instance1"
)

# Test 2: Spawn with transform
spawn_blueprint_actor(
    blueprint_name="BP_MyBlueprint",
    actor_name="BP_MyBlueprint_Instance2",
    location=[500, 500, 0],
    rotation=[0, 45, 0]
)
```

**Expected Results:**
- Blueprint instance appears in level
- All Blueprint components functional
- Transform applied correctly

**Actual Results:**
- [ ] Basic spawn works
- [ ] Transform applied
- [ ] Blueprint functionality intact

---

### 10. get_console_output
**Status:** ✅ PASS (Tested 2024-10-21)

**Description:** Get recent console output from the Unreal Editor's log.

**Parameters:**
- `max_lines` (int, default=500): Maximum number of log lines to return
- `severity` (str, default="All"): Filter by severity level ("All", "Display", "Warning", "Error")
- `category` (str, default=""): Filter by log category ("LogTemp", "LogBlueprint", etc.)

**Test Cases:**
```python
# Test 1: Get last 500 messages (default)
get_console_output()

# Test 2: Get only errors
get_console_output(severity="Error")

# Test 3: Get recent LogTemp messages
get_console_output(max_lines=100, category="LogTemp")

# Test 4: Get Blueprint warnings
get_console_output(category="LogBlueprint", severity="Warning")
```

**Expected Results:**
- Returns list of log entries with timestamp, category, severity, message
- Filters work correctly
- Returns metadata (count, max_lines, applied filters)

**Actual Results:**
- ✅ Default retrieval works - returns real log messages
- ✅ Severity filtering works - correctly filters by Warning/Error/Display
- ✅ Category filtering works - filters by LogTemp, LogStreaming, etc.
- ✅ Max lines limit respected - returns exactly requested number
- ✅ Circular buffer captures last 1000 entries
- ✅ Thread-safe operation

---

## Blueprint Tools (blueprint_tools.py)

### 11. create_blueprint
**Status:** 🔄 PENDING

**Description:** Create a new Blueprint class.

**Parameters:**
- `name` (str, required): Name for the new Blueprint
- `parent_class` (str, required): Parent class (e.g., "Actor", "Pawn", "Character")

**Test Cases:**
```python
# Test 1: Create basic Actor Blueprint
create_blueprint(name="BP_TestActor", parent_class="Actor")

# Test 2: Create Pawn Blueprint
create_blueprint(name="BP_TestPawn", parent_class="Pawn")
```

**Expected Results:**
- Blueprint asset created in Content Browser
- Blueprint can be opened in editor
- Inherits from correct parent class

**Actual Results:**
- [ ] Blueprint created
- [ ] Correct parent class
- [ ] Editable in BP editor

---

### 12. add_component_to_blueprint
**Status:** 🔄 PENDING

**Description:** Add a component to a Blueprint class.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint name
- `component_type` (str, required): Component class (e.g., "StaticMeshComponent", "PointLightComponent")
- `component_name` (str, required): Name for the component
- `location` (List[float], optional): Relative position
- `rotation` (List[float], optional): Relative rotation
- `scale` (List[float], optional): Relative scale
- `component_properties` (Dict, optional): Additional properties

**Test Cases:**
```python
# Test 1: Add mesh component
add_component_to_blueprint(
    blueprint_name="BP_TestActor",
    component_type="StaticMeshComponent",
    component_name="MeshComp"
)

# Test 2: Add light with properties
add_component_to_blueprint(
    blueprint_name="BP_TestActor",
    component_type="PointLightComponent",
    component_name="LightComp",
    location=[0, 0, 100],
    component_properties={"Intensity": 5000.0}
)
```

**Expected Results:**
- Component appears in Blueprint hierarchy
- Properties set correctly
- Transform applied

**Actual Results:**
- [ ] Component added
- [ ] Properties set
- [ ] Transform correct

---

### 13. compile_blueprint
**Status:** 🔄 PENDING

**Description:** Compile a Blueprint to apply changes.

**Parameters:**
- `blueprint_name` (str, required): Name of Blueprint to compile

**Test Cases:**
```python
# Test 1: Compile modified Blueprint
compile_blueprint(blueprint_name="BP_TestActor")
```

**Expected Results:**
- Blueprint compiles successfully
- Changes applied
- Compilation errors reported if any

**Actual Results:**
- [ ] Successful compilation
- [ ] Error reporting works

---

### 14. set_blueprint_property
**Status:** 🔄 PENDING

**Description:** Set a property on a Blueprint's Class Default Object (CDO).

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `property_name` (str, required): Property to set
- `property_value` (any, required): Value to assign

**Test Cases:**
```python
# Test 1: Set Blueprint property
set_blueprint_property(
    blueprint_name="BP_TestActor",
    property_name="bReplicates",
    property_value=True
)
```

**Expected Results:**
- Property set on CDO
- Changes visible in Blueprint defaults

**Actual Results:**
- [ ] Property set correctly

---

### 15. set_component_property
**Status:** 🔄 PENDING

**Description:** Set a property on a component within a Blueprint.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `component_name` (str, required): Component name
- `property_name` (str, required): Property to set
- `property_value` (any, required): Value to assign

**Test Cases:**
```python
# Test 1: Set mesh component property
set_component_property(
    blueprint_name="BP_TestActor",
    component_name="MeshComp",
    property_name="CastShadow",
    property_value=True
)
```

**Expected Results:**
- Component property updated
- Visible in Blueprint editor

**Actual Results:**
- [ ] Property set correctly

---

### 16. set_static_mesh_properties
**Status:** 🔄 PENDING

**Description:** Configure a StaticMeshComponent's mesh asset.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `component_name` (str, required): StaticMeshComponent name
- `static_mesh` (str, default="/Engine/BasicShapes/Cube.Cube"): Path to mesh asset

**Test Cases:**
```python
# Test 1: Set cube mesh
set_static_mesh_properties(
    blueprint_name="BP_TestActor",
    component_name="MeshComp",
    static_mesh="/Engine/BasicShapes/Cube.Cube"
)

# Test 2: Set sphere mesh
set_static_mesh_properties(
    blueprint_name="BP_TestActor",
    component_name="MeshComp",
    static_mesh="/Engine/BasicShapes/Sphere.Sphere"
)
```

**Expected Results:**
- Mesh asset assigned to component
- Visible in viewport

**Actual Results:**
- [ ] Mesh assigned correctly
- [ ] Renders in viewport

---

### 17. set_physics_properties
**Status:** 🔄 PENDING

**Description:** Configure physics simulation for a component.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `component_name` (str, required): Component name
- `simulate_physics` (bool, default=True): Enable/disable physics
- `mass` (float, default=1.0): Mass in kg
- `linear_damping` (float, default=0.01): Linear damping
- `angular_damping` (float, default=0.0): Angular damping
- `gravity_enabled` (bool, default=True): Enable/disable gravity

**Test Cases:**
```python
# Test 1: Enable physics with defaults
set_physics_properties(
    blueprint_name="BP_TestActor",
    component_name="MeshComp",
    simulate_physics=True
)

# Test 2: Custom physics settings
set_physics_properties(
    blueprint_name="BP_TestActor",
    component_name="MeshComp",
    simulate_physics=True,
    mass=10.0,
    linear_damping=0.1,
    gravity_enabled=True
)
```

**Expected Results:**
- Physics simulation enabled
- Properties set correctly
- Actor responds to physics when spawned

**Actual Results:**
- [ ] Physics enabled
- [ ] Properties correct
- [ ] Simulation works

---

## Node Tools (node_tools.py)

### 18. add_blueprint_event_node
**Status:** 🔄 PENDING

**Description:** Add an event node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `event_name` (str, required): Event name (e.g., "ReceiveBeginPlay", "ReceiveTick")
- `node_position` (List[float], optional): [X, Y] position in graph

**Test Cases:**
```python
# Test 1: Add BeginPlay event
add_blueprint_event_node(
    blueprint_name="BP_TestActor",
    event_name="ReceiveBeginPlay"
)

# Test 2: Add Tick event with position
add_blueprint_event_node(
    blueprint_name="BP_TestActor",
    event_name="ReceiveTick",
    node_position=[0, 200]
)
```

**Expected Results:**
- Event node appears in graph
- Node positioned correctly
- Returns node ID

**Actual Results:**
- [ ] Node created
- [ ] Position correct
- [ ] Node ID returned

---

### 19. add_blueprint_function_node
**Status:** 🔄 PENDING

**Description:** Add a function call node to a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `target` (str, required): Target object ("self", component name)
- `function_name` (str, required): Function to call
- `params` (Dict, optional): Parameter values
- `node_position` (List[float], optional): [X, Y] position

**Test Cases:**
```python
# Test 1: Call Print String
add_blueprint_function_node(
    blueprint_name="BP_TestActor",
    target="self",
    function_name="PrintString",
    params={"InString": "Hello World"}
)

# Test 2: Call component function
add_blueprint_function_node(
    blueprint_name="BP_TestActor",
    target="MeshComp",
    function_name="SetVisibility",
    params={"bNewVisibility": True}
)
```

**Expected Results:**
- Function node created
- Parameters set
- Returns node ID

**Actual Results:**
- [ ] Node created
- [ ] Parameters correct
- [ ] Node ID returned

---

### 20. connect_blueprint_nodes
**Status:** 🔄 PENDING

**Description:** Connect two nodes in a Blueprint's event graph.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `source_node_id` (str, required): Source node ID
- `source_pin` (str, required): Output pin name
- `target_node_id` (str, required): Target node ID
- `target_pin` (str, required): Input pin name

**Test Cases:**
```python
# Test 1: Connect event to function
connect_blueprint_nodes(
    blueprint_name="BP_TestActor",
    source_node_id="<event_node_id>",
    source_pin="then",
    target_node_id="<function_node_id>",
    target_pin="execute"
)
```

**Expected Results:**
- Nodes connected with wire
- Execution flows correctly
- Connection visible in graph

**Actual Results:**
- [ ] Connection created
- [ ] Execution works
- [ ] Visible in editor

---

### 21. find_blueprint_nodes
**Status:** 🔄 PENDING

**Description:** Find nodes in a Blueprint's event graph by type or event.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `node_type` (str, optional): Node type filter
- `event_type` (str, optional): Event type filter

**Test Cases:**
```python
# Test 1: Find all event nodes
find_blueprint_nodes(
    blueprint_name="BP_TestActor",
    node_type="Event"
)

# Test 2: Find specific event
find_blueprint_nodes(
    blueprint_name="BP_TestActor",
    event_type="BeginPlay"
)
```

**Expected Results:**
- Returns array of node IDs
- Filtered correctly

**Actual Results:**
- [ ] Correct nodes found
- [ ] Filtering works

---

### 22. add_blueprint_variable
**Status:** 🔄 PENDING

**Description:** Add a variable to a Blueprint class.

**Parameters:**
- `blueprint_name` (str, required): Target Blueprint
- `variable_name` (str, required): Variable name
- `variable_type` (str, required): Type (Boolean, Integer, Float, Vector, etc.)
- `is_exposed` (bool, default=False): Expose to editor

**Test Cases:**
```python
# Test 1: Add private variable
add_blueprint_variable(
    blueprint_name="BP_TestActor",
    variable_name="MyFloat",
    variable_type="Float"
)

# Test 2: Add exposed variable
add_blueprint_variable(
    blueprint_name="BP_TestActor",
    variable_name="MyExposedInt",
    variable_type="Integer",
    is_exposed=True
)
```

**Expected Results:**
- Variable appears in My Blueprint panel
- Correct type
- Exposed flag set correctly

**Actual Results:**
- [ ] Variable created
- [ ] Type correct
- [ ] Exposure works

---

### 22-26. Additional Node Tools
- `add_blueprint_get_self_component_reference`
- `add_blueprint_self_reference`
- `add_blueprint_input_action_node`
- `create_input_mapping`

(Similar structure as above - to be filled in during testing)

---

## UMG/Widget Tools (umg_tools.py)

### 27. create_umg_widget_blueprint
**Status:** 🔄 PENDING

**Description:** Create a new UMG Widget Blueprint for UI.

**Parameters:**
- `widget_name` (str, required): Name for the widget
- `parent_class` (str, default="UserWidget"): Parent widget class
- `path` (str, default="/Game/UI"): Content Browser path

**Test Cases:**
```python
# Test 1: Create basic widget
create_umg_widget_blueprint(widget_name="WBP_MainMenu")

# Test 2: Create in custom path
create_umg_widget_blueprint(
    widget_name="WBP_HUD",
    path="/Game/UI/HUD"
)
```

**Expected Results:**
- Widget Blueprint created
- Appears in Content Browser
- Can be opened in UMG Designer

**Actual Results:**
- [ ] Widget created
- [ ] Correct path
- [ ] Opens in designer

---

### 28. add_text_block_to_widget
**Status:** 🔄 PENDING

**Description:** Add a Text Block widget to a UMG Widget Blueprint.

**Parameters:**
- `widget_name` (str, required): Target widget
- `text_block_name` (str, required): Name for text block
- `text` (str, default=""): Initial text content
- `position` (List[float], default=[0,0]): [X, Y] position
- `size` (List[float], default=[200,50]): [Width, Height]
- `font_size` (int, default=12): Font size
- `color` (List[float], default=[1,1,1,1]): [R, G, B, A] color (0-1)

**Test Cases:**
```python
# Test 1: Basic text block
add_text_block_to_widget(
    widget_name="WBP_MainMenu",
    text_block_name="TitleText",
    text="Main Menu",
    font_size=24
)

# Test 2: Colored text with custom position
add_text_block_to_widget(
    widget_name="WBP_MainMenu",
    text_block_name="SubtitleText",
    text="Press any key",
    position=[100, 100],
    color=[1, 0, 0, 1]  # Red
)
```

**Expected Results:**
- Text block appears in widget
- Text/formatting correct
- Position/size accurate

**Actual Results:**
- [ ] Text block added
- [ ] Formatting correct
- [ ] Transform correct

---

### 29-32. Additional Widget Tools
- `add_button_to_widget`
- `bind_widget_event`
- `set_text_block_binding`
- `add_widget_to_viewport`

(Similar structure - to be filled in during testing)

---

## Project Tools (project_tools.py)

### 33. create_input_mapping
**Status:** 🔄 PENDING

**Description:** Create an input mapping for the project.

**Parameters:**
- `action_name` (str, required): Input action name
- `key` (str, required): Key to bind (e.g., "SpaceBar", "LeftMouseButton")
- `input_type` (str, default="Action"): "Action" or "Axis"

**Test Cases:**
```python
# Test 1: Create action mapping
create_input_mapping(
    action_name="Jump",
    key="SpaceBar"
)

# Test 2: Create axis mapping
create_input_mapping(
    action_name="MoveForward",
    key="W",
    input_type="Axis"
)
```

**Expected Results:**
- Input mapping appears in Project Settings
- Key binding works in game

**Actual Results:**
- [ ] Mapping created
- [ ] Visible in settings
- [ ] Functions in game

---

## Testing Notes

### General Issues Found
- [ ] List any cross-cutting issues
- [ ] Performance problems
- [ ] Connection issues
- [ ] Error handling gaps

### Improvement Suggestions
- [ ] Better parameter validation
- [ ] More detailed error messages
- [ ] Additional return data
- [ ] Missing features

### Documentation Needed
- [ ] Parameter constraints (valid ranges, formats)
- [ ] Return value structure
- [ ] Error codes and meanings
- [ ] Usage examples for complex tools

---

## Summary Statistics

**Total Tools:** 33+
**Tested:** 1
**Passing:** 1
**Failing:** 0
**Skipped:** 1
**Pending:** 31+

**Test Coverage:** ~3%

---

## Next Steps

1. Improve tool docstrings with detailed parameter descriptions
2. Add parameter type hints and validation
3. Test each tool systematically
4. Document edge cases and limitations
5. Create example scripts for common workflows
