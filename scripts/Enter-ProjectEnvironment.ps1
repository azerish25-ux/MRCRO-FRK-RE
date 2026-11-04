# Dot-source this script before local analysis commands.
$taskProjectRoot = Split-Path -Parent $PSScriptRoot
$taskStorageRoot = Split-Path -Parent $taskProjectRoot
if ([System.IO.Path]::GetPathRoot($taskStorageRoot) -ne 'D:\') {
    throw 'Project-controlled files must stay on D:.'
}
$taskLocalRoot = Join-Path $taskStorageRoot 'local'
$taskEnvironment = @{
    TEMP = "$taskLocalRoot\tmp"
    TMP = "$taskLocalRoot\tmp"
    TMPDIR = "$taskLocalRoot\tmp"
    npm_config_cache = "$taskLocalRoot\cache\npm"
    npm_config_prefix = "$taskLocalRoot\tools\npm"
    PIP_CACHE_DIR = "$taskLocalRoot\cache\pip"
    XDG_CACHE_HOME = "$taskLocalRoot\cache\xdg"
    XDG_DATA_HOME = "$taskLocalRoot\data\xdg"
    GRADLE_USER_HOME = "$taskLocalRoot\cache\gradle"
    PYTHONPYCACHEPREFIX = "$taskLocalRoot\cache\pycache"
}
foreach ($taskEntry in $taskEnvironment.GetEnumerator()) {
    New-Item -ItemType Directory -Force -Path $taskEntry.Value | Out-Null
    [Environment]::SetEnvironmentVariable($taskEntry.Key, $taskEntry.Value, 'Process')
}
$env:PYTHONDONTWRITEBYTECODE = '1'
$env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'
$env:POWERSHELL_TELEMETRY_OPTOUT = '1'
$taskNodeDirectories = @(Get-ChildItem -LiteralPath "$taskLocalRoot\tools" -Directory -Filter 'node-v*-win-x64' | Sort-Object Name -Descending)
if ($taskNodeDirectories.Count -gt 0) {
    $env:PATH = $taskNodeDirectories[0].FullName + ';' + $env:PATH
}
$taskGhidraDirectory = Join-Path $taskLocalRoot 'tools\ghidra_12.1.4_PUBLIC'
$taskJdkDirectory = Join-Path $taskLocalRoot 'tools\jdk-21.0.13+9'
if (Test-Path -LiteralPath $taskGhidraDirectory) {
    $env:GHIDRA_INSTALL_DIR = $taskGhidraDirectory
}
if (Test-Path -LiteralPath $taskJdkDirectory) {
    $env:JAVA_HOME = $taskJdkDirectory
}
Write-Output "Project storage: $taskStorageRoot"
