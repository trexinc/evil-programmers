@echo off
set PATH=D:\PrgTools\BC5\BIN;%PATH%
if .%1==. goto RELEASE

echo Compile _DEBUG
del Tmp\*.obj
make -D_DEBUG -fesc.mak>m.txt
goto end

:RELEASE
echo Compile RELEASE
del Tmp\*.obj
make -fesc.mak>m.txt

:end
type m.txt
