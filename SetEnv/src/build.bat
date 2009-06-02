@call vcvars32.bat >nul
@set projname=setenv

@if exist %projname%.exe del %projname%.exe
@if exist %projname%.res del %projname%.res

@set cppfiles=SetEnv.cpp cmd.cpp common.cpp env.cpp find.cpp match.cpp memory.cpp StdAfx.cpp

rc %projname%.rc
cmd /c cl /W4 /O1i /Gr /GF %cppfiles% %projname%.res /link /subsystem:console /out:SETENV.EXE /opt:nowin98 /stub:minstub.exe kernel32.lib user32.lib Shell32.Lib libc.lib

@if exist %projname%.res del %projname%.res
@if exist *.exp del *.exp>nul
@if exist *.obj del *.obj>nul
@if exist *.lib del *.lib>nul
@if exist *.cod del *.cod>nul
