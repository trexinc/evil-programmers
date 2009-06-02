#ifndef __SETENV_HPP__
#define __SETENV_HPP__

typedef enum tagSoughtProcessType
{
  SEARCH_BY_IMAGENAME = 0,
  SEARCH_BY_PROCESSID = 1
} SoughtProcessType;

typedef struct tagSoughtProcessItem
{
  BOOL UseMask;
  SoughtProcessType Type;
  TCHAR Name[MAX_PATH];
} SoughtProcessItem, *PSoughtProcessItem;

#endif /* __SETENV_HPP__ */
