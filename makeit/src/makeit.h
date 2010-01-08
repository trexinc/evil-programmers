/*
  Copyright (C) 2000 Konstantin Stupnik

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

#define APPNAME "MakeIt"
#define MAX_ITEMS 150
#define MAX_MSG 10
#define MAX_ORD 4
#define TEXTSIZE 100000
#define TEXTWIDTH 69

#include "XTools.hpp"

using namespace XClasses;

#define FARAPI(type) extern "C" type __declspec(dllexport) WINAPI

int szcmp(const void*,const void*);

struct SCommand{
  String name,parser,command;
  SCommand(){}
  SCommand(const SCommand& src)
  {
    name=src.name;
    parser=src.parser;
    command=src.command;
  }
};

typedef List<SCommand*> CCmdList;

struct SLineInfo{
  int error;
  String file;
  int line,col;
};

struct SColors{
  int bg;
  int text;
  int error;
  int warning;
  int number;
};

struct SOptions{
  int shownum;
  int autodelete;
  int beep;
  int beeptime;
  int autosave;
  String wave;
  SOptions()
  {
    shownum=0;
    autodelete=0;
    beep=0;
    beeptime=0;
    autosave=0;
  }
};


typedef List<SLineInfo*> CLineList;

int parse(char* s,char* fname,int* line,int *col,char*msg);
int pipesrv(const char* command);

int read_config(const char* filename);
int init_config(SColors *clr,SOptions* opt);
void free_config();
int get_commands(const String&,CCmdList&);
int match_line(const char *parser,const String& line,SLineInfo *li);
void get_parsers(StrList& lst);
String get_prefixes();
int set_prefix(const char *pfx,const char *cmd,SCommand& scmd);

int Msg(const char* err);
int Msg2(char* err,char* err2);
const char* GetMsg(int MsgId);


enum
{
 MPlugin,
 MPipeError,
 MProcessError,
 MConfigError,
 MNoFileType,
 MCommands,
 MShowErrors,
 MMakeCurrent,
 MMakeLast,
 MMakeActions,
 MOpenLog,
 MSaveLog,
 MMakeAtCur,
 MSelectParser,
 MLogFailed,
 MInvalidRegexp,
 MIncompleteCommand,
 MInvalidParser,
 MNoTypes,
 MFilename,
 MSaveFailed,
 MReloadCfg,
 MDone,
 MCompleted,
 MNoParsersFound,
};
