#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include "plugin.hpp"

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1, Y1, X2, Y2;
  unsigned int Flags;
  signed char Data;
};

void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber);
const char *GetMsg(int MsgId);

extern struct PluginStartupInfo Info;

#endif
