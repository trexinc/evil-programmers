@echo off
if %FARLANG%.==Russian. goto Rus
if %FARLANG%.==English. goto Eng

:Eng
echo Background Copy macro installer sets keyboard macro sequence for
echo calling FAR standard file functions from the plugin's dialog.
echo  F5 in the background copy dialog will invoke FAR copy dialog,
echo  F8 in the background delete dialog will invoke FAR delete dialog, etc.
echo.
echo (press Ctrl-C to cancel)
pause
goto continue
:Rus
echo ���⠫���� ����ᮢ Background Copy �����砥� ���ப������ ��� �맮��
echo �⠭������ 䠩����� �㭪権 FAR �� ������� �������, ���ਬ��:
echo  F5 � ������� 䮭����� ����஢���� �맮��� �⠭����� ������ ����஢���� FAR,
echo  F8 � ������� 䮭����� 㤠����� �맮��� �⠭����� ������ 㤠����� FAR, � �.�.
echo.
echo (������ Ctrl-C ��� ��室�)
pause
goto continue

:continue
set MacroTmp=MACRO$$$.REG
set BCopySettingsRoot=HKEY_CURRENT_USER\Software\Far
if not %FARUSER%.==. set BCopySettingsRoot=%BCopySettingsRoot%\Users\%FARUSER%
set BCopySettingsRoot=%BCopySettingsRoot%\Plugins\BCopy
echo REGEDIT4                                     >%MacroTmp%
echo [%BCopySettingsRoot%\Macros\Attributes]     >>%MacroTmp%
echo "CtrlA"="Esc CtrlA"                         >>%MacroTmp%
echo [%BCopySettingsRoot%\Macros\Copy]           >>%MacroTmp%
echo "F5"="Esc F5"                               >>%MacroTmp%
echo [%BCopySettingsRoot%\Macros\Delete]         >>%MacroTmp%
echo "F8"="Esc F8"                               >>%MacroTmp%
echo [%BCopySettingsRoot%\Macros\Move]           >>%MacroTmp%
echo "F6"="Esc F6"                               >>%MacroTmp%
echo [%BCopySettingsRoot%\Macros\Wipe]           >>%MacroTmp%
echo "AltDel"="Esc AltDel"                       >>%MacroTmp%

start /wait regedit.exe -s "%MacroTmp%"
del %MacroTmp% >nul
goto Cleanup

:Cleanup
set MacroTmp=
set BCopySettingsRoot=
