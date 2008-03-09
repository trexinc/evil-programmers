@echo off
if %FARLANG%.==Russian. goto Rus
if %FARLANG%.==English. goto Eng

:Eng
echo Background Copy macro installer assigns keyboard macro sequence for
echo "Overwrite All" and "Skip All" actions in the overwrite confirmation dialog
echo to the "A" and "K" keys respectively (in the manner of FAR v1.65).
echo.
echo (press Ctrl-C to cancel)
pause
set OverwriteAll="a"="a Add o"
set      SkipAll="k"="a Add s"
goto continue
:Rus
echo Инсталлятор макросов Background Copy назначает макрокоманды для вызова действий
echo "Переписать все" и "Пропустить все" в диалоге подтверждения перезаписи на
echo клавиши "в" и "р" соответственно (по образцу FAR v1.65).
echo.
echo (нажмите Ctrl-C для выхода)
pause
set OverwriteAll="в"="в Add м"
set      SkipAll="р"="в Add п"
goto continue

:continue
set MacroTmp=MACRO$$$.REG
set BCopySettingsRoot=HKEY_CURRENT_USER\Software\Far
if not %FARUSER%.==. set BCopySettingsRoot=%BCopySettingsRoot%\Users\%FARUSER%
set BCopySettingsRoot=%BCopySettingsRoot%\Plugins\BCopy
echo REGEDIT4                                     >%MacroTmp%
echo [%BCopySettingsRoot%\Macros\Overwrite]      >>%MacroTmp%
echo %OverwriteAll%                              >>%MacroTmp%
echo %SkipAll%                                   >>%MacroTmp%

start /wait regedit.exe -s "%MacroTmp%"
del %MacroTmp% >nul
goto Cleanup

:Cleanup
set MacroTmp=
set BCopySettingsRoot=
set OverwriteAll=
set SkipAll=
