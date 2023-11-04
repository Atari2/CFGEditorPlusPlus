function SetupMSVC {
    param (
        [PSDefaultValue(Help = "x64")]
        [string]$Arch = "x64",
        [PSDefaultValue(Help = "false")]
        [switch]$Pre
    )

    $moduleAlreadyLoaded = Get-Module Microsoft.VisualStudio.DevShell
    if ($moduleAlreadyLoaded) {
        Write-Error "Module with the same name already loaded, cannot load another one because there will be assemblies conflicts"
        return
    }
    $assemblyPresent = [System.AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.FullName -like '*Microsoft.VisualStudio.DevShell*' }
    if ($assemblyPresent) {
        Write-Error "Assembly with the same name already loaded, cannot load another one because there will be assemblies conflicts"
        return
    }

    if ($Pre) {
        $vsPath = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -prerelease -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationpath
    }
    else {
        $vsPath = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationpath
    }
    $commonLocation = "$vsPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
    if (Test-Path $commonLocation) {
        $dllPath = $commonLocation
    }
    else {
        $dllPath = (Get-ChildItem $vsPath -Recurse -File -Filter Microsoft.VisualStudio.DevShell.dll).FullName
    }
    Import-Module -Force $dllPath
    Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments "-arch=$Arch"
}

function FindQt6 {
    $versions = Get-ChildItem -Path "C:\Qt" -Directory -Filter "6.*" -Attributes !ReparsePoint | Select-Object -ExpandProperty Name | Sort-Object -Descending
    if ($versions.Length -eq 0) {
        Write-Error "No Qt6 installation found"
        return
    }
    $qtPath = "C:\Qt\$($versions[0])\msvc2019_64"
    if (Test-Path $qtPath) {
        return $qtPath
    }
    else {
        Write-Error "No Qt6 installation found"
        return
    }
}

SetupMSVC
$qtPath = FindQt6
if (!$qtPath) {
    return
}
Write-Output "Using Qt6 from $qtPath"
cmake.exe -S . -B build -GNinja -DCMAKE_BUILD_TYPE:String=Release -DCMAKE_PREFIX_PATH:STRING=$qtPath -DCMAKE_C_COMPILER:STRING=cl.exe -DCMAKE_CXX_COMPILER:STRING=cl.exe
Set-Location build
cmake.exe --build . --target all
