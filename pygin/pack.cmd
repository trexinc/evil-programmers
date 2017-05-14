@echo off

for /f "tokens=1-4 delims=. " %%a in ('date /t') do (set timestamp=%%c%%b%%a)
for /f %%i in ('git rev-parse --short HEAD') do set hash=%%i

pushd artefacts\product
7z a -mx -r pygin_%timestamp%_%hash%.7z *.dll *.py *.md
popd
