# DMusicPak Interactive Build Script for Windows (PowerShell)
# Similar to Vue CLI - interactive feature selection

param(
    [switch]$NonInteractive
)

# Colors for output
$ErrorColor = "Red"
$SuccessColor = "Green"
$InfoColor = "Cyan"
$WarningColor = "Yellow"
$MagentaColor = "Magenta"

# Function to print header
function Print-Header {
    Write-Host ""
    Write-Host "╔════════════════════════════════════════╗" -ForegroundColor $InfoColor
    Write-Host "║     DMusicPak Build Configuration     ║" -ForegroundColor $InfoColor
    Write-Host "╚════════════════════════════════════════╝" -ForegroundColor $InfoColor
    Write-Host ""
}

# Function to print section
function Print-Section {
    param([string]$Text)
    Write-Host "▶ $Text" -ForegroundColor $InfoColor -NoNewline
    Write-Host ""
}

# Function to print selected
function Print-Selected {
    param([string]$Text)
    Write-Host "  ✓ " -ForegroundColor $SuccessColor -NoNewline
    Write-Host $Text
}

# Function to ask yes/no with default
function Ask-YesNo {
    param(
        [string]$Prompt,
        [string]$Default = "n"
    )
    
    if ($NonInteractive) {
        return $Default -eq "y"
    }
    
    $choices = @()
    if ($Default -eq "y") {
        $choices = @("&Yes", "&No")
        $defaultIndex = 0
    } else {
        $choices = @("&Yes", "&No")
        $defaultIndex = 1
    }
    
    $choice = $Host.UI.PromptForChoice("", $Prompt, $choices, $defaultIndex)
    return $choice -eq 0
}

# Function to select from menu
function Select-Option {
    param(
        [string]$Prompt,
        [string[]]$Options
    )
    
    if ($NonInteractive) {
        return 0
    }
    
    $selected = 0
    
    while ($true) {
        Clear-Host
        Print-Header
        Write-Host $Prompt -ForegroundColor White
        Write-Host ""
        
        for ($i = 0; $i -lt $Options.Length; $i++) {
            if ($i -eq $selected) {
                Write-Host "  ▶ " -ForegroundColor $SuccessColor -NoNewline
                Write-Host $Options[$i] -ForegroundColor $SuccessColor
            } else {
                Write-Host "    $($Options[$i])"
            }
        }
        
        Write-Host ""
        Write-Host "Use ↑↓ or number keys to navigate, Enter to select" -ForegroundColor $InfoColor
        
        # Try to read key with better compatibility
        try {
            $key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
            
            # Check for number keys (1-9)
            if ($key.Character -match '^[1-9]$') {
                $num = [int]$key.Character
                if ($num -le $Options.Length) {
                    return $num - 1
                }
            }
            
            # Check for arrow keys
            switch ($key.VirtualKeyCode) {
                38 { # Up arrow
                    $selected = if ($selected -eq 0) { $Options.Length - 1 } else { $selected - 1 }
                }
                40 { # Down arrow
                    $selected = if ($selected -eq $Options.Length - 1) { 0 } else { $selected + 1 }
                }
                13 { # Enter
                    return $selected
                }
            }
        } catch {
            # Fallback: use simple number input
            Write-Host ""
            Write-Host "Enter option number (1-$($Options.Length)): " -ForegroundColor $InfoColor -NoNewline
            $input = Read-Host
            if ($input -match '^\d+$' -and [int]$input -ge 1 -and [int]$input -le $Options.Length) {
                return [int]$input - 1
            }
        }
    }
}

# Start interactive configuration
Clear-Host
Print-Header

# Check for CMake
try {
    $cmakeVersion = & cmake --version 2>&1 | Select-Object -First 1
    Write-Host "✓ CMake found: $cmakeVersion" -ForegroundColor $SuccessColor
} catch {
    Write-Host "Error: CMake is not installed or not in PATH" -ForegroundColor $ErrorColor
    Write-Host "Please install CMake from: https://cmake.org/download/" -ForegroundColor $WarningColor
    exit 1
}

# Check for Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "✓ Visual Studio found" -ForegroundColor $SuccessColor
    }
} else {
    Write-Host "⚠ Warning: Visual Studio detection tool not found" -ForegroundColor $WarningColor
}

Write-Host ""

# Configuration options
Print-Section "Build Configuration"

# Build Type
$buildTypeOptions = @("Release", "Debug", "MinSizeRel", "RelWithDebInfo")
$buildTypeIndex = Select-Option "Select Build Type:" $buildTypeOptions
$BUILD_TYPE = $buildTypeOptions[$buildTypeIndex]
Print-Selected "Build Type: $BUILD_TYPE"
Write-Host ""

# Network Support
Print-Section "Feature Selection"
if (Ask-YesNo "Enable Network Streaming Support (requires libcurl)?" "n") {
    $ENABLE_NETWORK = "ON"
    Print-Selected "Network Streaming: Enabled"
    
    # Check for libcurl (basic check)
    $curlFound = $false
    $possiblePaths = @(
        "${env:ProgramFiles}\curl\bin\libcurl.dll",
        "${env:ProgramFiles(x86)}\curl\bin\libcurl.dll",
        "$env:USERPROFILE\vcpkg\installed\x64-windows\bin\curl.dll"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $curlFound = $true
            break
        }
    }
    
    if (-not $curlFound) {
        Write-Host "  ⚠ Warning: libcurl may not be installed" -ForegroundColor $WarningColor
        Write-Host "    Install with vcpkg: vcpkg install curl" -ForegroundColor $WarningColor
        if (-not (Ask-YesNo "Continue anyway?" "y")) {
            exit 1
        }
    }
} else {
    $ENABLE_NETWORK = "OFF"
    Print-Selected "Network Streaming: Disabled"
}
Write-Host ""

# Build Examples
if (Ask-YesNo "Build Example Programs?" "y") {
    $BUILD_EXAMPLES = "ON"
    Print-Selected "Examples: Enabled"
} else {
    $BUILD_EXAMPLES = "OFF"
    Print-Selected "Examples: Disabled"
}
Write-Host ""

# Build Tests
if (Ask-YesNo "Build Test Programs?" "n") {
    $BUILD_TESTS = "ON"
    Print-Selected "Tests: Enabled"
} else {
    $BUILD_TESTS = "OFF"
    Print-Selected "Tests: Disabled"
}
Write-Host ""

# Build Shared Libraries
if (Ask-YesNo "Build Shared Libraries (DLL)?" "y") {
    $BUILD_SHARED_LIBS = "ON"
    Print-Selected "Shared Libraries: Enabled"
} else {
    $BUILD_SHARED_LIBS = "OFF"
    Print-Selected "Shared Libraries: Disabled (Static)"
}
Write-Host ""

# Generator Selection
Print-Section "Visual Studio Generator"
$generatorOptions = @(
    "Visual Studio 17 2022",
    "Visual Studio 16 2019",
    "Visual Studio 15 2017",
    "Visual Studio 14 2015",
    "MinGW Makefiles",
    "Ninja"
)
$generatorIndex = Select-Option "Select Generator:" $generatorOptions
$GENERATOR = $generatorOptions[$generatorIndex]
Print-Selected "Generator: $GENERATOR"
Write-Host ""

# Install Prefix (optional)
Print-Section "Installation (Optional)"
$INSTALL_PREFIX = Read-Host "Install Prefix [default: C:\Program Files\DMusicPak]"
if ([string]::IsNullOrWhiteSpace($INSTALL_PREFIX)) {
    $INSTALL_PREFIX = "C:\Program Files\DMusicPak"
}
Print-Selected "Install Prefix: $INSTALL_PREFIX"
Write-Host ""

# Summary
Print-Section "Configuration Summary"
Write-Host "  Build Type:        $BUILD_TYPE"
Write-Host "  Network Support:   $ENABLE_NETWORK"
Write-Host "  Build Examples:    $BUILD_EXAMPLES"
Write-Host "  Build Tests:       $BUILD_TESTS"
Write-Host "  Shared Libraries:  $BUILD_SHARED_LIBS"
Write-Host "  Generator:         $GENERATOR"
Write-Host "  Install Prefix:    $INSTALL_PREFIX"
Write-Host ""

if (-not (Ask-YesNo "Proceed with build?" "y")) {
    Write-Host "Build cancelled." -ForegroundColor $WarningColor
    exit 0
}

# Build directory
$BuildDir = "build"

# Create build directory
Write-Host ""
Print-Section "Preparing Build"
Write-Host "Creating build directory..." -ForegroundColor $InfoColor
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# Configure with CMake
Write-Host "Configuring project with CMake..." -ForegroundColor $InfoColor
Push-Location $BuildDir

try {
    $cmakeArgs = @(
        ".."
        "-G", $GENERATOR
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
        "-DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS"
        "-DBUILD_EXAMPLES=$BUILD_EXAMPLES"
        "-DBUILD_TESTS=$BUILD_TESTS"
        "-DENABLE_NETWORK=$ENABLE_NETWORK"
    )
    
    & cmake $cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host ""
    Print-Section "Building Project"
    Write-Host "Compiling..." -ForegroundColor $InfoColor
    
    # Build
    & cmake --build . --config $BUILD_TYPE
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    Write-Host ""
    Write-Host "╔════════════════════════════════════════╗" -ForegroundColor $SuccessColor
    Write-Host "║        Build Successful! ✓            ║" -ForegroundColor $SuccessColor
    Write-Host "╚════════════════════════════════════════╝" -ForegroundColor $SuccessColor
    Write-Host ""
    
    Print-Section "Build Output"
    $dllPath = "lib\$BUILD_TYPE\dmusicpak.dll"
    $libPath = "lib\$BUILD_TYPE\dmusicpak.lib"
    
    if (Test-Path $dllPath) {
        Write-Host "  ✓ DLL: $BuildDir\$dllPath" -ForegroundColor $SuccessColor
    }
    if (Test-Path $libPath) {
        Write-Host "  ✓ LIB: $BuildDir\$libPath" -ForegroundColor $SuccessColor
    }
    
    if ($BUILD_EXAMPLES -eq "ON") {
        Write-Host ""
        Write-Host "Examples:" -ForegroundColor $InfoColor
        $exampleDir = "bin\$BUILD_TYPE"
        if (Test-Path $exampleDir) {
            Get-ChildItem $exampleDir -Filter "*.exe" | ForEach-Object {
                Write-Host "  ✓ $($_.Name)" -ForegroundColor $SuccessColor
            }
        }
    }
    
    Write-Host ""
    Print-Section "Next Steps"
    if ($BUILD_EXAMPLES -eq "ON") {
        Write-Host "  Run examples:"
        Write-Host "    cd $BuildDir\bin\$BUILD_TYPE"
        Write-Host "    .\example_write.exe"
        Write-Host "    .\example_read.exe example.dmusicpak"
        if ($ENABLE_NETWORK -eq "ON") {
            Write-Host "    .\example_network.exe"
        }
        Write-Host ""
    }
    
    Write-Host "  Install library:"
    Write-Host "    cmake --install . --config $BUILD_TYPE"
    Write-Host ""
    
    # Offer to run examples
    if ($BUILD_EXAMPLES -eq "ON") {
        if (Ask-YesNo "Run write example now?" "n") {
            Push-Location "bin\$BUILD_TYPE"
            if (Test-Path ".\example_write.exe") {
                & .\example_write.exe
                Write-Host ""
                Write-Host "✓ Example package created: example.dmusicpak" -ForegroundColor $SuccessColor
                
                if (Ask-YesNo "Run read example?" "n") {
                    if (Test-Path ".\example_read.exe") {
                        & .\example_read.exe example.dmusicpak
                    }
                }
            }
            Pop-Location
        }
    }
    
    Write-Host ""
    Write-Host "Build completed successfully!" -ForegroundColor $SuccessColor
    
} catch {
    Write-Host ""
    Write-Host "╔════════════════════════════════════════╗" -ForegroundColor $ErrorColor
    Write-Host "║          Build Failed! ✗              ║" -ForegroundColor $ErrorColor
    Write-Host "╚════════════════════════════════════════╝" -ForegroundColor $ErrorColor
    Write-Host "Error: $_" -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

Pop-Location
