#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include "plugin.hpp"

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const char *Data;
};

void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber);
const char *GetMsg(int MsgId);

#endif
