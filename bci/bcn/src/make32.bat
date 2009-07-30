@echo off
set includ=/I"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include" /I"C:\Program Files\Microsoft Visual Studio 9.0\VC\include"

set LIB=C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib
cl /nologo /c /W3 /Gy /GF /Zp8 /J /GS- /Gr /GR- /EHs-c- /LD /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_CRT_NON_CONFORMING_SWPRINTFS /D_WIN32_WINNT=0x0502 bcn.cpp
link /nodefaultlib /nologo /dll /release /noentry /def:bcn.vc.def bcn.obj kernel32.lib
