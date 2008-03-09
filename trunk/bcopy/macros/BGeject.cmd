@echo off
set macrokey=CtrlShiftE
set macrodesceng=BCopy: Eject removable media
set macrodescrus=BCopy: �����祭�� ᬥ����� ������⥫�

if %1.==/?. goto Usage
if %1.==. goto Usage
set macrokey=%1

:Usage
if %FARLANG%.==Russian. goto Rus
if %FARLANG%.==English. goto Eng
:Eng
echo The FAR manager macro installer binds keyboard sequence to the specified key.
echo %~nx0 [MacroActivationKey]
echo MacroActivationKey overrides the default key (if specified).
echo.
echo Sequence description: %macrodesceng%
echo Are you sure to assign the keyboard sequence to %macrokey%?
echo (press Ctrl-C to cancel)
pause
set macrodesc=%macrodesceng%
goto continue
:Rus
echo ���⠫���� ����ᮢ FAR manager �����砥� ���ப������ �� 㪠������ �������.
echo %~nx0 [�����蠂맮�������]
echo �����蠂맮�������, �᫨ 㪠����, ����頥� ������� �맮�� �� 㬮�砭��.
echo.
echo ���ᠭ�� ���ப������: %macrodescrus%
echo �� ����⢨⥫쭮 ��� �������� ��� ������� �� %macrokey%?
echo (������ Ctrl-C ��� ��室�)
pause
set macrodesc=%macrodescrus%
goto continue

:continue
set MacroTmp=MACRO$$$.REG
set FarSettingsRoot=HKEY_CURRENT_USER\Software\Far
if not %FARUSER%.==. set FarSettingsRoot=%FarSettingsRoot%\Users\%FARUSER%
echo REGEDIT4                                          >%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\Shell\%macrokey%]   >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\Viewer\%macrokey%]  >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\Editor\%macrokey%]  >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\Search\%macrokey%]  >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\QView\%macrokey%]   >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\Info\%macrokey%]    >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%
echo [%FarSettingsRoot%\KeyMacros\Tree\%macrokey%]    >>%MacroTmp%
echo "Sequence"="F11 b a"                             >>%MacroTmp%
echo "Description"="%macrodesc%"                      >>%MacroTmp%
echo "DisableOutput"=dword:00000001                   >>%MacroTmp%

start /wait regedit.exe -s "%MacroTmp%"
del %MacroTmp% >nul
goto Cleanup

:Cleanup
set MacroTmp=
set FarSettingsRoot=
set macrokey=
set macrodesc=
set macrodesceng=
set macrodescrus=
