param(
    [string]$ReportDirectory = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "reports")
)

$ErrorActionPreference = "Stop"
$report = Join-Path $ReportDirectory ("local-rehearsal-{0}.txt" -f [DateTime]::Now.ToString("yyyyMMdd-HHmmss"))
New-Item -ItemType Directory -Path $ReportDirectory -Force | Out-Null

try {
    & (Join-Path $PSScriptRoot "smoke-test.ps1") *>&1 | Tee-Object -FilePath $report
    if ($LASTEXITCODE -ne 0) { throw "Smoke test exited with code $LASTEXITCODE" }
    Add-Content -LiteralPath $report -Value "RESULT: PASS"
    Write-Host "Rehearsal report: $report"
} catch {
    Add-Content -LiteralPath $report -Value "RESULT: FAIL"
    Add-Content -LiteralPath $report -Value $_.Exception.Message
    Write-Host "Rehearsal report: $report"
    throw
}
