"""
Blueprint Introspection Tools for Unreal MCP.

This module provides tools for extracting complete Blueprint data including
metadata, components, variables, functions, and event graphs.
"""

import logging
from typing import Dict, Any
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_blueprint_introspection_tools(mcp: FastMCP):
    """Register Blueprint introspection tools with the MCP server."""
    
    @mcp.tool()
    def get_blueprint_data(
        ctx: Context,
        blueprint_name: str
    ) -> Dict[str, Any]:
        """
        Get complete Blueprint data including metadata, components, and variables.
        
        This tool retrieves comprehensive information about a Blueprint asset,
        including its basic information, component hierarchy, and variable definitions.
        
        Args:
            blueprint_name: Name of the Blueprint to inspect (e.g., "BP_MyActor")
                           Can be just the name or the full path
            
        Returns:
            Dict containing:
            - success (bool): Whether the operation succeeded
            - blueprint_info (dict): Basic Blueprint metadata
              - name (str): Blueprint name
              - path (str): Full asset path
              - parent_class (str): Parent class name
              - blueprint_type (str): Type of Blueprint
              - description (str): Blueprint description
              - category (str): Blueprint category
            - components (list): Array of component data
              - name (str): Component variable name
              - type (str): Component class type
              - parent_component (str): Parent component name
              - transform (dict): Location, rotation, and scale
            - variables (list): Array of Blueprint variables
              - name (str): Variable name
              - type (str): Variable type
              - category (str): Variable category
              - friendly_name (str): Display name/tooltip
              - is_exposed (bool): Exposed on spawn
              - is_blueprint_read_only (bool): Read-only flag
              - replication (str): Replication mode
              - default_value (str): Default value
        
        Examples:
            >>> # Get data for a simple Blueprint
            >>> result = get_blueprint_data(blueprint_name="BP_TestActor")
            >>> if result["success"]:
            ...     info = result["blueprint_info"]
            ...     print(f"Blueprint: {info['name']}")
            ...     print(f"Parent: {info['parent_class']}")
            ...     print(f"Components: {len(result['components'])}")
            ...     print(f"Variables: {len(result['variables'])}")
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "error": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Getting Blueprint data for '{blueprint_name}'")
            response = unreal.send_command("get_blueprint_data", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "error": "No response from Unreal Engine"}
            
            logger.info(f"Blueprint data retrieval response: {response.get('success', False)}")
            return response
            
        except Exception as e:
            error_msg = f"Error getting Blueprint data: {e}"
            logger.error(error_msg)
            return {"success": False, "error": error_msg}
