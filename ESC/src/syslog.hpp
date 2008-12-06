#ifndef __syslog_hpp
#define __syslog_hpp

#ifdef _DEBUG
 #ifdef __cplusplus
  extern "C"
  {
  #endif
    void SysLog(char *fmt, ...);
  #ifdef __cplusplus
  }
  #endif
 #define _D(x) x
#else
 #define _D(x)
#endif

#endif // __syslog_hpp
