/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __PROGRESS_HPP__
#define __PROGRESS_HPP__

class Progress
{
  private:
    long Total;
    long TotalShow;
    long Cur;
    long CurShow;
    HANDLE hs;
    const char *mes[4];
    char head[100];
    char body[200];
    DWORD start;
    //handles
    HANDLE Event,Mutex,Thread;
    void ThreadProc(void);
  public:
    Progress(long total,const char *s,long totalshow=-1);
    ~Progress();
    int UseProgress(long cur,long curshow=-1);
  friend DWORD WINAPI ProgressThread(LPVOID lpvThreadParm);
};

#endif
