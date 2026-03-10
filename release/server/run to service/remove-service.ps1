param(
  [string]$ServiceName = 'OnAirBridge'
)

sc.exe stop $ServiceName | Out-Null
sc.exe delete $ServiceName

Write-Host "Service '$ServiceName' removed."
