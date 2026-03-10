param(
  [string]$ServiceName = 'OnAirBridge',
  [string]$DisplayName = 'OnAir Bridge',
  [string]$Description = 'FDOnAir <-> Companion bridge',
  [string]$ExePath = "$(Join-Path $PSScriptRoot '..\build\Release\onAir2Companion.exe')",
  [string]$NssmPath = 'nssm.exe',
  [switch]$StartNow
)

$resolvedExe = (Resolve-Path $ExePath).Path
$appDir = Split-Path $resolvedExe -Parent

if (-not (Get-Command $NssmPath -ErrorAction SilentlyContinue)) {
  throw "nssm.exe not found. Pass -NssmPath <path-to-nssm.exe> or add nssm to PATH."
}

& $NssmPath install $ServiceName $resolvedExe
& $NssmPath set $ServiceName DisplayName $DisplayName
& $NssmPath set $ServiceName Description $Description
& $NssmPath set $ServiceName AppDirectory $appDir
& $NssmPath set $ServiceName Start SERVICE_AUTO_START
& $NssmPath set $ServiceName AppStdout (Join-Path $appDir 'bridge-service.out.log')
& $NssmPath set $ServiceName AppStderr (Join-Path $appDir 'bridge-service.err.log')

if ($StartNow) {
  sc.exe start $ServiceName | Out-Null
}

Write-Host "Service '$ServiceName' installed via NSSM."
Write-Host "Executable: $resolvedExe"
