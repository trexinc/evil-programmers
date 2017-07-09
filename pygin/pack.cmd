@echo off

setlocal

for /f %%i in ('powershell get-date -format "{yyyyMMdd}"') do set timestamp=%%i
for /f %%i in ('git rev-parse --short HEAD') do set hash=%%i

pushd artefacts\product
7z a -mx -r pygin_%timestamp%_%hash%.7z *.dll *.py *.md
popd

endlocal
