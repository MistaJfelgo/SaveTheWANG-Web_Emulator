param([ValidateRange(1,4)][int]$Jobs = 2)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$createdAlias = $false
$mapping = (& subst | Where-Object { $_ -match '^V:' }) -join ''
if ($mapping) {
    if ($mapping -notlike ('*=> ' + $root)) { throw 'V: is already mapped elsewhere; release it before building.' }
} else {
    if (Test-Path V:\) { throw 'V: is already in use.' }
    & subst V: $root
    if ($LASTEXITCODE -ne 0) { throw 'Could not map the short build path.' }
    $createdAlias = $true
}
try {
    $sdk = Join-Path $env:TEMP 'wang-sdk'
    $target = Join-Path $root 'tools/emsdk'
    if (Test-Path -LiteralPath $sdk) {
        $item = Get-Item -LiteralPath $sdk
        if ($item.LinkType -ne 'Junction' -or $item.Target -ne $target) { throw 'The temporary wang-sdk path is already used by another installation.' }
    } else { New-Item -ItemType Junction -Path $sdk -Target $target | Out-Null }
    $env:WANG_SDK_SHORT = $sdk.Replace('\','/')
    $env:WANG_WEB_CACHE = (Join-Path $env:TEMP 'wang-emcache').Replace('\','/')
    $env:WANG_WEB_JOBS = [string]$Jobs
    $env:MSYSTEM = 'UCRT64'
    $env:MSYS2_PATH_TYPE = 'inherit'
    $env:EMSDK_QUIET = '1'
    Push-Location $root
    try {
        # Windows PowerShell wraps native stderr warnings as ErrorRecords.
        # Log those warnings and decide success from the compiler's exit code.
        $priorErrorAction = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            & C:\msys64\usr\bin\bash.exe -lc '/v/tools/build-wang-web.sh' *> tools/wang-web-build.log
            $buildExitCode = $LASTEXITCODE
        } finally { $ErrorActionPreference = $priorErrorAction }
        if ($buildExitCode -ne 0) {
            Get-Content tools/wang-web-build.log -Tail 30
            throw 'Browser build failed; see tools/wang-web-build.log.'
        }
        & python.exe tools/stage-wang-web.py
        if ($LASTEXITCODE -ne 0) { throw 'Browser artifact staging failed.' }
        & python.exe tools/record-wang-web-build.py
        if ($LASTEXITCODE -ne 0) { throw 'Build metadata recording failed.' }
    } finally { Pop-Location }
} finally { if ($createdAlias) { & subst V: /d } }
