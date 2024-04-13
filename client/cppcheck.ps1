Write-Host ""
Write-Host "---------CPPCHECK - CLIENT----------"

If (-Not (Test-Path -Path "build-cppcheck")) {
    New-Item -ItemType Directory -Path "build-cppcheck"
}

$files = Get-ChildItem -Path src, ../shared -Include *.cpp, *.hpp -Recurse | ForEach-Object { $_.FullName }
cppcheck --cppcheck-build-dir=build-cppcheck --error-exitcode=1 --enable=all --suppressions-list=.suppress.cppcheck --inline-suppr --std=c++23 $files


$result = $LASTEXITCODE

If ($result -ne 0) {
    Write-Host "Cppcheck failed with exit code $result"
    Exit $result
}

Set-Location ..
Write-Host "-------CPPCHECK DONE----------"
Write-Host ""
