@echo off
call D:\dev\vc10\vcvarsall.bat x86_amd64
cl /EHsc /Zi /LD /I ..\common\unicode autoloader.cpp  user32.lib /link /def:autoloader.def /release