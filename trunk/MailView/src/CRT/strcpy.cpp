#include "strcpy.hpp"
#include <windows.h>

char *strcpy(char *dest,const char *src)
{
  return lstrcpy(dest,src);
}
