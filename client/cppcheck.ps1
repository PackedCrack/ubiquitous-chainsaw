Write-Host ""
Write-Host "---------CPPCHECK - CLIENT----------"

If (-Not (Test-Path -Path "build-cppcheck")) {
    New-Item -ItemType Directory -Path "build-cppcheck"
}

$command = "cppcheck --cppcheck-build-dir=build-cppcheck --error-exitcode=1 --enable=all --suppressions-list=.suppress.cppcheck --inline-suppr --std=c++23 src/*.cpp src/*.hpp"
Invoke-Expression $command

$result = $LASTEXITCODE

If ($result -ne 0) {
    Write-Host "Cppcheck failed with exit code $result"
    Exit $result
}

Set-Location ..
Write-Host "-------CPPCHECK DONE----------"
Write-Host ""
