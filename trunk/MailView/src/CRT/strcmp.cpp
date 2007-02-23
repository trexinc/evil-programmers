#include "strcmp.hpp"
#include <windows.h>

int strcmp(const char *first, const char *last)
{
  return lstrcmp(first,last);
}
