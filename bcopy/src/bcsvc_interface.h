#ifndef __BCSVC_INTERFACE_H__
#define __BCSVC_INTERFACE_H__

#define BCSVC_START      0UL
#define BCSVC_STOP       1UL
#define BCSVC_START_JOB  2UL
#define BCSVC_STOP_JOB   3UL

struct BCSvcJob
{
  unsigned long StructSize;
  wchar_t JobName[2*MAX_PATH];
};

#ifdef __cplusplus
extern "C" {
#endif
  int WINAPI _export Notify(unsigned long Msg,void *Data);
#ifdef __cplusplus
}
#endif

#endif
