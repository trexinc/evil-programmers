#include "strlen.hpp"
#include <windows.h>

size_t strlen(const char *src)
{
  return lstrlen(src);
}
