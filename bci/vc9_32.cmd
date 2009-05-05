cls
call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
cl /c /W3 /nologo /Oi- /O1 /Os /J /GF /GR- /GS- /EHs-c- /Zp1 bci.cpp
link /NOLOGO /entry:wWinMain /nodefaultlib /release /merge:.rdata=.text bci.obj kernel32.lib winmm.lib shell32.lib user32.lib gdi32.lib 
