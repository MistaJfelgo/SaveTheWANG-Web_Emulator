# SPDX-License-Identifier: GPL-2.0-or-later
param([ValidateRange(1,4)][int]$Jobs = 2)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$aliasCreated = $false
$mapping = (& subst | Where-Object { $_ -match '^V:' }) -join ''
if ($mapping) {
    if ($mapping -notlike ('*=> ' + $root)) { throw 'V: is mapped elsewhere. Free it or use the portable Bash helper.' }
} else {
    if (Test-Path V:\) { throw 'V: is already in use.' }
    & subst V: $root
    if ($LASTEXITCODE -ne 0) { throw 'Could not create the temporary V: alias.' }
    $aliasCreated = $true
}
try {
    $sdkTarget = Join-Path $root 'tools/emsdk'
    if (-not (Test-Path (Join-Path $sdkTarget '.emscripten'))) { throw 'Install and activate Emscripten 6.0.2 under tools/emsdk first; see BUILDING.md.' }
    $sdkLink = Join-Path $env:TEMP 'wang-public-sdk'
    if (Test-Path -LiteralPath $sdkLink) {
        $item = Get-Item -LiteralPath $sdkLink
        if ($item.LinkType -ne 'Junction' -or $item.Target -ne $sdkTarget) { throw 'The temporary SDK path belongs to another checkout.' }
    } else { New-Item -ItemType Junction -Path $sdkLink -Target $sdkTarget | Out-Null }
    $env:WANG_SDK_SHORT = $sdkLink.Replace('\','/')
    $env:WANG_WEB_CACHE = (Join-Path $env:TEMP 'wang-public-emcache').Replace('\','/')
    $env:WANG_WEB_JOBS = [string]$Jobs
    $env:MSYSTEM = 'UCRT64'
    $env:MSYS2_PATH_TYPE = 'inherit'
    $env:EMSDK_QUIET = '1'
    $priorErrorAction = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        & C:\msys64\usr\bin\bash.exe -lc '/v/scripts/build-web.sh' *> (Join-Path $root 'build-web.log')
        $buildExit = $LASTEXITCODE
    } finally { $ErrorActionPreference = $priorErrorAction }
    if ($buildExit -ne 0) { Get-Content (Join-Path $root 'build-web.log') -Tail 30; throw 'Build failed; see build-web.log.' }
} finally { if ($aliasCreated) { & subst V: /d } }
