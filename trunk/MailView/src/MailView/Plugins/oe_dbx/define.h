/***************************************************************************
                          define.h  -  Contains my debug macros
                             -------------------
    begin                : January 2001
    copyright            : (C) 2001 by David Smith
    email                : Dave.S@Earthcorp.com
    win32.vc6 adapt      : Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#define DEBUG_ALL

#ifndef DEFINEH_H
#define DEFINEH_H

//variable number of arguments to this macro. will expand them into 
// ## args, then exit with status of 1
#include <stdio.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <unistd.h>


#ifdef DEBUSAVE
#define MESSAGEINIT(filename) messagefp=fopen(filename, "w");
#define MESSAGESAVE(format, args...) messagefp!=NULL?fprintf(messagefp, format, ##args):0;
#define MESSAGESTOP() fclose(messagefp);
#define DEBUGINIT(filename) debugfp=fopen(filename, "w"); //open with truncate
#define DEBUGWRITE(format,args...) debugfp!=NULL?fprintf(debugfp, format, ##args):0;
#define DEBUGSTOP() fclose(debugfp);
#else
#define MESSAGEINIT(filename)
#define MESSAGESAVE(format, args...)
#define MESSAGESTOP()
#define DEBUGINIT(a...)
#define DEBUGWRITE(format,args...)
#define DEBUGSTOP()
#endif


#ifdef DEBUG_ALL
#define MESSAGEPRINT(format, args...) fprintf(stderr, format, ## args);
#else
#define MESSAGEPRINT(format,args...)
#endif

#define LOGSTOP() {MESSAGESTOP();DEBUGSTOP();}

#define DIE(format,args...) {\
 MESSAGEPRINT(format, ##args);\
 MESSAGESAVE(format, ##args);\
 LOGSTOP();\
 exit(1);\
}
#define WARN(format,args...) {MESSAGEPRINT(format, ##args); MESSAGESAVE(format, ##args);}

#ifdef DEBUG_ALL
#define DEBUG_MODE_GEN
//#define DEBUG_MODE_INDEX
//#define DEBUG_MODE_EMAIL
#define DEBUGPRINT
#define DEBUG_MODE_WARN
//#define DEBUGSAVE
//extern FILE *debugfp;
#endif

//extern FILE *messagefp;

#ifdef DEBUGPRINT
#define DEBUG_PRINT(format,args...) fprintf(stderr, format, ## args);
#else
#define DEBUG_PRINT(a...)
#endif

#ifdef DEBUG_MODE_GEN
#define DEBUG(format,args...) {DEBUG_PRINT(format, ##args);DEBUGWRITE(format, ##args)}
#else
#define DEBUG(format,args...) {}
#endif

#ifdef DEBUG_MODE_INDEX
#define DEBUG_INDEX(format,args...) DEBUG(format, ##args);
#else
#define DEBUG_INDEX(format,args...) {}
#endif

#ifdef DEBUG_MODE_EMAIL
#define DEBUG_EMAIL(format,args...) DEBUG(format, ##args);
#else
#define DEBUG_EMAIL(format,args...) {}
#endif

#ifdef DEBUG_MODE_WARN
#define DEBUG_WARN(format,args...) DEBUG(format, ##args);
#else
#define DEBUG_WARN(format,args...) {}
#endif

#define RET_DERROR(res, ret_val, debug_msg, args...)\
	if (res) { DEBUG_WARN(debug_msg, ## args); dbx_errno = ret_val; return -1; }
#define RET_ERROR(res, ret_val)\
	if (res) {dbx_errno = ret_val; return -1;}	

#else
#define MESSAGEINIT(filename)        ((void)0)
#define MESSAGESAVE()                ((void)0)
#define MESSAGESTOP()                ((void)0)
#define DEBUGINIT(a)                 ((void)0)
#define DEBUGWRITE()                 ((void)0)
#define DEBUGSTOP()                  ((void)0)
#define DEBUG_PRINT(a)               ((void)0)

#define DEBUG3(a,b,c)                ((void)0)
#define DEBUG4(a,b,c,d)              ((void)0)
#define DEBUG_INDEX2(a,b)            ((void)0)
#define DEBUG_INDEX3(a,b,c)          ((void)0)
#define DEBUG_INDEX4(a,b,c,d)        ((void)0)
#define DEBUG_EMAIL3(a,b,c)          ((void)0)
#define DEBUG_EMAIL4(a,b,c,d)        ((void)0)
#define DEBUG_WARN2(a,b)             ((void)0)
#define DEBUG_WARN3(a,b,c)           ((void)0)
#define DEBUG_WARN4(a,b,c,d)         ((void)0)

#define RET_ERROR(res,ret_val)\
	if (res) {dbx_errno = ret_val; return -1;}

#define RET_DERROR4(res,ret_val,_3,_4) RET_ERROR(res,ret_val)
#define RET_DERROR5(res,ret_val,_3,_4,_5) RET_ERROR(res,ret_val)
#define RET_DERROR6(res,ret_val,_3,_4,_5,_6) RET_ERROR(res,ret_val)

#define WARN(a,b)                    ((void)0)

#define DEBUG         DEBUG3
#define DEBUG_WARN    DEBUG_WARN2
#define DEBUG_INDEX   DEBUG_INDEX3
#define DEBUG_EMAIL   DEBUG_EMAIL3
#define RET_DERROR    RET_DERROR5

#endif // !defined(_WIN32)

#endif //DEFINEH_H

