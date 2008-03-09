#ifndef __DM_MACRO_HPP__
#define __DM_MACRO_HPP__

#define DN_MACRO_DISABLE 0x3100
#define DN_MACRO_GETDIALOGINFO 0x3101

struct MacroDialogInfo
{
  long StructSize;
  char DialogId[128];
};

#endif
