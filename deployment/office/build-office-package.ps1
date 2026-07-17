param(
    [string]$Configuration = "Release",
    [string]$OutputDirectory = (Join-Path $PSScriptRoot "artifacts"),
    [string]$ApiKeyFile = ""
)

$ErrorActionPreference = "Stop"
$repositoryRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$pluginRoot = Join-Path $repositoryRoot "integrations\SecureZone.XProtectPlugin"
$project = Join-Path $pluginRoot "SecureZone.XProtectPlugin.csproj"
$contractProject = Join-Path $repositoryRoot "integrations\SecureZone.XProtectPlugin.ContractTests\SecureZone.XProtectPlugin.ContractTests.csproj"
$msbuild = Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio" -Filter MSBuild.exe -Recurse -ErrorAction SilentlyContinue |
    Where-Object FullName -Match "MSBuild\\Current\\Bin\\MSBuild.exe$" |
    Select-Object -First 1 -ExpandProperty FullName
if (-not $msbuild) { throw "MSBuild.exe was not found. Install Visual Studio Build Tools with .NET Framework support." }

& $msbuild $project /t:Restore,Build /p:Configuration=$Configuration /m
if ($LASTEXITCODE -ne 0) { throw "SecureZone XProtect plug-in build failed." }
& $msbuild $contractProject /t:Restore,Build /p:Configuration=$Configuration /m
if ($LASTEXITCODE -ne 0) { throw "SecureZone XProtect contract test build failed." }
$contractExecutable = Join-Path (Split-Path -Parent $contractProject) "bin\$Configuration\SecureZone.XProtectPlugin.ContractTests.exe"
& $contractExecutable
if ($LASTEXITCODE -ne 0) { throw "SecureZone XProtect contract tests failed." }

$dll = Join-Path $pluginRoot "bin\$Configuration\SecureZone.XProtectPlugin.dll"
$version = [Diagnostics.FileVersionInfo]::GetVersionInfo($dll).FileVersion
$packageName = "SecureZone-XProtectPlugin-v$version"
$package = Join-Path $OutputDirectory $packageName
if (Test-Path -LiteralPath $package) { Remove-Item -LiteralPath $package -Recurse -Force }
New-Item -ItemType Directory -Path $package -Force | Out-Null

Copy-Item -LiteralPath $dll -Destination $package
Copy-Item -LiteralPath (Join-Path $pluginRoot "plugin.def") -Destination $package
Copy-Item -LiteralPath (Join-Path $pluginRoot "install-on-xprotect-server.ps1") -Destination $package
foreach ($file in @("install-office-plugin.ps1", "preflight-office.ps1", "rollback-office-plugin.ps1", "OFFICE-INSTALL.txt", "XProtect-RULES.txt")) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $file) -Destination $package
}
Set-Content -LiteralPath (Join-Path $package "securezone-api-key.txt.example") `
    -Value "PLACE THE PRIVATE API KEY HERE AND RENAME THIS FILE TO securezone-api-key.txt" -Encoding ASCII
if (-not [string]::IsNullOrWhiteSpace($ApiKeyFile)) {
    if (-not (Test-Path -LiteralPath $ApiKeyFile -PathType Leaf)) {
        throw "API key file was not found: $ApiKeyFile"
    }
    $key = (Get-Content -LiteralPath $ApiKeyFile -Raw).Trim()
    if ($key.Length -lt 32) { throw "API key must contain at least 32 characters." }
    Set-Content -LiteralPath (Join-Path $package "securezone-api-key.txt") -Value $key -Encoding ASCII -NoNewline
}

$hashLines = Get-ChildItem -LiteralPath $package -File |
    Where-Object Name -ne "SHA256SUMS.txt" |
    Sort-Object Name |
    ForEach-Object { "{0}  {1}" -f (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash, $_.Name }
$hashLines | Set-Content -LiteralPath (Join-Path $package "SHA256SUMS.txt") -Encoding ASCII

$zip = "$package.zip"
if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force }
Compress-Archive -Path (Join-Path $package "*") -DestinationPath $zip -CompressionLevel Optimal
Write-Host "Package: $package"
Write-Host "Archive: $zip"
