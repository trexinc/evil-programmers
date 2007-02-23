//
//  Copyright (c) Cail Lomecb (Igor Ruskih) 1999-2001 <ruiv@uic.nnov.ru>
//  You can use, modify, distribute this code or any other part
//  of this program in sources or in binaries only according
//  to License (see /doc/license.txt for more information).
//

/*
    Position Saver plugin for FAR Manager
    Copyright (C) 2002-2005 Alex Yaroslavsky

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
#ifndef _CSGML_
#define _CSGML_

#include "PositionSaver.hpp"

typedef class CSgmlExt *PSgmlExt;
typedef char  *SParams[2];

#define MAXPARAMS 0x20
#define MAXTAG 0x10
#define SP 1

enum ElType
{
  EPLAINEL, ESINGLEEL, EBLOCKEDEL, EBASEEL
};

class CSgmlExt
{
  //for derived classes
  virtual PSgmlExt createnew(ElType type, PSgmlExt parent, PSgmlExt after);
  virtual bool init();

  void destroylevel();
  void insert(PSgmlExt El);
  //bool setcontent(const char *src,int sz);
  void substquote(SParams par, char *sstr, char c);

  //char *content;
  //int contentsz;
  char name[MAXTAG];

  PSgmlExt eparent;
  PSgmlExt enext;
  PSgmlExt eprev;
  PSgmlExt echild;
  int chnum;
  ElType type;

  SParams params[MAXPARAMS];
  int parnum;

  //editing
  bool isloop(PSgmlExt El, PSgmlExt Parent);

public:
  CSgmlExt();
  ~CSgmlExt();
  bool parse(const char *src,int sz);

  virtual PSgmlExt parent();
  virtual PSgmlExt next();
  virtual PSgmlExt prev();
  virtual PSgmlExt child();

  ElType gettype();
  char *getname();
  //char *getcontent();
  //int getcontentsize();

  char *GetParam(int no);
  char *GetChrParam(const char *par);
  bool GetIntParam(const char *par, int *result);
  #ifdef ALLOW_VIEWER_API
  bool GetInt64Param(const char *par, __int64 *result);
  #endif
  //bool GetFltParam(const char *par, double *result);

  PSgmlExt search(const char *TagName);
  PSgmlExt enumchilds(int no);
  // in full hierarchy
  virtual PSgmlExt fprev();
  virtual PSgmlExt fnext();
  virtual PSgmlExt ffirst();
  virtual PSgmlExt flast();

  //editing
  void setname(char *newname);

  // parameters add and change
  void Delete(void);
  PSgmlExt addnewnext(char *src, int sz);
  /*PSgmlExt addnewchild(char *src, int sz);*/
  bool addparam(char *name, char *val);
  bool addparam(char *name, int val);
  #ifdef ALLOW_VIEWER_API
  bool addparam(char *name, __int64 val);
  #endif
  //bool addparam(char *name, double val);
  //bool changecontent(char *data, int len);
  bool delparam(char *name);

  bool move(PSgmlExt parent, PSgmlExt after);
  //PSgmlExt copytree(PSgmlExt el);

  // saving tree into text
  int  getlevelsize(int lev);
  int  savelevel(char *dest,int lev);
};

#endif
