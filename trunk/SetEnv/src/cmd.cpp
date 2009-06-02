#include "stdafx.h"
#include "cmd.hpp"

INT  myargc = 0;
PTCHAR* myargv = NULL;
TCHAR myargp[1024];

#ifndef UNICODE
static INT IsSpace(INT c);
static VOID parse_cmdline(PTCHAR cmdstart, PTCHAR argv[], PTCHAR args, PUINT numargs, PUINT numchars);

static INT IsSpace(INT c)
{
  return (c == 0x20 || (c >= 0x09 && c <= 0x0D));
}

static VOID parse_cmdline(PTCHAR cmdstart, PTCHAR* argv, PTCHAR args, PUINT numargs, PUINT numchars)
{
  PTCHAR p;
  INT inquote;                    /* 1 = inside quotes */
  INT copychar;                   /* 1 = copy char to *args */
  unsigned numslash;              /* num of backslashes seen */

  *numchars = 0;
  *numargs = 0;

  p = cmdstart;

  inquote = 0;

  /* loop on each argument */
  for(;;)
  {

    while (IsSpace((INT)*p)) ++p;

    if (*p == TEXT('\0'))break;   /* end of args */

    /* scan an argument */
    if (argv) *argv++ = args;     /* store ptr to arg */
    ++*numargs;

    /* loop through scanning one argument */
    for (;;) {
      copychar = 1;
      /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
         2N+1 backslashes + " ==> N backslashes + literal "
         N backslashes ==> N backslashes */
      numslash = 0;
      while (*p == TEXT('\\')) { /* count number of backslashes for use below */
        ++p;
        ++numslash;
      }
      if (*p == TEXT('\"')) {
      /* if 2N backslashes before, start/end quote, otherwise
        copy literally */
        if ((numslash & 1) == 0) {
          if (inquote) {
            if (p[1] == TEXT('\"'))
              p++;    /* Double quote inside quoted string */
            else        /* skip first quote char and copy second */
              copychar = 0;
          } else copychar = 0;       /* don't copy quote */
          inquote = !inquote;
        }
        numslash >>= 1;             /* divide numslash by two */
      }

      /* copy slashes */
      while (numslash--) {
        if (args) *args++ = TEXT('\\');
        ++*numchars;
      }

      /* if at end of arg, break loop */
      if (*p == TEXT('\0') || (!inquote && IsSpace((INT)*p))) break;

      /* copy character into argument */
      if (copychar) {
        if (args) *args++ = *p;
        ++*numchars;
      }
      ++p;
    }

    /* null-terminate the argument */

    if (args) *args++ = TEXT('\0');          /* terminate string */
    ++*numchars;
  }
}
#endif //UNICODE

BOOL IsParamPrefix(TCHAR c)
{
  return c == TEXT('/');// || c == TEXT('-');
}

VOID ProcessCmdLine(VOID)
{
#ifdef UNICODE
  myargv = CommandLineToArgvW(GetCommandLine(), &myargc);
#else
  UINT numchars;
  parse_cmdline(GetCommandLine(), NULL, NULL, (UINT*)&myargc, &numchars);
  myargv = (PTCHAR*)malloc(myargc * sizeof(PTCHAR) + numchars * sizeof(TCHAR) + 1);
  parse_cmdline(GetCommandLine(), myargv, ((PTCHAR)myargv) + sizeof(PTCHAR) * myargc, (UINT*)&myargc, &numchars);
#endif
}

INT CheckParam(LPCTSTR check)
{
  int i = myargc;
  while (--i > 0)
  {
    if (lstrlen(myargv[i]) > 1)
    {
      if (IsParamPrefix(myargv[i][0]) && lstrcmpi(check, myargv[i] + 1) == 0)
        return i;
    }
  }
  return 0;
}

LPTSTR GetParam(LPTSTR lpCmdLine, INT index)
{
  LPTSTR  p = lpCmdLine;
  BOOL    bInQuotes = FALSE;

  for (INT i = 0; i < index; i++)
  {
    for (p; *p; p = CharNext(p))
    {
      if ((*p == TEXT(' ') || *p == TEXT('\t')) && !bInQuotes)
        break;

      if (*p == TEXT('\"'))
        bInQuotes = !bInQuotes;
    }

    while (*p == TEXT(' ') || *p == TEXT('\t'))
      p++;
  }

  return (p);
}
