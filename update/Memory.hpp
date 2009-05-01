#pragma once

#include "headers.hpp"

LPVOID WINAPIV malloc(size_t size);
LPVOID WINAPIV realloc(LPVOID block,size_t size);
VOID WINAPIV free(LPVOID block);

LPVOID WINAPIV operator new(size_t size);
LPVOID WINAPIV operator new[](size_t size);
VOID WINAPIV operator delete(LPVOID ptr);
VOID WINAPIV operator delete[](LPVOID ptr);

LPVOID WINAPIV memcpy(LPVOID dst,LPCVOID src,size_t count);
LPVOID WINAPIV memset(LPVOID dst,INT val,size_t count);
INT WINAPIV memcmp(LPCVOID src,LPCVOID dst,size_t count);
LPVOID WINAPIV memmove(LPVOID dst,LPCVOID src,size_t count);

INT WINAPIV _purecall();
