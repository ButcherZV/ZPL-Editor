# deploy.ps1 - copies ZPLEditor.exe + all required DLLs into build\deploy\
# Run from the project root: .\deploy.ps1

$buildDir  = "$PSScriptRoot\build"
$deployDir = "$buildDir\deploy"
$ucrt64    = "C:\msys64\ucrt64\bin"

if (Test-Path $deployDir) {
    Remove-Item "$deployDir\*" -Force
} else {
    New-Item -ItemType Directory -Path $deployDir | Out-Null
}

$exe = "$buildDir\ZPLEditor.exe"
if (-not (Test-Path $exe)) {
    Write-Error "ZPLEditor.exe not found in $buildDir - build the project first."
    exit 1
}
Copy-Item $exe $deployDir
Copy-Item "$buildDir\libzint.dll" $deployDir

$runtimeDlls = @(
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll"
)

$wxDlls = @(
    "wxbase32u_gcc_custom.dll",
    "wxmsw32u_core_gcc_custom.dll",
    "wxmsw32u_aui_gcc_custom.dll",
    "wxmsw32u_propgrid_gcc_custom.dll"
)

$imageDlls = @(
    "libpng16-16.dll",
    "libjpeg-8.dll",
    "libtiff-6.dll",
    "libwebp-7.dll",
    "libsharpyuv-0.dll",
    "libLerc.dll",
    "zlib1.dll",
    "libdeflate.dll",
    "liblzma-5.dll",
    "libzstd.dll",
    "libjbig-0.dll",
    "libpcre2-16-0.dll"
)

$allDlls = $runtimeDlls + $wxDlls + $imageDlls
$missing = @()

foreach ($dll in $allDlls) {
    $src = "$ucrt64\$dll"
    if (Test-Path $src) {
        Copy-Item $src $deployDir
    } else {
        $missing += $dll
    }
}

$copied = (Get-ChildItem $deployDir).Count
Write-Host ""
Write-Host "Deploy folder: $deployDir" -ForegroundColor Cyan
Write-Host "Files copied : $copied" -ForegroundColor Green

if ($missing.Count -gt 0) {
    Write-Host ""
    Write-Host "WARNING - these DLLs were not found in ${ucrt64}:" -ForegroundColor Yellow
    foreach ($m in $missing) { Write-Host "  $m" -ForegroundColor Yellow }
}
