#ifndef __CMD_HPP__
#define __CMD_HPP__

extern INT myargc;
extern PTCHAR* myargv;
extern TCHAR myargp[];

BOOL IsParamPrefix(TCHAR c);
VOID ProcessCmdLine(VOID);
INT CheckParam(LPCTSTR check);
LPTSTR GetParam(LPTSTR lpCmdLine, INT index);
//LPTSTR SkipProgramName (LPTSTR lpCmdLine);

#endif /* __CMD_HPP__ */