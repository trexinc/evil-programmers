/*
    bcsvc.h
    Copyright (C) 2000-2009 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __BCSVC_H__
#define __BCSVC_H__

#define LISTEN_THREADS_COUNT 16

extern void ServiceStop(void);
extern void ServiceStart(BOOL aService);
extern BOOL ReportStatusToSCMgr(DWORD State,DWORD ExitCode,DWORD Wait);
extern DWORD WINAPI CopyThread(LPVOID lpvThreadParm);
extern DWORD WINAPI PrepareThread(LPVOID lpvThreadParm);
extern void LogEx(wchar_t *Func,wchar_t *parm1,wchar_t *parm2);
extern void LogSys(wchar_t *Func,wchar_t *parm1,wchar_t *parm2);

extern void InitInfo(void);
extern void FreeInfo(void);
extern void ResetStartEvent(void);
extern void WaitStartEvent(BOOL aService);
extern void AddInfo(DWORD type,const wchar_t *Src,const wchar_t *SrcDir,const wchar_t *DestDir,BOOL InfoEx,struct AdditionalData *add);
extern void UnwaitInfo(void);
extern void DelInfo(void);
extern DWORD GetAllInfo(struct SmallInfoRec **GetInfo);
extern void GetInfo(DWORD ThreadId,struct InfoRec *GetInfo);
extern BOOL UpdateInfo(const wchar_t *newSrc,const wchar_t *newDest);
extern void SetStopInfo(DWORD ThreadId);
extern void SetPauseInfo(DWORD ThreadId);
extern BOOL SetStartInfo(unsigned long long size,unsigned int type);
extern BOOL IdleInfo(void);
extern BOOL UpdatePosInfo(unsigned long long add,unsigned long long subadd);
extern DWORD CheckErrorInfo(void);
extern void SetAllStopInfo(void);
extern void WaitForInfo(void);
extern DWORD AskUserInfo(DWORD Type,DWORD Error);
extern void SetAskInfo(DWORD ThreadId,DWORD Flags,LUID AskID);

extern DWORD GetMaxError(void);
extern DWORD GetThreadCount(void);
extern BOOL GetAllowNetwork(void);
extern int GetWorkPriority(void);
extern int GetHearPriority(void);

extern void InitNotify(void);
extern void FreeNotify(void);
extern void ServiceNotify(unsigned long Code);
extern void JobNotify(unsigned long Code,const wchar_t *JobName);

extern void *DefaultTokenInformation(HANDLE token,TOKEN_INFORMATION_CLASS tic);

extern BOOL WipeFileW(wchar_t *filename);

#endif
