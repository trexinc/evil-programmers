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

// creates object tree structure on html/xml files

// you can disable clocale class - but import string services
#include"sgmlext.hpp"
#include "memory.hpp"
//#include<stdio.h>
//#include<math.h>
//#include<float.h>


bool inline isspace(char c)
{
  if (c==0x20 || c=='\t' || c=='\r' || c=='\n') return true;
  return false;
};

//  modifed GetNumber - sign extension!
#ifdef ALLOW_VIEWER_API
bool get_number(const char *str, __int64 *res)
#else
bool get_number(const char *str, /*double*/ int *res)
#endif
{
#ifdef ALLOW_VIEWER_API
  __int64 Numr, r/*, flt*/;
  __int64 Num;
#else
  /*double*/ int Numr, r/*, flt*/;
  int Num;
#endif
  int pos, Type;
  int s, e, i, j, /*pt,*/ k/*, ExpS, ExpE*/;
  bool /*Exp = false, ExpSign = true,*/ sign = false;

  pos = lstrlen(str);
  if (!pos) return false;

  s = 0;
  e = pos;
  Type = 3;
  while(1){
    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X')){
      s = 2;
      Type = 0;
      break;
    };
    if (str[0] == '$' || str[0] == '#'){
      s = 1;
      Type = 0;
      break;
    };
    if (str[0] == '-'){
      Type = 3;
      s = 1;
      sign = true;
      break;
    };
    break;
  };

  switch(Type){
    case 0:
      Num = 0;
      i = e-1;
      while(i >= s){
        j = str[i];
        if(((j < 0x30) || (j > 0x39)) &&
          (((j | 0x20) < 'a') || ((j | 0x20) > 'f')))
            return false;
        if(j > 0x40) j -=7;
        j &=15;
        if(i > e-9) Num |= (j << ((e-i-1)*4) );
        i--;
      };
      *res = Num;
      break;
    case 3:
      /*for(i = s;i < e;i++)
      if (str[i] == 'e' || str[i] =='E'){
        Exp = true;
        ExpS = i+1;
        if (str[i+1] == '+' || str[i+1] == '-'){
          ExpS++;
          if (str[i+1] == '-') ExpSign = false;
        };
        ExpE = e;
        e = i;
      };
      pt = e;
      for(i = s;i < e;i++)
        if (str[i] == '.'){
          pt = i;
          break;
        };*/
      Numr = 0;
      i = /*pt-1*/ e-1;
      while(i >= s){
        j = str[i];
        if((j < 0x30)||(j > 0x39))
          return false;
        j &=15;
        k = /*pt*/e-i-1;
        #ifdef ALLOW_VIEWER_API
        r = (__int64)j;
        #else
        r = (/*long double*/ int)j;
        #endif
        while(k){
          k--;
          r *=10;
        };
        Numr += r;
        i--;
      };
      /*
      i = e-1;
      while(i > pt){
        j = str[i];
        if((j < 0x30)||(j > 0x39))
          return false;
        j &=15;
        k = i-pt;
        //r = 0;
        r = j;
        while(k){
          k--;
          r /=10;
        };
        Numr += r;
        i--;
      };
      if (Exp){
        flt = 0;
        i = ExpE-1;
        while(i >= ExpS){
          j = str[i];
          if((j < 0x30)||(j > 0x39))
            return false;
          j &=15;
          k = ExpE-i-1;
          r = (long double)j;
          while(k){
            k--;
            r *=10;
          };
          flt += r;
          i--;
        };
        if (ExpSign)  Numr = Numr*pow(10,flt);
        if (!ExpSign) Numr = Numr/pow(10,flt);
      };*/
      *res = Numr;
      break;
  };
  if (sign) *res = -(*res);
  return true;
};

CSgmlExt::CSgmlExt()
{
  eparent= 0;
  enext  = 0;
  eprev  = 0;
  echild = 0;
  chnum  = 0;
  type   = EBASEEL;

  name[0] = 0;
  //content= 0;
  //contentsz = 0;
  parnum = 0;
};

CSgmlExt::~CSgmlExt()
{
  if (eparent){
    eparent->chnum--;
    if (eparent->echild == this) eparent->echild = (enext ? enext : eprev);
  };
  if (enext) enext->eprev = eprev;
  if (eprev) eprev->enext = enext;
  if (type == EBASEEL && enext) enext->destroylevel();
  if (echild) echild->destroylevel();
  // if (name) delete[] name;
  //if (content) delete[] content;
  for (int i=0;i < parnum;i++){
    if (params[i][0]) free(params[i][0]);
    if (params[i][1]) free(params[i][1]);
  };
};

PSgmlExt CSgmlExt::createnew(ElType type, PSgmlExt parent, PSgmlExt after)
{
  PSgmlExt El = new CSgmlExt;
  El->type = type;
  if (parent){
    El->enext = parent->echild;
    El->eparent = parent;
    if (parent->echild) parent->echild->eprev = El;
    parent->echild = El;
    parent->chnum++;
    parent->type = EBLOCKEDEL;
  }else
    if (after) after->insert(El);
  return El;
};

bool CSgmlExt::init()
{
  return true;
};

bool CSgmlExt::parse(const char *src,int sz)
{
PSgmlExt Child, Parent, Next = 0;
int i, j;//, lins, line;
int ls, le, rs, re, empty;

  // start object - base
  type = EBASEEL;
  Next = this;

  //lins = line = 0;
  for (i = 0; i < sz; i++)
  {
    //if (i >= sz) continue;

    // comments
//    if ( *((int*)(src+i)) == '--!<' && i+4 < sz){
//    if ( src[i] == '<' && src[i+1] == '!' && src[i+2] == '-' && src[i+3] == '-' && i+4 < sz)
//    {
//      i += 4;
//      while((src[i] != '-' || src[i+1] != '-' || src[i+2] != '>') && i+3 < sz) i++;
//      i+=3;
//    }
    //line = i;

    if ( src[i] == '<' && i < sz)
    {
      /*while(line > lins){
        // linear
        j = lins;
        while(isspace(src[j]) && j < i){
          j++;
        };
        if(j == i) break; // empty text
        Child = createnew(EPLAINEL,0,Next);
        Child->init();
        Child->setcontent(src + lins, i - lins);
        Next = Child;
        break;
      };*/
      if (i == sz-1) continue;
      // start or single tag
      if (src[i+1] != '/')
      {
        Child = createnew(ESINGLEEL,NULL,Next);
        Next  = Child;
        Child->init();
        j = i+1;
        while (src[i] != '>' && !isspace(src[i]) && i < sz) i++;
        // Child->name = new char[i-j+1];
        if (i-j > MAXTAG) i = j + MAXTAG - 1;
        lstrcpyn(Child->name, src+j, i-j+1);
        Child->name[i-j] = 0;
        // parameters
        Child->parnum = 0;
        while(src[i] != '>' && Child->parnum < MAXPARAMS && i < sz){
          ls = i;
          while (isspace(src[ls]) && ls < sz) ls++;
          le = ls;
          while (!isspace(src[le]) && src[le]!='>' && src[le]!='=' && le < sz) le++;
          rs = le;
          while (isspace(src[rs]) && rs < sz) rs++;
          empty = 1;
          if (src[rs] == '='){
            empty = 0;
            rs++;
            while (isspace(src[rs]) && rs < sz) rs++;
            re = rs;
            if (src[re] == '"'){
              while(src[++re] != '"' && re < sz);
              rs++;
              i = re+1;
            /*}else if (src[re] == '\''){
              while(src[++re] != '\'' && re < sz);
              rs++;
              i = re+1;*/
            }else{
              while(!isspace(src[re]) && src[re] != '>' && re < sz) re++;
              i = re;
            };
          }else
            i = re = rs;

          if (ls == le) continue;
          if (rs == re && empty){
            rs = ls;
            re = le;
          };
          int pn = Child->parnum;
          Child->params[pn][0] = (char *)malloc(le-ls+1);
          lstrcpyn(Child->params[pn][0], src+ls, le-ls+1);
          Child->params[pn][0][le-ls] = 0;
          Child->params[pn][1] = (char *)malloc(re-rs+1);
          lstrcpyn(Child->params[pn][1], src+rs, re-rs+1);
          Child->params[pn][1][re-rs] = 0;
          Child->parnum++;
          //substquote(Child->params[pn], "&lt;", '<');
          //substquote(Child->params[pn], "&gt;", '>');
          //substquote(Child->params[pn], "&quot;", '"');
          //substquote(Child->params[pn], "&amp;", '&');
        };
        //lins = i+1;
      }else{  // end tag
        j = i+2;
        i+=2;
        while (src[i] != '>' && !isspace(src[i]) && i < sz) i++;
        int cn = 0;
        for(Parent = Next; Parent->eprev; Parent = Parent->eprev, cn++){
          if(!*Parent->name) continue;
          int len = lstrlen(Parent->name);
          if (len != i-j) continue;
          if (Parent->type != ESINGLEEL ||
            FSF.LStrnicmp((char*)src+j, Parent->name, len)) continue;
          break;
        };
        if(Parent && Parent->eprev){
          Parent->echild = Parent->enext;
          Parent->chnum = cn;
          Parent->type = EBLOCKEDEL;
          Child = Parent->echild;
          if (Child) Child->eprev = 0;
          while(Child){
            Child->eparent = Parent;
            Child = Child->enext;
          };
          Parent->enext = 0;
          Next = Parent;
        };
        while(src[i] != '>' && i < sz) i++;
        //lins = i+1;
      };
    };
  };
////
  return true;
};

void CSgmlExt::substquote(SParams par, char *sstr, char c)
{
int len = lstrlen(sstr);
int plen = lstrlen(par[1]);

  for (int i = 0; i <= plen-len; i++)
    if (!strncmp(par[1]+i, sstr, len)){
      par[1][i] = c;
      for(int j = i+1; j <= plen-len+1; j++)
        par[1][j] = par[1][j+len-1];
      plen -= len-1;
    };
};

/*bool CSgmlExt::setcontent(const char *src,int sz)
{
  content = new char[sz+1];
  memmove(content,src,sz);
  content[sz]=0;
  contentsz = sz;
  return true;
};*/

void CSgmlExt::insert(PSgmlExt El)
{
  El->eprev = this;
  El->enext = this->enext;
  El->eparent = this->eparent;
  if (this->enext) this->enext->eprev = El;
  this->enext = El;
};

// recursive deletion
void CSgmlExt::destroylevel()
{
  if (enext) enext->destroylevel();
  delete this;
};

PSgmlExt CSgmlExt::parent()
{
  return eparent;
};
PSgmlExt CSgmlExt::next()
{
  return enext;
};
PSgmlExt CSgmlExt::prev()
{
  return eprev;
};
PSgmlExt CSgmlExt::child()
{
  return echild;
};
ElType  CSgmlExt::gettype()
{
  return type;
};
char *CSgmlExt::getname()
{
  if (!*name) return NULL;
  return name;
};

/*char *CSgmlExt::getcontent()
{
  return content;
};
int CSgmlExt::getcontentsize()
{
  return contentsz;
};*/

char* CSgmlExt::GetParam(int no)
{
  if (no >= parnum) return 0;
  return params[no][0];
};
char* CSgmlExt::GetChrParam(const char *par)
{
  for (int i=0; i < parnum; i++)
    if (!FSF.LStricmp(par,params[i][0])){
      return params[i][1];
    };
  return 0;
};

bool CSgmlExt::GetIntParam(const char *par, int *result)
{
  #ifdef ALLOW_VIEWER_API
  __int64 res = 0;
  #else
  int res = 0;
  #endif
  for (int i=0; i < parnum; i++)
    if (!FSF.LStricmp(par,params[i][0])){
      bool b = get_number(params[i][1],&res);
      *result = (int)res;
      if (!b) *result = 0;
      return b;
    };
  return false;
};

#ifdef ALLOW_VIEWER_API
bool CSgmlExt::GetInt64Param(const char *par, __int64 *result)
{
  __int64 res = 0;
  for (int i=0; i < parnum; i++)
    if (!FSF.LStricmp(par,params[i][0])){
      bool b = get_number(params[i][1],&res);
      *result = res;
      if (!b) *result = 0;
      return b;
    };
  return false;
};
#endif

/*bool CSgmlExt::GetFltParam(const char *par, double *result)
{
double res;
  for (int i = 0; i < parnum; i++)
    if (!FSF.LStricmp(par,params[i][0])){
      bool b = get_number(params[i][1],&res);
      *result = (double)res;
      if (!b) *result = 0;
      return b;
    };
  return false;
};*/

PSgmlExt CSgmlExt::search(const char *TagName)
{
PSgmlExt Next = this->enext;
  while(Next){
    if (!FSF.LStricmp(TagName,Next->name)) return Next;
    Next = Next->enext;
  };
  return Next;
};

PSgmlExt CSgmlExt::enumchilds(int no)
{
PSgmlExt El = this->echild;
  while(no && El){
    El = El->enext;
    no--;
  };
  return El;
};

PSgmlExt CSgmlExt::fprev()
{
PSgmlExt El = this;
  if (!El->eprev) return El->eparent;
  if (El->eprev->echild)
    return El->eprev->echild->flast();
  return El->eprev;
};
PSgmlExt CSgmlExt::fnext()
{
PSgmlExt El = this;
  if (El->echild) return El->echild;
  while(!El->enext){
    El = El->eparent;
    if (!El) return 0;
  };
  return El->enext;
};
PSgmlExt CSgmlExt::ffirst()
{
PSgmlExt Prev = this;
  while(Prev){
    if (!Prev->eprev) return Prev;
    Prev = Prev->eprev;
  };
  return Prev;
};

PSgmlExt CSgmlExt::flast()
{
PSgmlExt Nxt = this;
  while(Nxt->enext || Nxt->echild){
    if (Nxt->enext){
      Nxt = Nxt->enext;
      continue;
    };
    if (Nxt->echild){
      Nxt = Nxt->echild;
      continue;
    };
  };
  return Nxt;
}

// lets you modify objects...
void CSgmlExt::Delete(void)
{
  PSgmlExt This = this;
  PSgmlExt Next = This->enext;
  PSgmlExt Prev = This->eprev;
  PSgmlExt Parent = This->eparent;
  This->eprev = 0;
  This->enext = 0;
  This->eparent = 0;
  if (Prev) Prev->enext = Next;
  if (Next) Next->eprev = Prev;
  if (Parent && Parent->echild == This) Parent->echild = (Prev ? Prev : Next);
  if (Parent) Parent->chnum--;
  delete This;
};

PSgmlExt CSgmlExt::addnewnext(char *src, int sz)
{
  PSgmlExt Next = new CSgmlExt;
  if (!Next)
    return NULL;
  if (!Next->parse(src,sz))
  {
    delete Next;
    return NULL;
  }
  PSgmlExt New = Next->enext;
  if (!New)
  {
    delete Next;
    return NULL;
  }
  Next->enext = 0;
  Next->echild = 0;
  Next->chnum = 0;
  Next->type = EBLOCKEDEL;
  delete Next;
  PSgmlExt This = this;
  Next = This->enext;
  This->enext = New;
  New->eprev = This;
  New->eparent = This->eparent;
  if (This->eparent)
    This->eparent->chnum++;
  while (New->enext)
  {
    if (This->eparent)
      This->eparent->chnum++;
    New = New->enext;
  }
  New->enext = Next;
  if (Next)
    Next->eprev = New;
  return New;
};

/*
PSgmlExt CSgmlExt::addnewchild(char *src, int sz)
{
  PSgmlExt Child = new CSgmlExt;
  if (!Child)
    return NULL;
  Child->eparent = this;
  if (!Child->addnewnext(src,sz))
  {
    delete Child;
    return NULL;
  }
  PSgmlExt This = this;
  Child->enext->eprev = 0;
  Child->enext->enext = This->echild;
  if (This->echild)
    This->echild->eprev = Child->enext;
  This->echild = Child->enext;
  This = Child->enext;
  Child->eparent = 0;
  Child->enext = 0;
  Child->type = EBLOCKEDEL;
  delete Child;
  return This;
}*/

void CSgmlExt::setname(char *newname)
{
/*  if (name){
    delete name;
    name = 0;
  };*/
  if (newname){
    // name = new char[lstrlen(newname)+1];
    // !!! length
    lstrcpy(name,newname);
  };
};

bool CSgmlExt::addparam(char *name, char *val)
{
int i;
  if (parnum == MAXPARAMS) return false;
  //i = parnum;
  parnum++;
  for (i = 0; i < parnum - 1; i++)
    if (!FSF.LStricmp(params[i][0],name)){
      free(params[i][0]);
      free(params[i][1]);
      parnum--;
      break;
    };
  params[i][0] = (char *)malloc(lstrlen(name)+1);
  params[i][1] = (char *)malloc(lstrlen(val)+1);
  lstrcpy(params[i][0], name);
  lstrcpy(params[i][1], val);
  return true;
};

bool CSgmlExt::addparam(char *name, int val)
{
  char IntVal[20];
  FSF.sprintf(IntVal,"%d",val);
  return addparam(name,IntVal);
};

#ifdef ALLOW_VIEWER_API
bool CSgmlExt::addparam(char *name, __int64 val)
{
  char IntVal[50];
  FSF.itoa64(val,IntVal,10);
  return addparam(name,IntVal);
};
#endif

/*bool CSgmlExt::addparam(char *name, double val)
{
char FltVal[20];
  FSF.sprintf(FltVal,"%.2f",val);
  return addparam(name,FltVal);
};*/

bool CSgmlExt::delparam(char *name)
{
  for (int i = 0; i < parnum; i++)
    if (!FSF.LStricmp(params[i][0],name)){
      free(params[i][0]);
      free(params[i][1]);
      params[i][0] = params[parnum-1][0];
      params[i][1] = params[parnum-1][1];
      parnum--;
      return true;
    };
  return false;
};

/*bool CSgmlExt::changecontent(char *data, int len)
{
  if (type != EPLAINEL) return false;
  if (content) delete[] content;
  content = new char[len];
  memmove(content, data, len);
  contentsz = len;
  return true;
};*/

bool CSgmlExt::isloop(PSgmlExt El, PSgmlExt Parent)
{
  while(Parent){
    if (El == Parent) return true;
    Parent = (PSgmlExt)Parent->eparent;
  };
  return false;
};

bool CSgmlExt::move(PSgmlExt parent, PSgmlExt after)
{
  if (isloop(this,parent)) return false;
  if (after && isloop(this,(PSgmlExt)after->eparent)) return false;
  if (after){
    if (enext) enext->eprev = eprev;
    if (eprev) eprev->enext = enext;
    if (this->eparent->echild == this)
      this->eparent->echild = this->enext;
    this->eparent->chnum--;

    after->insert(this);
    this->eparent = after->eparent;
    if (this->eparent) this->eparent->chnum++;
    return true;
  }else
  if (parent){
    if (enext) enext->eprev = eprev;
    if (eprev) eprev->enext = enext;
    if (this->eparent->echild == this)
      this->eparent->echild = this->enext;
    this->eparent->chnum--;
    this->eparent = parent;
    enext = parent->echild;
    eprev = 0;
    this->eparent->echild = this;
    this->eparent->chnum++;
    if (enext) enext->eprev = this;
    return true;
  };
  return false;
};

int CSgmlExt::getlevelsize(int Lev)
{
int Pos = 0;
PSgmlExt tmp = this;
  do{
    if (tmp->gettype() != EPLAINEL)
      Pos +=Lev*SP;
    if (tmp->name[0])
      Pos += lstrlen(tmp->name)+1;
    for (int i = 0;i < tmp->parnum;i++){
      Pos +=lstrlen(tmp->params[i][0])+2;
      Pos +=lstrlen(tmp->params[i][1])+2;
    };
    if (tmp->name[0]) Pos +=3;
    //if (tmp->gettype() == EPLAINEL && tmp->content)
      //Pos += lstrlen(tmp->content)+2;
    if (tmp->echild)
      Pos += PSgmlExt(tmp->echild)->getlevelsize(Lev+1);
    if (tmp->gettype() == EBLOCKEDEL && tmp->name){
      Pos += Lev*SP+5;
      Pos += lstrlen(tmp->name);
    };
    tmp = tmp->enext;
  }while(tmp);
  return Pos;
};

int CSgmlExt::savelevel(char *Dest,int Lev)
{
int i,Pos = 0;
PSgmlExt tmp = this;
  do{
    if (tmp->gettype() != EPLAINEL)
      for(i = 0; i < Lev*SP; i++)
        Pos += FSF.sprintf(Dest+Pos," ");
    if (tmp->name[0])
      Pos += FSF.sprintf(Dest+Pos,"<%s",tmp->name);
    for (i = 0; i < tmp->parnum; i++){
      Pos += FSF.sprintf(Dest+Pos," %s=",tmp->params[i][0]);
      Pos += FSF.sprintf(Dest+Pos,"\"%s\"",tmp->params[i][1]);
    }
    if (tmp->name[0])
      Pos += FSF.sprintf(Dest+Pos,">\r\n");
    //if (tmp->gettype() == EPLAINEL)
      //Pos += FSF.sprintf(Dest+Pos,"%s\r\n", tmp->content);
    if (tmp->echild)
      Pos += PSgmlExt(tmp->echild)->savelevel(Dest+Pos,Lev+1);
    if (tmp->gettype() == EBLOCKEDEL){
      for(i = 0; i < Lev*SP; i++)
        Pos += FSF.sprintf(Dest+Pos," ");
      Pos += FSF.sprintf(Dest+Pos,"</");
      if (tmp->name) Pos += FSF.sprintf(Dest+Pos,"%s",tmp->name);
      Pos += FSF.sprintf(Dest+Pos,">\r\n");
    };
    tmp = tmp->enext;
  }while(tmp);
  return Pos;
};
