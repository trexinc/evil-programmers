@echo off
cls
setlocal

echo Choose operation:
echo 1 - Install macros
echo 2 - Remove macros

:id
set ID=1
set /p ID=Enter id (1/2, default - 1): 
if %ID% neq 1 (if %ID% neq 2 (echo "%ID%" is bad answer, try again) && goto id)

set USER=
set /p USER=username (default - none): 

set name=Yet Another Completion

set callplg=F11 $If(Menu.Select(\"%name%\")) Enter $If(Menu)
set cmdend=$Exit $End $End $Else Esc $End $End $AKey
set panels=Shell^|^|Info^|^|QView^|^|Tree

echo preparing...
if defined USER (set userpath=\Users\%USER%) else set userpath=
set regpath=HKCU\Software\Far2%userpath%\KeyMacros\Common

set keyname=CtrlSpace
set Sequence="$If(%panels%||Dialog||Editor) %callplg% $If(CheckHotKey(\"0\")) 0 %cmdend%"
set Description="Completion (next)"
call :job

set keyname=Tab
set Sequence="$If((%panels%)&&!PPanel.Visible) %callplg% $If(CheckHotKey(\"0\")) 0 %cmdend%"
call :job

set keyname=CtrlShiftSpace
set Sequence="$If(%panels%||Dialog||Editor) %callplg% $If(CheckHotKey(\"1\")) 1 %cmdend%"
set Description="Completion (previous)"
call :job

set keyname=ShiftTab
set Sequence="$If(%panels%) %callplg% $If(CheckHotKey(\"1\")) 1 %cmdend%"
call :job

echo.
echo done.
pause

goto :eof
:job
set op=removing
if %ID%==1 (set op=adding)
echo %op% %keyname%
set key="%regpath%\%keyname%"
reg delete %key% /f > nul 2>&1
if %ID%==1 call :setmacro

goto :eof

:setmacro
reg delete %key% /f > nul 2>&1
reg add %key% /v Sequence /d %sequence% /f > nul
reg add %key% /v DisableOutput /t REG_DWORD /d 1 /f > nul
reg add %key% /v Description /d %description% /f > nul
reg add %key% /ve /d %description% /f > nul

goto :eof

endlocal
