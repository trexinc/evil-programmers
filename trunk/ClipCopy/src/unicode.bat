call vcvars32.bat
@del ClipCopyW.dll
cl /O1i /GF /Gr /GR- /GX- /LD ClipCopy.cpp /D "UNICODE" /link /opt:nowin98 /out:ClipCopyW.dll /noentry /nodefaultlib /def:ClipCopy.def kernel32.lib user32.lib shell32.lib ole32.lib uuid.lib libc.lib
rem /merge:.rdata=.text
rem /FAsc /Fa ClipCopy.asm /W4 /WX
rem @copy ClipCopy.dll D:\TOOLS\FAR\Unicode\Plugins\ClipCopy\ClipCopyW.dll /Y
@if exist ClipCopy.exp del ClipCopy.exp>nul
@if exist ClipCopy.obj del ClipCopy.obj>nul
@if exist ClipCopy.lib del ClipCopy.lib>nul
