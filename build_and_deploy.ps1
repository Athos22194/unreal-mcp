# Build and Deploy UnrealMCP Plugin
# This is the ONE script you need - builds plugin and deploys to your target project
# Usage: .\build_and_deploy.ps1
# After running: Restart Unreal Editor and manually restart MCP server

param(
    [string]$TargetProject = "D:\Phantom5\Phantom-OS",
    [string]$UEPath = ""
)

$ErrorActionPreference = "Stop"

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  UnrealMCP Plugin - Build & Deploy" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Paths
$PluginSource = "$PSScriptRoot\MCPGameProject\Plugins\UnrealMCP"
$PluginTarget = "$TargetProject\Plugins\UnrealMCP"

# Verify paths
if (-not (Test-Path $PluginSource)) {
    Write-Error "Plugin source not found at: $PluginSource"
    exit 1
}

if (-not (Test-Path $TargetProject)) {
    Write-Error "Target project not found at: $TargetProject"
    exit 1
}

# Auto-detect UE installation
if ([string]::IsNullOrEmpty($UEPath)) {
    $PathsToCheck = @(
        "E:\epic\hell\UE_5.5",
        "D:\UE_5.5",
        "C:\Program Files\Epic Games\UE_5.5",
        "D:\Epic Games\UE_5.5",
        "C:\UE_5.5"
    )
    
    foreach ($path in $PathsToCheck) {
        if (Test-Path "$path\Engine\Build\BatchFiles\RunUAT.bat") {
            $UEPath = $path
            Write-Host "Found Unreal Engine: $UEPath" -ForegroundColor Green
            break
        }
    }
    
    if ([string]::IsNullOrEmpty($UEPath)) {
        Write-Error "Could not find Unreal Engine. Please specify -UEPath parameter."
        exit 1
    }
}

$UAT = "$UEPath\Engine\Build\BatchFiles\RunUAT.bat"
if (-not (Test-Path $UAT)) {
    Write-Error "Unreal Automation Tool not found at: $UAT"
    exit 1
}

Write-Host "Target Project: $TargetProject" -ForegroundColor Green
Write-Host ""

# STEP 1: Build the plugin
Write-Host "[1/2] Building plugin..." -ForegroundColor Cyan
Write-Host "---------------------------------------------" -ForegroundColor Gray

$BuildOutput = "$PSScriptRoot\Build\UnrealMCP"

& $UAT BuildPlugin `
    -Plugin="$PluginSource\UnrealMCP.uplugin" `
    -Package="$BuildOutput" `
    -TargetPlatforms=Win64 `
    -VS2022

if ($LASTEXITCODE -ne 0) {
    Write-Error "Plugin build failed!"
    exit 1
}

Write-Host ""
Write-Host "Plugin built successfully!" -ForegroundColor Green
Write-Host ""

# STEP 2: Deploy to target project
Write-Host "[2/2] Deploying to target project..." -ForegroundColor Cyan
Write-Host "---------------------------------------------" -ForegroundColor Gray

# Check if editor is running
$editorProcess = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue
if ($editorProcess) {
    Write-Host ""
    Write-Host "WARNING: Unreal Editor is running!" -ForegroundColor Yellow
    Write-Host "Please close the editor and press Enter to continue..." -ForegroundColor Yellow
    Read-Host
    
    # Check again
    $editorProcess = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue
    if ($editorProcess) {
        Write-Error "Editor is still running. Please close it and try again."
        exit 1
    }
}

# Remove old plugin files
if (Test-Path $PluginTarget) {
    Write-Host "Removing old plugin files..." -ForegroundColor Gray
    Remove-Item -Path "$PluginTarget\*" -Recurse -Force -ErrorAction SilentlyContinue
} else {
    Write-Host "Creating plugin directory..." -ForegroundColor Gray
    New-Item -ItemType Directory -Path $PluginTarget -Force | Out-Null
}

# Copy built plugin
Write-Host "Copying plugin files..." -ForegroundColor Gray
Copy-Item -Path "$BuildOutput\*" -Destination $PluginTarget -Recurse -Force

Write-Host ""
Write-Host "Plugin deployed successfully!" -ForegroundColor Green
Write-Host ""

# Final instructions
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  Build & Deploy Complete!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "  1. Open/Restart Unreal Editor with your project" -ForegroundColor White
Write-Host "  2. Manually restart MCP server (mcp.json config)" -ForegroundColor White
Write-Host "  3. Plugin will be loaded automatically" -ForegroundColor White
Write-Host ""
Write-Host "Plugin deployed to: $PluginTarget" -ForegroundColor Gray
Write-Host ""
