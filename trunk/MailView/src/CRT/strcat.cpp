#include "strcat.hpp"
#include <windows.h>

char *strcat(char *dest,const char *src)
{
  return lstrcat(dest,src);
}
