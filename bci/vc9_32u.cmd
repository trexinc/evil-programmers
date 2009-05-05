cls
@set path=%path%;E:\private\far\ulink
call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
cl /c /W3 /nologo /Oi- /O1 /Os /J /GF /GR- /GS- /EHs-c- /Zp1 bci.cpp
ulink -q -e_wWinMain@16 -n -aa -Tpe -Gz -GM:.rdata=.text -Gh bci.obj -zkernel32;winmm;shell32;user32;gdi32 
