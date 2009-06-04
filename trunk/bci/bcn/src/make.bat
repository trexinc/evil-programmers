@echo off
set ooo=32
set includ=/I"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include" /I"C:\Program Files\Microsoft Visual Studio 9.0\VC\include"

if "%ooo%" == "32" (
	set LIB=C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib
	cl /nologo /c /W3 /Gy /GF /Zp8 /J /GS- /Gr /GR- /EHs-c- /LD /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_CRT_NON_CONFORMING_SWPRINTFS /D_WIN32_WINNT=0x0502 bcn.cpp
	link /nodefaultlib /nologo /dll /release /noentry /def:bcn.vc.def bcn.obj kernel32.lib
) ELSE (
	set LIB=C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib\x64
	"C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\x86_amd64\cl.exe" /nologo /c /W3 /Gy /GF /Zp8 /J /GS- /Gr /GR- /EHs-c- /LD /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_CRT_NON_CONFORMING_SWPRINTFS /D_WIN32_WINNT=0x0502 bcn.cpp
	link /nodefaultlib /nologo /dll /release /noentry /def:bcn.vc.def bcn.obj kernel32.lib /MACHINE:X64
)

rem user32.lib gdi32.lib advapi32.lib shell32.lib