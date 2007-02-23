#ifndef ___SPLITPATH_HPP__
#define ___SPLITPATH_HPP__
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void _splitpath(register const char *path, char *drive, char *dir, char *fname, char *ext);
#ifdef __cplusplus
};
#endif

#endif
