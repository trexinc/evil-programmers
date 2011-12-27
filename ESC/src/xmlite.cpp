/*
  [ESC] Editor's settings changer plugin for FAR Manager
  Copyright (C) 2000 Konstantin Stupnik
  Copyright (C) 2008 Alex Yaroslavsky

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

  Non validating XML parser for well-formed canonical XML.
  Can be used to read/write config files stored in XML format.
  Very fast. Very memory effecient.

*/

#include <CRT/crt.hpp>
#include "xmem.h"
#include "xmlite.h"
#include "syslog.hpp"
#include "plugin.hpp"

extern struct FarStandardFunctions FSF;

#define CHK if(i==-1)return 0


PXMLNode xmlNew(void* Pool)
{
  PXMLNode x;
  x=(PXMLNode)xcalloc(Pool,sizeof(SXMLNode));
  x->eType=xmlRoot;
  return x;
}

static PXMLNode xmlNewChild(void* Pool,PXMLNode p,xmlNodeType e)
{
  PXMLNode x;
  x=(PXMLNode)xcalloc(Pool,sizeof(SXMLNode));
  x->eType=e;
  x->pParent=p;
  return x;
}


void xmlFree(PXMLNode x)
{
/*  PXMLNode c,n;
  if(x->eType==xmlRoot || x->eType==xmlNode)
  {
    c=x->pChildren;
    while(c)
    {
      n=c->pNext;
      xmlFree(c);
      c=n;
    }
    hashFree(x->hChildren);
  }
  if(x->tAttrs)
  {
    wchar_t *k,*v;
    tableFirst(x->tAttrs);
    while(tableNext(x->tAttrs,&k,&v))
    {
//      xfree(k);
      xfree(v);
    }
    tableFree(x->tAttrs);
  }
  xfree(x->szCont);
  xfree(x);*/
}

static int xmlIsSpace(wchar_t c)
{
  return c==0x20||c==0x0d||c==0x0a||c==0x09;
}

static wchar_t x_wc[256]={
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#define WC(a) (a>255?0:x_wc[a])

//extern int strtol(wchar_t *,wchar_t **,int);
//extern int sprintf(wchar_t *,wchar_t *,...);

static wchar_t* xmlSubst(wchar_t* str)
{
  wchar_t *s=str;
  wchar_t *dst=str;//xstrchr(str,'&');
//  if(!dst)return str;
//  str=dst;
  while(xmlIsSpace(*str))str++;
  for(;*str;str++,dst++)
  {
    if(*str==L'&')
    {
      if(str[1]==L'#')
      {
        int symb;
        if(str[2]==L'x')
        {
          symb=wcstol(str+3,&str,16);
        }else
        {
          symb=wcstol(str+2,&str,10);
        }
        *dst=(wchar_t)symb;
        continue;
      }else
      if(xstrncmp(str+1,L"amp;",4))
      {
        *dst=*str;
        str+=4;
        continue;
      }else
      if(xstrncmp(str+1,L"lt;",3))
      {
        *dst=L'<';
        str+=3;
        continue;
      }else
      if(xstrncmp(str+1,L"gt;",3))
      {
        *dst=L'>';
        str+=3;
        continue;
      }else
      if(xstrncmp(str+1,L"quot;",5))
      {
        *dst=L'"';
        str+=5;
        continue;
      }else
      if(xstrncmp(str+1,L"apos;",5))
      {
        *dst=L'\'';
        str+=5;
        continue;
      }
    }
    else if(xmlIsSpace(*str))
    {
      str++;
      while(xmlIsSpace(*str))str++;
      if(!*str)break;
      *dst=L' ';
      str--;
      continue;
    }
    *dst=*str;
  }
  *dst=0;
  return s;
}

static int xmlSkipTill(wchar_t *str,int pos,wchar_t lim)
{
  while(str[pos] && str[pos]!=lim)pos++;
  if(!str[pos])return -1;
  return pos;
}

static int xmlSkipSpace(wchar_t *str,int pos)
{
  while(xmlIsSpace(str[pos]))pos++;
  if(!str[pos])return -1;
  return pos;
}

static int xmlGetWordEnd(wchar_t *str,int pos)
{
  while(WC(str[pos]))pos++;
  if(!str[pos])return -1;
  return pos;
}
/*
static PXMLNode xmlEndOfList(PXMLNode p)
{
  PXMLNode n;
  if(!p)return NULL;
  n=p->pNext;
  while(n)
  {
    p=n;
    n=p->pNext;
  }
  return p;
}*/

static int xmlParseTag(void* Pool,PXMLNode x,PXMLNode* p,wchar_t *src,int i)
{
  int st,j,q;
  for(;;)
  {
    i=xmlSkipSpace(src,i);CHK;
    if(src[i]==L'/')
    {
      if(src[i+1]!=L'>')return 0;
      x->eType=xmlEmpty;
      return i+2;
    }
    if(src[i]=='>')
    {
      *p=x;
      return i+1;
    }
    st=i;
    i=xmlGetWordEnd(src,i);CHK;
    if(src[i]!=L'=')return 0;
    if(src[i+1]!=L'"' && src[i+1]!=L'\'')return 0;
    q=src[i+1];
    src[i]=0;
    i+=2;
    j=i;
    i=xmlSkipTill(src,i,(wchar_t)q);
    if(i==-1)
    {
      src[j-2]=L'=';
      return 0;
    }
    if(x->tAttrs==NULL)x->tAttrs=tableNew(Pool,TABLE_FLAG_ALLOCNAME);
    tableAdd(x->tAttrs,src+st,xmlSubst(xstrndup(Pool,src+j,i-j)));
    src[j-2]=L'=';
    i++;
  }
//  return i;
}

int xmlParse(void* Pool,PXMLNode x,wchar_t *src,int flags)
{
  int sz=xstrlen(src);
  int i,st,n;
  wchar_t str[256];
  PXMLNode stack[256];
  int stackpos=0;
  PXMLNode p=x,c;
  PHashLink l;
  PXMLNode chlds=NULL;
  if(!sz)return 0;

  for(i=0;i<sz;i++)
  {
    if(xmlIsSpace(src[i]))continue;
    if(src[i]==L'<' && src[i+1]==L'!' && src[i+2]==L'-' && src[i+3]==L'-' && sz-i>4)
    {
      i+=4;
      st=i;
      while((src[i]!=L'-' || src[i+1]!=L'-' || src[i+2]!=L'>') && sz-i>3)i++;
      i+=2;
      if(sz-i<3)return 0;
      c=xmlNewChild(Pool,p,xmlComment);
      c->szCont=xstrndup(Pool,src+st,i-st-3);
      if(chlds)chlds->pNext=c;
      else p->pChildren=c;
      chlds=c;
      continue;
    }
    if(src[i]==L'<')
    {
      i++;
      if(src[i]==L'/')
      {
        i++;
        st=i;
        i=xmlGetWordEnd(src,i);
        if(i==-1)return 0;
        i=xmlSkipSpace(src,i);CHK;
        if(src[i]!=L'>')return 0;
        n=i-st;
        if(!xstrncmp(p->szName,src+st,n))return 0;
        //i++;
        p=p->pParent;
        if(p->eType==xmlRoot)return 1;
        stackpos--;
        chlds=stack[stackpos];
        continue;
      }
      if(WC(src[i]))
      {
        st=i;
        i=xmlGetWordEnd(src,i);CHK;
        c=xmlNewChild(Pool,p,xmlLeaf);
        if(p->eType!=xmlRoot)p->eType=xmlNode;
        n=i-st;
        if(n>255)n=255;
        xstrncpy(str,src+st,n);
        if(p->hChildren==NULL)p->hChildren=hashNew(Pool);
        l=hashAdd(p->hChildren,str,c);
        c->szName=l->szKey;
        if(chlds)chlds->pNext=c;
        else p->pChildren=c;
        chlds=c;
        i=xmlParseTag(Pool,c,&p,src,i);
        if(i==0)return 0;
        if(p==c)
        {
          stack[stackpos]=chlds;
          stackpos++;
          chlds=NULL;
        }
        i=xmlSkipSpace(src,i);CHK;
        st=i;
        i=xmlSkipTill(src,i,L'<');CHK;
        if(i>st)
        {
          p->szCont=xmlSubst(xstrndup(Pool,src+st,i-st));
        }
        i--;
        continue;
      }
      if(src[i]==L'!' || src[i]==L'?')
      {
        st=i+1;
        if(src[i+1]==L'[')
        {
          if(xstrncmp(src+i,L"![CDATA[",8))
          {
            const wchar_t *p=src+i+8;
            while((p=xstrchr(p,']')))
            {
              if(p[1]==L']' && p[2]==L'>')break;
            }
            if(p)i=(int)(p-src+2);
          }else i=xmlSkipTill(src,i,L'>');
        }else
        {
          i=xmlSkipTill(src,i,L'>');
        }
        if(i==-1)return 0;
        c=xmlNewChild(Pool,p,src[st-1]==L'!'?xmlExcl:xmlQuest);
        c->szCont=xstrndup(Pool,src+st,i-st);
        if(chlds)chlds->pNext=c;
        else p->pChildren=c;
        chlds=c;
        continue;
      }
    }else
    {
      st=i;
      i=xmlSkipTill(src,i,L'<');CHK;
      c=xmlNewChild(Pool,p,xmlContent);
      c->szCont=xmlSubst(xstrndup(Pool,src+st,i-st));
      if(chlds)chlds->pNext=c;
      else p->pChildren=c;
      chlds=c;
      i--;
    }
  }
  return 1;
}

PXMLNode xmlGetItem(PXMLNode x,const wchar_t *szPath)
{
  const wchar_t *s;
  if(!x)return NULL;
  if(*szPath==L'/')
  {
    while(x->pParent)x=x->pParent;
    szPath++;
  }
  for(;;)
  {
    s=xstrchr(szPath,L'/');
    if(!s)return (PXMLNode)hashGet(x->hChildren,szPath);
    //*s=0;
    x=(PXMLNode)hashGetEx(x->hChildren,szPath,(int)(s-szPath));
    //*s='/';
    if(!x)return NULL;
    szPath=s+1;
  }
}

const wchar_t *xmlGetItemValue(PXMLNode x,const wchar_t *szPath,const wchar_t *szAttr)
{
  x=xmlGetItem(x,szPath);
  if(!x)return NULL;
  if(szAttr)
  {
    return xmlGetItemAttr(x,szAttr);
  }else
  {
    return x->szCont;
  }

}

PHashLink xmlEnumNode(PXMLNode x,const wchar_t *Key,PHashLink lnk,PXMLNode *val)
{
  if(!x)return NULL;
  return hashEnumKey(x->hChildren,Key,lnk,(void**)val);
}

const wchar_t *xmlGetItemAttr(PXMLNode x, const wchar_t *Name)
{
  if(!x)return NULL;
  if(!x->tAttrs)return NULL;
  return tableGet(x->tAttrs,Name);
}

void xmlSetItemAttr(void* Pool,PXMLNode x,const wchar_t *Name,const wchar_t *Value)
{
  if(!x)return;
  if(!x->tAttrs)x->tAttrs=tableNew(Pool,TABLE_FLAG_ALLOCNAME);
  tableSet(x->tAttrs,Name,xstrdup(Pool,Value));
}

void xmlSetItemContent(void *Pool,PXMLNode x,const wchar_t *Content)
{
  if(!x)return;
  if(xstrlen(x->szCont)>=xstrlen(Content))
    xstrcpy(x->szCont,Content);
  else
    x->szCont=xstrdup(Pool,Content);
  if(x->eType==xmlEmpty)x->eType=xmlLeaf;
}

static wchar_t x_toquot[]=
{
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

#define TOQUOT(a) (a>255?1:x_toquot[a])


#define OUTSTRL(s,l) if(l && !out(data,s,l))return 0
#define OUTSTR(s) if(!out(data,s,xstrlen(s)))return 0

static int xmlSaveString(xmlOutStream out,void* data,const wchar_t *str)
{
  int i,l;
  wchar_t num[8];
  i=0;l=0;
  if(!str)return 1;
  while(str[i])
  {
    while(str[i] && !TOQUOT(str[i]))i++;
    OUTSTRL(str+l,i-l);
    if(str[i])
    {
      if     (str[i]==L'<' ){OUTSTRL(L"&lt;"  ,4);}
      else if(str[i]==L'>' ){OUTSTRL(L"&gt;"  ,4);}
      else if(str[i]==L'&' ){OUTSTRL(L"&amp;" ,5);}
      else if(str[i]==L'"' ){OUTSTRL(L"&quot;",6);}
      else if(str[i]==L'\''){OUTSTRL(L"&apos;",6);}
      else
      {
        FSF.sprintf(num,L"&#x%04x;",(int)(str[i]));
        //sprintf(num,"&#x%02x;",(int)((unsigned wchar_t)str[i]));
        OUTSTR(num);
      }
      i++;
      l=i;
    }
  }
  return 1;
}

static int xmlSaveLevel(PXMLNode x,xmlOutStream out,void* data,int level,int levelshift)
{
  wchar_t space[256];
  wchar_t *k,*v;
  int shift;
  shift=level*levelshift;
  if(shift)
  {
    wmemset(space,' ',shift);
  }
  for(;x;x=x->pNext)
  {
    if(x->eType==xmlQuest)
    {
      OUTSTRL(L"<?",2);
      OUTSTR(x->szCont);
      OUTSTRL(L">\n",2);
      continue;
    }
    if(x->eType==xmlExcl)
    {
      OUTSTRL(L"<!",2);
      OUTSTR(x->szCont);
      OUTSTRL(L">\n",2);
      continue;
    }
    OUTSTRL(space,shift);
    if(x->eType==xmlComment)
    {
      OUTSTRL(L"<!--",4);
      OUTSTR(x->szCont);
      OUTSTRL(L"-->\n",4);
      continue;
    }
    if(x->eType==xmlContent)
    {
      OUTSTR(x->szCont);
      OUTSTRL(L"\n",1);
      continue;
    }
    OUTSTRL(L"<",1);
    OUTSTR(x->szName);
    if(x->tAttrs)
    {
      tableFirst(x->tAttrs);
      while(tableNext(x->tAttrs,&k,&v))
      {
        OUTSTRL(L" ",1);
        OUTSTR(k);
        OUTSTRL(L"=\"",2);
        if(!xmlSaveString(out,data,v))return 0;
        OUTSTRL(L"\"",1);
      }
    }
    if(x->eType==xmlEmpty)OUTSTRL(L"/",1);
    OUTSTRL(L">",1);
    if(x->eType==xmlNode || x->eType==xmlEmpty)OUTSTRL(L"\n",1);
    if(x->szCont)
    {
      if(x->eType==xmlNode)
      {
        OUTSTRL(space,shift);
        OUTSTRL(space,levelshift);
      }
      if(!xmlSaveString(out,data,x->szCont))return 0;
      if(x->eType==xmlNode)OUTSTRL(L"\n",1);
    }
    if(x->eType==xmlNode)
    {
      if(!xmlSaveLevel(x->pChildren,out,data,level+1,levelshift))return 0;
      OUTSTRL(space,shift);
      OUTSTRL(L"</",2);
      OUTSTR(x->szName);
      OUTSTRL(L">\n",2);
    }else
    if(x->eType==xmlLeaf)
    {
      OUTSTRL(L"</",2);
      OUTSTR(x->szName);
      OUTSTRL(L">\n",2);
    }
  }
  return 1;
}

void xmlSaveToStream(PXMLNode x,xmlOutStream out,void* data,int levelshift)
{
  xmlSaveLevel(x->eType==xmlRoot?x->pChildren:x,out,data,0,levelshift);
}

static void xmlCloneTree(void* Pool,PXMLNode x,PXMLNode nx)
{
  nx->eType=x->eType;
  if(x->szCont)nx->szCont=xstrdup(Pool,x->szCont);

  if(x->tAttrs)
  {
    wchar_t *k,*v;
    tableFirst(x->tAttrs);
    nx->tAttrs=tableNew(Pool,TABLE_FLAG_ALLOCNAME);
    while(tableNext(x->tAttrs,&k,&v))
    {
      tableSet(nx->tAttrs,k,v);
    }
  }
  if(x->hChildren)
  {
    PXMLNode p=x->pChildren;
    PXMLNode np=NULL,c;
    PHashLink l;
    nx->hChildren=hashNewEx(Pool,x->hChildren->iCount);
    while(p)
    {
      c=xmlNewChild(Pool,nx,p->eType);
      if(np)np->pNext=c;
      else nx->pChildren=c;
      np=c;
      //c->pParent=nx;
      if(p->szName)
      {
        l=hashAdd(nx->hChildren,p->szName,c);
        c->szName=l->szKey;
      }
      xmlCloneTree(Pool,p,c);
      p=p->pNext;
    }
  }
}

PXMLNode xmlClone(void* Pool,PXMLNode x)
{
  PXMLNode nx;
  nx=xmlNew(Pool);
  xmlCloneTree(Pool,x,nx);
  return nx;
}

static PXMLNode xmlCreateChild(void* Pool,PXMLNode x,const wchar_t *Name)
{
  PXMLNode c;
  PHashLink l;
  if(x->hChildren)
  {
    c=(PXMLNode)hashGet(x->hChildren,Name);
    if(c)return c;
  }
  c=xmlNewChild(Pool,x,xmlEmpty);
  c->pNext=x->pChildren;
  x->pChildren=c;
  if(x->hChildren==NULL)x->hChildren=hashNew(Pool);
  l=hashAdd(x->hChildren,Name,c);
  c->szName=l->szKey;
  return c;
}

PXMLNode xmlAddItem(void* Pool,PXMLNode x,const wchar_t *szName)
{
  PXMLNode c;
  PHashLink l;
  c=xmlNewChild(Pool,x,xmlEmpty);
  c->pNext=x->pChildren;
  x->pChildren=c;
  x->eType=xmlNode;
  if(x->hChildren==NULL)x->hChildren=hashNew(Pool);
  l=hashAdd(x->hChildren,szName,c);
  c->szName=l->szKey;
  return c;
}


PXMLNode xmlCreateItem(void* Pool,PXMLNode x,const wchar_t *szPath)
{
  const wchar_t *s;
  if(!x)return NULL;
  if(*szPath==L'/')
  {
    while(x->pParent)x=x->pParent;
    szPath++;
  }
  for(;;)
  {
    s=xstrchr(szPath,L'/');
    if(!s)
    {
      return xmlCreateChild(Pool,x,szPath);
    }
    *((wchar_t *)s)=0;
    x=xmlCreateChild(Pool,x,szPath);
    x->eType=xmlNode;
    *((wchar_t *)s)=L'/';
    szPath=s+1;
  }
}

int xmlDeleteItem(PXMLNode x,const wchar_t *szPath)
{
  PXMLNode t,q,p=xmlGetItem(x,szPath);
  const wchar_t *name;
  int ok=0;
  if(!p)return 0;
  name=wcsrchr(szPath,L'/');
  if(!name)name=szPath;
  else name++;
  p=p->pParent;
  hashDelete(p->hChildren,name);
  q=p->pChildren;
  t=NULL;
  while(q)
  {
    if(!lstrcmp(q->szName,name))
    {
      if(t)
      {
        t->pNext=q->pNext;
      }else
      {
        p->pChildren=q->pNext;
      }
      t=q->pNext;
      xmlFree(q);
      q=t;
      ok++;
    }
    t=q;
    if(q)q=q->pNext;
  }
  return ok;
}

void xmlNewQuery(const wchar_t *queryString,PXMLQuery query)
{
  int i,cnt=1;
  const wchar_t *q;
  const wchar_t *p=wcschr(queryString,L'.');
  while(p)
  {
    cnt++;
  p++;
    p=wcschr(p,L'.');
  }
  query->query=(wchar_t**)malloc(sizeof(wchar_t*)*cnt);
  query->nodes=(PXMLNode*)malloc(sizeof(PXMLNode)*cnt);
  p=queryString;
  for(i=0;i<cnt;i++)
  {
    query->nodes[i]=0;
    q=wcschr(p,L'.');
    if(!q)
    {
      query->query[i]=wcsdup(p);
    }else
    {
      query->query[i]=(wchar_t*)malloc((q-p+1)*sizeof(wchar_t));
      memcpy(query->query[i],p,(q-p)*sizeof(wchar_t));
      query->query[i][q-p]=0;
      p=q+1;
    }
  }
  query->count=cnt;
}

void xmlResetQuery(PXMLQuery query)
{
  int i;
  for(i=0;i<query->count;i++)
  {
    query->nodes[i]=0;
  }
}

void xmlFreeQuery(PXMLQuery query)
{
  int i;
  if(query->query)
  {
    for(i=0;i<query->count;i++)
    {
      if(query->query[i])free(query->query[i]);
    }
    free(query->query);
  }
  if(query->nodes)free(query->nodes);
}

static int xmlMatchPattern(wchar_t *pattern,wchar_t *name)
{
  while(*pattern)
  {

    if(*pattern==L'*')
    {
      if(pattern[1])
      {
        while(*name && pattern[1]!=*name)name++;
      }else
      {
        return 1;
      }
      pattern++;
    }
    if(*pattern!=*name)return 0;
    pattern++;
    name++;
  }
  return *name==0;
}

PXMLNode xmlQueryNext(PXMLNode node,PXMLQuery query)
{
  PXMLNode p;
  int i;
  if(query->nodes[0]==0)
  {
    p=node;
    if(p->eType==xmlRoot)p=p->pChildren;
    for(i=0;i<query->count;i++)
    {
      while(p && !xmlMatchPattern(query->query[i],p->szName))
      {
        p=p->pNext;
      }
      if(!p)return 0;
      query->nodes[i]=p;
      p=p->pChildren;
    }
    return query->nodes[query->count-1];
  }
  i=query->count-1;
  while(i>=0 && !query->nodes[i])i--;
  if(i<0)return 0;
  p=query->nodes[i]=query->nodes[i]->pNext;
  for(;;)
  {
    if(!p)
    {
      i--;
      if(i<0)return 0;
      p=query->nodes[i]=query->nodes[i]->pNext;
    }else
    {
      if(xmlMatchPattern(query->query[i],p->szName))
      {
        if(i==query->count-1)return p;
        p=p->pChildren;
        i++;
        query->nodes[i]=p;
      }else
      {
        p=query->nodes[i]=query->nodes[i]->pNext;
      }
    }
  }
}
