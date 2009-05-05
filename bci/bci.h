/*
    bci.h
    Copyright (C) 2005 WhiteDragon
    Copyright (C) 2009 DrKnS

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

#define MAX_JOB 100
#define swb(a) a=a>>8|a<<8
#define WM_TASKICON (WM_USER+27)
#define wCls L"BCI"
#define wNm L"BCI"

#define SVC_NAME L"FARBCopy"
#define ACCESS_NAME L"FARBCopy.access"
#define PIPE_NAME L"\\\\.\\pipe\\FARBCopy"

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

#define OPERATION_COPY   1
#define OPERATION_INFO   2
#define OPERATION_PWD    3
#define OPERATION_ANSWER 4
#define OPERATION_ASCII  2037411683 //"copy"

// OPERATION_COPY

#define FILETYPE_FILE     1
#define FILETYPE_FOLDER   2
#define FILETYPE_JUNCTION 4
#define FILETYPE_DELETE   0x40000000

#define FILETYPE_UNKNOWN  0x80000000

#define COPYFLAG_ATTRMAIN           0x80000000
#define COPYFLAG_ATTR               0x40000000
#define COPYFLAG_WIPE               0x20000000
#define COPYFLAG_DELETE             0x10000000

#define COPYFLAG_OVERWRITE          0x08000000
#define COPYFLAG_APPEND             0x04000000
#define COPYFLAG_SKIP               0x02000000
#define COPYFLAG_REFRESH            0x01000000

#define COPYFLAG_MOVE               0x00800000
#define COPYFLAG_FAIL               0x00400000
#define COPYFLAG_ROSRC              0x00200000
#define COPYFLAG_RODST              0x00100000

#define COPYFLAG_STATISTIC          0x00080000
#define COPYFLAG_ACCESS             0x00040000

#define COPYFLAG_SET_READONLY       0x00020000
#define COPYFLAG_SET_HIDDEN         0x00010000
#define COPYFLAG_SET_SYSTEM         0x00008000
#define COPYFLAG_SET_ARCHIVE        0x00004000
#define COPYFLAG_SET_COMPRESSED     0x00002000

#define COPYFLAG_CLEAR_READONLY     0x00001000
#define COPYFLAG_CLEAR_HIDDEN       0x00000800
#define COPYFLAG_CLEAR_SYSTEM       0x00000400
#define COPYFLAG_CLEAR_ARCHIVE      0x00000200
#define COPYFLAG_CLEAR_COMPRESSED   0x00000100

#define COPYFLAG_SET_ENCRYPTED      0x00000080
#define COPYFLAG_CLEAR_ENCRYPTED    0x00000040

#define COPYFLAG_ADDITIONAL_DATA    0x00000020

#define COPYFLAG_LINK               0x00000018

#define COPYFLAG_DONTLOGERRORS      0x00000004
#define COPYFLAG_FAIL_ASK           0x00000002
#define COPYFLAG_ASK                0x00000001

#define LINK_TYPE_SKIP              0
#define LINK_TYPE_COPY              1
#define LINK_TYPE_LINK              2
#define LINK_TYPE_ASK               3

#define LINK_TYPE_MASK              3
#define LINK_TYPE_SHIFT             3

#define GET_LINK_TYPE(flags)        (((flags)>>LINK_TYPE_SHIFT)&LINK_TYPE_MASK)
#define SET_LINK_TYPE(flags,type)   (((type)&LINK_TYPE_MASK)<<LINK_TYPE_SHIFT)|((flags)&(~COPYFLAG_LINK))


#define ASKTYPE_CANCEL              0x00000000
#define ASKTYPE_OVERWRITE           0x00000001
#define ASKTYPE_APPEND              0x00000002
#define ASKTYPE_SKIP                0x00000003
#define ASKTYPE_SKIPALL             0x00000004
#define ASKTYPE_RETRY               0x00000005
#define ASKTYPE_COPY                0x00000006
#define ASKTYPE_LINK                0x00000007
#define ASKTYPE_MASK                0x00FFFFFF
#define ASKFLAG_ALL                 0x80000000

#define ASKGROUP_OVERWRITE          1
#define ASKGROUP_RETRY              2
#define ASKGROUP_RETRYONLY          3
#define ASKGROUP_LINK               4

#define STRFLAG_OVERWRITE L'o'
#define STRFLAG_APPEND    L'a'
#define STRFLAG_REFRESH   L'r'

#define STRFLAG_MOVE      L'm'
#define STRFLAG_FAIL      L'f'
#define STRFLAG_ROSRC     L'1'
#define STRFLAG_RODST     L'2'

#define STRFLAG_ACCESS    L'u'

#define STRFLAG_STATISTIC L'0'


struct FileRec
{
  DWORD type;
  wchar_t name[MAX_PATH];
};

struct FileRecs
{
  HANDLE event;
  DWORD flags;
  DWORD count;
  struct FileRec *files;
  void *add;
};

struct StrRec
{
  DWORD flags;
  wchar_t from[MAX_PATH];
  wchar_t to[MAX_PATH];
};

// OPERATION_INFO

#define INFOTYPE_INVALID  0
#define INFOTYPE_COPY     1
#define INFOTYPE_MOVE     2
#define INFOTYPE_WIPE     3
#define INFOTYPE_DELETE   4
#define INFOTYPE_ATTR     5

#define INFOFLAG_ALL      0x00000001
#define INFOFLAG_BYHANDLE 0x00000002
#define INFOFLAG_STOP     0x00000004
#define INFOFLAG_PAUSE    0x00000008

struct SmallInfoRec
{
  DWORD ThreadId;
  DWORD type;
  wchar_t Src[2*MAX_PATH];
  wchar_t SrcDir[MAX_PATH];
  wchar_t DestDir[MAX_PATH];
  DWORD percent;
  BOOL pause;
  BOOL InfoEx;
  DWORD Ask; //Ask
  BOOL wait;
  DWORD Reserved;
};

struct InfoRec
{
  struct SmallInfoRec info;
  wchar_t Src[2*MAX_PATH];
  wchar_t Dest[2*MAX_PATH];
  BOOL stop;
  //Ask
  DWORD Flags;
  LUID AskID;
  DWORD Error;
  DWORD Reserved;
  // InfoEx
  SYSTEMTIME StartTime;
  unsigned long long TotalSize;
  unsigned long long CurrentSize;
  unsigned long long CurrentSizeAdd;
  unsigned long long Errors;
  unsigned long long PauseTime;
  unsigned int SizeType;
  unsigned int Reserved2;
  // Internal data
  unsigned long long CPS;
};

struct AdditionalData
{
  unsigned long long CPS; //in bytes per second
};

// OPERATION_PWD

#define PWDFLAG_CLEAR    0x00000001
#define PWDFLAG_SET      0x00000002

WORD a[16];//={0,0,0,0,0xffff,0,0,0,0,0,0xffff,0,0,0,0,0xffff};
WORD x[16];//={0,0,0,0,0,     0,0,0,0,0,0,     0,0,0,0,0};


BYTE n[11][5]=
{
  {6,9,9,9,6},
  {2,6,2,2,7},
  {6,9,2,4,15},
  {6,9,2,9,6},
  {3,5,9,15,1},
  {15,8,14,1,14},
  {7,8,14,9,6},
  {15,1,2,4,8},
  {6,9,6,9,6},
  {6,9,7,1,14},
  {25,18,4,9,19}
};

BYTE t[5][4]=
{
  {0x30,0x40,0x40,0x30},
  {0x44,0x7c,0x54,0x44},
  {0x44,0x54,0x54,0x28},
  {0x70,0x48,0x48,0x70},
  {0x30,0x48,0x78,0x48}
};

wchar_t *O[5]=
{
  L"Копирование",
  L"Перенос",
  L"Стирание",
  L"Удаление",
  L"Установка атрибутов"
};

struct JobItem
{
  UINT id;
  BYTE pr;
}
lst[MAX_JOB];
