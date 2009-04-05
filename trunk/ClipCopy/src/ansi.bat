call vcvars32.bat
@del ClipCopy.dll
cl /O1i /GF /Gr /GR- /GX- /LD ClipCopy.cpp /link /opt:nowin98 /noentry /nodefaultlib /def:ClipCopy_ansi.def kernel32.lib user32.lib shell32.lib ole32.lib uuid.lib libc.lib
rem /merge:.rdata=.text
rem /FAsc /Fa ClipCopy.asm /W4 /WX
rem @copy ClipCopy.dll D:\TOOLS\FAR\Plugins\ClipCopy\ClipCopy.dll /Y
@if exist ClipCopy.exp del ClipCopy.exp>nul
@if exist ClipCopy.obj del ClipCopy.obj>nul
@if exist ClipCopy.lib del ClipCopy.lib>nul
