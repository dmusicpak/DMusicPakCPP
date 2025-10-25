# DMusicPak Build Script for Windows (PowerShell)
# Automatically builds the library and examples

param(
    [string]$BuildType = "Release",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$InstallPrefix = "C:\Program Files\DMusicPak"
)

# Colors for output
$ErrorColor = "Red"
$SuccessColor = "Green"
$InfoColor = "Cyan"
$WarningColor = "Yellow"

Write-Host "========================================" -ForegroundColor $InfoColor
Write-Host "  DMusicPak Build Script for Windows" -ForegroundColor $InfoColor
Write-Host "========================================" -ForegroundColor $InfoColor
Write-Host ""

# Configuration
$BuildDir = "build"

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
        Write-Host "✓ Visual Studio found: $vsPath" -ForegroundColor $SuccessColor
    }
} else {
    Write-Host "Warning: Visual Studio detection tool not found" -ForegroundColor $WarningColor
    Write-Host "Make sure Visual Studio is installed with C++ development tools" -ForegroundColor $WarningColor
}

Write-Host ""
Write-Host "Configuration:" -ForegroundColor $WarningColor
Write-Host "  Build Type:      $BuildType"
Write-Host "  Build Directory: $BuildDir"
Write-Host "  Generator:       $Generator"
Write-Host "  Install Prefix:  $InstallPrefix"
Write-Host ""

# Create build directory
Write-Host "Creating build directory..." -ForegroundColor $InfoColor
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# Configure with CMake
Write-Host "Configuring project..." -ForegroundColor $InfoColor
Push-Location $BuildDir

try {
    & cmake .. `
        -G $Generator `
        -DCMAKE_BUILD_TYPE="$BuildType" `
        -DCMAKE_INSTALL_PREFIX="$InstallPrefix" `
        -DBUILD_SHARED_LIBS=ON `
        -DBUILD_EXAMPLES=ON `
        -DBUILD_TESTS=OFF

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    Write-Host ""
    Write-Host "Building project..." -ForegroundColor $InfoColor

    # Build
    & cmake --build . --config $BuildType

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor $SuccessColor
    Write-Host "  Build Successful!" -ForegroundColor $SuccessColor
    Write-Host "========================================" -ForegroundColor $SuccessColor
    Write-Host ""
    Write-Host "Library location:"
    Write-Host "  $BuildDir\lib\$BuildType\dmusicpak.dll"
    Write-Host ""
    Write-Host "Examples built:"
    Write-Host "  $BuildDir\bin\$BuildType\write_example.exe"
    Write-Host "  $BuildDir\bin\$BuildType\read_example.exe"
    Write-Host "  $BuildDir\bin\$BuildType\stream_example.exe"
    Write-Host ""
    Write-Host "To run examples:"
    Write-Host "  cd $BuildDir\bin\$BuildType"
    Write-Host "  .\write_example.exe"
    Write-Host "  .\read_example.exe example.dmusicpak"
    Write-Host "  .\stream_example.exe example.dmusicpak"
    Write-Host ""
    Write-Host "To install (requires admin privileges):"
    Write-Host "  cmake --install . --config $BuildType"
    Write-Host ""

    # Offer to run example
    $response = Read-Host "Run write example? (y/n)"
    if ($response -eq "y" -or $response -eq "Y") {
        Push-Location "bin\$BuildType"
        & .\write_example.exe
        Write-Host ""
        Write-Host "✓ Example package created: example.dmusicpak" -ForegroundColor $SuccessColor

        $response2 = Read-Host "Run read example? (y/n)"
        if ($response2 -eq "y" -or $response2 -eq "Y") {
            & .\read_example.exe example.dmusicpak
        }
        Pop-Location
    }

    Write-Host ""
    Write-Host "Build script completed successfully!" -ForegroundColor $SuccessColor

} catch {
    Write-Host ""
    Write-Host "Build failed: $_" -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

Pop-Location