#include "wcslen.hpp"

size_t wcslen (const wchar_t *wcs)
{
  const wchar_t *eos = wcs;

  while (*eos++)
    ;

  return ((size_t)(eos - wcs - 1));
}
