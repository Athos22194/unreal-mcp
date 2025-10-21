"""
Editor Tools for Unreal MCP.

This module provides tools for controlling the Unreal Editor viewport and other editor functionality.
"""

import logging
from typing import Dict, List, Any, Optional
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_editor_tools(mcp: FastMCP):
    """Register editor tools with the MCP server."""
    
    @mcp.tool()
    def get_actors_in_level(ctx: Context, max_actors: int = 100) -> List[Dict[str, Any]]:
        """Get a list of actors in the current level.
        
        Args:
            max_actors: Maximum number of actors to return (default: 100). Set to 0 for all actors (use with caution in large levels).
        
        Returns:
            List of actor dictionaries with their properties.
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.warning("Failed to connect to Unreal Engine")
                return []
                
            # Send max_actors parameter to the engine
            response = unreal.send_command("get_actors_in_level", {
                "max_actors": max_actors
            })
            
            if not response:
                logger.warning("No response from Unreal Engine")
                return []
                
            # Log the complete response for debugging
            logger.info(f"Complete response from Unreal: {response}")
            
            # Check response format
            if "result" in response and "actors" in response["result"]:
                actors = response["result"]["actors"]
                logger.info(f"Found {len(actors)} actors in level (max requested: {max_actors})")
                return actors
            elif "actors" in response:
                actors = response["actors"]
                logger.info(f"Found {len(actors)} actors in level (max requested: {max_actors})")
                return actors
                
            logger.warning(f"Unexpected response format: {response}")
            return []
            
        except Exception as e:
            logger.error(f"Error getting actors: {e}")
            return []

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
            
            >>> find_actors_by_name(pattern="NonExistent")
            []
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.warning("Failed to connect to Unreal Engine")
                return []
                
            response = unreal.send_command("find_actors_by_name", {
                "pattern": pattern
            })
            
            if not response:
                return []
                
            actors = response.get("actors", [])
            logger.info(f"Found {len(actors)} actors matching pattern '{pattern}'")
            return actors
            
        except Exception as e:
            logger.error(f"Error finding actors: {e}")
            return []
    
    @mcp.tool()
    def spawn_actor(
        ctx: Context,
        name: str,
        type: str,
        location: List[float] = [0.0, 0.0, 0.0],
        rotation: List[float] = [0.0, 0.0, 0.0]
    ) -> Dict[str, Any]:
        """Create a new actor in the current level.
        
        Args:
            ctx: The MCP context
            name: The name to give the new actor (must be unique)
            type: The type of actor to create (e.g. StaticMeshActor, PointLight)
            location: The [x, y, z] world location to spawn at
            rotation: The [pitch, yaw, roll] rotation in degrees
            
        Returns:
            Dict containing the created actor's properties
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            # Ensure all parameters are properly formatted
            params = {
                "name": name,
                "type": type.upper(),  # Make sure type is uppercase
                "location": location,
                "rotation": rotation
            }
            
            # Validate location and rotation formats
            for param_name in ["location", "rotation"]:
                param_value = params[param_name]
                if not isinstance(param_value, list) or len(param_value) != 3:
                    logger.error(f"Invalid {param_name} format: {param_value}. Must be a list of 3 float values.")
                    return {"success": False, "message": f"Invalid {param_name} format. Must be a list of 3 float values."}
                # Ensure all values are float
                params[param_name] = [float(val) for val in param_value]
            
            logger.info(f"Creating actor '{name}' of type '{type}' with params: {params}")
            response = unreal.send_command("spawn_actor", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            # Log the complete response for debugging
            logger.info(f"Actor creation response: {response}")
            
            # Handle error responses correctly
            if response.get("status") == "error":
                error_message = response.get("error", "Unknown error")
                logger.error(f"Error creating actor: {error_message}")
                return {"success": False, "message": error_message}
            
            return response
            
        except Exception as e:
            error_msg = f"Error creating actor: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def delete_actor(ctx: Context, name: str) -> Dict[str, Any]:
        """Delete an actor by name."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("delete_actor", {
                "name": name
            })
            return response or {}
            
        except Exception as e:
            logger.error(f"Error deleting actor: {e}")
            return {}
    
    @mcp.tool()
    def set_actor_transform(
        ctx: Context,
        name: str,
        location: List[float]  = None,
        rotation: List[float]  = None,
        scale: List[float] = None
    ) -> Dict[str, Any]:
        """Set the transform of an actor."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            params = {"name": name}
            if location is not None:
                params["location"] = location
            if rotation is not None:
                params["rotation"] = rotation
            if scale is not None:
                params["scale"] = scale
                
            response = unreal.send_command("set_actor_transform", params)
            return response or {}
            
        except Exception as e:
            logger.error(f"Error setting transform: {e}")
            return {}
    
    @mcp.tool()
    def get_actor_properties(ctx: Context, name: str) -> Dict[str, Any]:
        """Get all properties of an actor."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("get_actor_properties", {
                "name": name
            })
            return response or {}
            
        except Exception as e:
            logger.error(f"Error getting properties: {e}")
            return {}

    @mcp.tool()
    def set_actor_property(
        ctx: Context,
        name: str,
        property_name: str,
        property_value,
    ) -> Dict[str, Any]:
        """
        Set a property on an actor.
        
        Args:
            name: Name of the actor
            property_name: Name of the property to set
            property_value: Value to set the property to
            
        Returns:
            Dict containing response from Unreal with operation status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("set_actor_property", {
                "name": name,
                "property_name": property_name,
                "property_value": property_value
            })
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Set actor property response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error setting actor property: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    # @mcp.tool() commented out because it's buggy
    def focus_viewport(
        ctx: Context,
        target: str = None,
        location: List[float] = None,
        distance: float = 1000.0,
        orientation: List[float] = None
    ) -> Dict[str, Any]:
        """
        Focus the viewport on a specific actor or location.
        
        Args:
            target: Name of the actor to focus on (if provided, location is ignored)
            location: [X, Y, Z] coordinates to focus on (used if target is None)
            distance: Distance from the target/location
            orientation: Optional [Pitch, Yaw, Roll] for the viewport camera
            
        Returns:
            Response from Unreal Engine
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            params = {}
            if target:
                params["target"] = target
            elif location:
                params["location"] = location
            
            if distance:
                params["distance"] = distance
                
            if orientation:
                params["orientation"] = orientation
                
            response = unreal.send_command("focus_viewport", params)
            return response or {}
            
        except Exception as e:
            logger.error(f"Error focusing viewport: {e}")
            return {"status": "error", "message": str(e)}

    @mcp.tool()
    def spawn_blueprint_actor(
        ctx: Context,
        blueprint_name: str,
        actor_name: str,
        location: List[float] = [0.0, 0.0, 0.0],
        rotation: List[float] = [0.0, 0.0, 0.0]
    ) -> Dict[str, Any]:
        """Spawn an actor from a Blueprint.
        
        Args:
            ctx: The MCP context
            blueprint_name: Name of the Blueprint to spawn from
            actor_name: Name to give the spawned actor
            location: The [x, y, z] world location to spawn at
            rotation: The [pitch, yaw, roll] rotation in degrees
            
        Returns:
            Dict containing the spawned actor's properties
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            # Ensure all parameters are properly formatted
            params = {
                "blueprint_name": blueprint_name,
                "actor_name": actor_name,
                "location": location or [0.0, 0.0, 0.0],
                "rotation": rotation or [0.0, 0.0, 0.0]
            }
            
            # Validate location and rotation formats
            for param_name in ["location", "rotation"]:
                param_value = params[param_name]
                if not isinstance(param_value, list) or len(param_value) != 3:
                    logger.error(f"Invalid {param_name} format: {param_value}. Must be a list of 3 float values.")
                    return {"success": False, "message": f"Invalid {param_name} format. Must be a list of 3 float values."}
                # Ensure all values are float
                params[param_name] = [float(val) for val in param_value]
            
            logger.info(f"Spawning blueprint actor with params: {params}")
            response = unreal.send_command("spawn_blueprint_actor", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Spawn blueprint actor response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error spawning blueprint actor: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def get_console_output(
        ctx: Context, 
        max_lines: int = 500,
        severity: str = "All",
        category: str = ""
    ) -> Dict[str, Any]:
        """Get recent console output from the Unreal Editor.
        
        Retrieves log messages from the Unreal Editor's output log with optional filtering.
        Useful for debugging, monitoring operations, and checking for errors or warnings.
        
        Args:
            max_lines: Maximum number of log lines to return (default: 500).
                      Higher values return more history but may impact performance.
                      Examples: 100, 500, 1000
                      
            severity: Filter by severity level (default: "All").
                     Options: "All", "Display", "Warning", "Error"
                     Examples:
                     - "All": Returns all log messages
                     - "Error": Returns only error messages
                     - "Warning": Returns warnings and errors
                     
            category: Filter by log category (default: "" for all categories).
                     Common categories: "LogTemp", "LogBlueprint", "LogPython", "LogActor"
                     Examples:
                     - "": All categories
                     - "LogTemp": Only messages logged via UE_LOG(LogTemp, ...)
                     - "LogBlueprint": Only Blueprint-related messages
        
        Returns:
            Dict containing:
            - logs: List of log entry dictionaries with:
              - timestamp: When the message was logged
              - category: Log category (LogTemp, LogBlueprint, etc.)
              - severity: Message severity (Display, Warning, Error)
              - message: The actual log message text
            - count: Number of log entries returned
            - max_lines: The maximum lines that were requested
            - severity_filter: The severity filter that was applied
            - category_filter: The category filter that was applied
        
        Examples:
            # Get last 500 log messages (default)
            >>> get_console_output()
            
            # Get last 1000 log messages
            >>> get_console_output(max_lines=1000)
            
            # Get only error messages
            >>> get_console_output(severity="Error")
            
            # Get only Blueprint-related warnings and errors
            >>> get_console_output(category="LogBlueprint", severity="Warning")
            
            # Get recent LogTemp messages for debugging
            >>> get_console_output(max_lines=100, category="LogTemp")
        
        Note:
            This tool captures log messages from the editor's output log.
            For real-time monitoring, consider calling this tool periodically.
            The editor must be running for this tool to work.
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.warning("Failed to connect to Unreal Engine")
                return {
                    "success": False,
                    "message": "Failed to connect to Unreal Engine",
                    "logs": [],
                    "count": 0
                }
            
            params = {
                "max_lines": max_lines,
                "severity": severity,
                "category": category
            }
            
            logger.info(f"Getting console output with params: {params}")
            response = unreal.send_command("get_console_output", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {
                    "success": False,
                    "message": "No response from Unreal Engine",
                    "logs": [],
                    "count": 0
                }
            
            logger.info(f"Console output response: {response}")
            
            # Extract the result if it's nested
            if "result" in response:
                return response["result"]
            
            return response
            
        except Exception as e:
            error_msg = f"Error getting console output: {e}"
            logger.error(error_msg)
            return {
                "success": False,
                "message": error_msg,
                "logs": [],
                "count": 0
            }

    logger.info("Editor tools registered successfully")

