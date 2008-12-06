/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@aurorisoft.com>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Non validating XML parser for well-formed canonical XML.
  Can be used to read/write config files stored in XML format.
  Very fast. Very memory effecient.

*/

#include "xmem.h"
#include "xmlite.h"
#include "myrtl.hpp"
#include "syslog.hpp"
#include "plugin.hpp"
extern struct FarStandardFunctions FSF;

#define CHK if(i==-1)return 0


PXMLNode xmlNew(void* Pool)
{
  PXMLNode x;
  x=xcalloc(Pool,sizeof(SXMLNode));
  x->eType=xmlRoot;
  return x;
}

static PXMLNode xmlNewChild(void* Pool,PXMLNode p,xmlNodeType e)
{
  PXMLNode x;
  x=xcalloc(Pool,sizeof(SXMLNode));
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
    pchar k,*v;
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

static int xmlIsSpace(xchar c)
{
  return c==0x20||c==0x0d||c==0x0a||c==0x09;
}

static xchar wc[256]={
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


//extern int strtol(pchar,pchar*,int);
//extern int sprintf(pchar,pchar,...);

static xchar* xmlSubst(xchar* str)
{
  pchar s=str;
  pchar dst=str;//xstrchr(str,'&');
//  if(!dst)return str;
//  str=dst;
  while(xmlIsSpace(*str))str++;
  for(;*str;str++,dst++)
  {
    if(*str=='&')
    {
      if(str[1]=='#')
      {
        int symb;
        if(str[2]=='x')
        {
          symb=strtol(str+3,&str,16);
        }else
        {
          symb=strtol(str+2,&str,10);
        }
        *dst=(xchar)symb;
        continue;
      }else
      if(xstrncmp(str+1,"amp;",4))
      {
        *dst=*str;
        str+=4;
        continue;
      }else
      if(xstrncmp(str+1,"lt;",3))
      {
        *dst='<';
        str+=3;
        continue;
      }else
      if(xstrncmp(str+1,"gt;",3))
      {
        *dst='>';
        str+=3;
        continue;
      }else
      if(xstrncmp(str+1,"quot;",5))
      {
        *dst='"';
        str+=5;
        continue;
      }else
      if(xstrncmp(str+1,"apos;",5))
      {
        *dst='\'';
        str+=5;
        continue;
      }
    }
    else if(xmlIsSpace(*str))
    {
      str++;
      while(xmlIsSpace(*str))str++;
      if(!*str)break;
      *dst=' ';
      str--;
      continue;
    }
    *dst=*str;
  }
  *dst=0;
  return s;
}

static int xmlSkipTill(pchar str,int pos,xchar lim)
{
  while(str[pos] && str[pos]!=lim)pos++;
  if(!str[pos])return -1;
  return pos;
}

static int xmlSkipSpace(pchar str,int pos)
{
  while(xmlIsSpace(str[pos]))pos++;
  if(!str[pos])return -1;
  return pos;
}

static int xmlGetWordEnd(pchar str,int pos)
{
  while(wc[(xuchar)str[pos]])pos++;
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

static int xmlParseTag(void* Pool,PXMLNode x,PXMLNode* p,pchar src,int i)
{
  int st,j,q;
  for(;;)
  {
    i=xmlSkipSpace(src,i);CHK;
    if(src[i]=='/')
    {
      if(src[i+1]!='>')return 0;
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
    if(src[i]!='=')return 0;
    if(src[i+1]!='"' && src[i+1]!='\'')return 0;
    q=src[i+1];
    src[i]=0;
    i+=2;
    j=i;
    i=xmlSkipTill(src,i,(xchar)q);
    if(i==-1)
    {
      src[j-2]='=';
      return 0;
    }
    if(x->tAttrs==NULL)x->tAttrs=tableNew(Pool,TABLE_FLAG_ALLOCNAME);
    tableAdd(x->tAttrs,src+st,xmlSubst(xstrndup(Pool,src+j,i-j)));
    src[j-2]='=';
    i++;
  }
//  return i;
}

int xmlParse(void* Pool,PXMLNode x,pchar src,int flags)
{
  int sz=xstrlen(src);
  int i,st,n;
  xchar str[256];
  PXMLNode stack[256];
  int stackpos=0;
  PXMLNode p=x,c;
  PHashLink l;
  PXMLNode chlds=NULL;
  if(!sz)return 0;

  for(i=0;i<sz;i++)
  {
    if(xmlIsSpace(src[i]))continue;
    if(src[i]=='<' && src[i+1]=='!' && src[i+2]=='-' && src[i+3]=='-' && sz-i>4)
    {
      i+=4;
      st=i;
      while((src[i]!='-' || src[i+1]!='-' || src[i+2]!='>') && sz-i>3)i++;
      i+=3;
      if(sz-i<3)return 0;
      c=xmlNewChild(Pool,p,xmlComment);
      c->szCont=xstrndup(Pool,src+st,i-st-3);
      if(chlds)chlds->pNext=c;
      else p->pChildren=c;
      chlds=c;
      continue;
    }
    if(src[i]=='<')
    {
      i++;
      if(src[i]=='/')
      {
        i++;
        st=i;
        i=xmlGetWordEnd(src,i);
        if(i==-1)return 0;
        i=xmlSkipSpace(src,i);CHK;
        if(src[i]!='>')return 0;
        n=i-st;
        if(!xstrncmp(p->szName,src+st,n))return 0;
        //i++;
        p=p->pParent;
        if(p->eType==xmlRoot)return 1;
        stackpos--;
        chlds=stack[stackpos];
        continue;
      }
      if(wc[(xuchar)src[i]])
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
        i=xmlSkipTill(src,i,'<');CHK;
        if(i>st)
        {
          p->szCont=xmlSubst(xstrndup(Pool,src+st,i-st));
        }
        i--;
        continue;
      }
      if(src[i]=='!' || src[i]=='?')
      {
        st=i+1;
        if(src[i+1]=='[')
        {
          if(xstrncmp(src+i,"![CDATA[",8))
          {
            pchar p=src+i+8;
            while((p=xstrchr(p,']')))
            {
              if(p[1]==']' && p[2]=='>')break;
            }
            if(p)i=p-src+2;
          }else i=xmlSkipTill(src,i,'>');
        }else
        {
          i=xmlSkipTill(src,i,'>');
        }
        if(i==-1)return 0;
        c=xmlNewChild(Pool,p,src[st-1]=='!'?xmlExcl:xmlQuest);
        c->szCont=xstrndup(Pool,src+st,i-st);
        if(chlds)chlds->pNext=c;
        else p->pChildren=c;
        chlds=c;
        continue;
      }
    }else
    {
      st=i;
      i=xmlSkipTill(src,i,'<');CHK;
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

PXMLNode xmlGetItem(PXMLNode x,pchar szPath)
{
  pchar s;
  if(!x)return NULL;
  if(*szPath=='/')
  {
    while(x->pParent)x=x->pParent;
    szPath++;
  }
  for(;;)
  {
    s=xstrchr(szPath,'/');
    if(!s)return hashGet(x->hChildren,szPath);
    //*s=0;
    x=hashGetEx(x->hChildren,szPath,s-szPath);
    //*s='/';
    if(!x)return NULL;
    szPath=s+1;
  }
}

pchar xmlGetItemValue(PXMLNode x,pchar szPath,pchar szAttr)
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

PHashLink xmlEnumNode(PXMLNode x,pchar Key,PHashLink lnk,PXMLNode *val)
{
  if(!x)return NULL;
  return hashEnumKey(x->hChildren,Key,lnk,(void*)val);
}

pchar xmlGetItemAttr(PXMLNode x,pchar Name)
{
  if(!x)return NULL;
  if(!x->tAttrs)return NULL;
  return tableGet(x->tAttrs,Name);
}

void xmlSetItemAttr(void* Pool,PXMLNode x,pchar Name,pchar Value)
{
  if(!x)return;
  if(!x->tAttrs)x->tAttrs=tableNew(Pool,TABLE_FLAG_ALLOCNAME);
  tableSet(x->tAttrs,Name,xstrdup(Pool,Value));
}

void xmlSetItemContent(void *Pool,PXMLNode x,pchar Content)
{
  if(!x)return;
  if(xstrlen(x->szCont)>=xstrlen(Content))
    xstrcpy(x->szCont,Content);
  else
    x->szCont=xstrdup(Pool,Content);
  if(x->eType==xmlEmpty)x->eType=xmlLeaf;
}

static xchar toquot[]=
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




#define OUT(s,l) if(l && !out(data,s,l))return 0
#define OUTSTR(s) if(!out(data,s,xstrlen(s)))return 0

static int xmlSaveString(xmlOutStream out,void* data,pchar str)
{
  int i,l;
  char num[8];
  i=0;l=0;
  if(!str)return 1;
  while(str[i])
  {
    while(str[i] && !toquot[(xuchar)str[i]])i++;
    OUT(str+l,i-l);
    if(str[i])
    {
      if     (str[i]=='<' ){OUT("&lt;"  ,4);}
      else if(str[i]=='&' ){OUT("&amp;" ,5);}
      else if(str[i]=='"' ){OUT("&quot;",6);}
      else if(str[i]=='\''){OUT("&apos;",6);}
      else
      {
        FSF.sprintf(num,"&#x%02x;",(int)((xuchar)str[i]));
        //sprintf(num,"&#x%02x;",(int)((xuchar)str[i]));
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
  xchar space[256];
  pchar k,v;
  int shift;
  shift=level*levelshift;
  if(shift)
  {
    memset(space,' ',shift);
  }
  for(;x;x=x->pNext)
  {
    if(x->eType==xmlQuest)
    {
      OUT("<?",2);
      OUTSTR(x->szCont);
      OUT(">\n",2);
      continue;
    }
    if(x->eType==xmlExcl)
    {
      OUT("<!",2);
      OUTSTR(x->szCont);
      OUT(">\n",2);
      continue;
    }
    OUT(space,shift);
    if(x->eType==xmlComment)
    {
      OUT("<!--",4);
      OUTSTR(x->szCont);
      OUT("-->\n",4);
      continue;
    }
    if(x->eType==xmlContent)
    {
      OUTSTR(x->szCont);
      OUT("\n",1);
      continue;
    }
    OUT("<",1);
    OUTSTR(x->szName);
    if(x->tAttrs)
    {
      tableFirst(x->tAttrs);
      while(tableNext(x->tAttrs,&k,&v))
      {
        OUT(" ",1);
        OUTSTR(k);
        OUT("=\"",2);
        if(!xmlSaveString(out,data,v))return 0;
        OUT("\"",1);
      }
    }
    if(x->eType==xmlEmpty)OUT("/",1);
    OUT(">",1);
    if(x->eType==xmlNode || x->eType==xmlEmpty)OUT("\n",1);
    if(x->szCont)
    {
      if(x->eType==xmlNode)
      {
        OUT(space,shift);
        OUT(space,levelshift);
      }
      if(!xmlSaveString(out,data,x->szCont))return 0;
      if(x->eType==xmlNode)OUT("\n",1);
    }
    if(x->eType==xmlNode)
    {
      if(!xmlSaveLevel(x->pChildren,out,data,level+1,levelshift))return 0;
      OUT(space,shift);
      OUT("</",2);
      OUTSTR(x->szName);
      OUT(">\n",2);
    }else
    if(x->eType==xmlLeaf)
    {
      OUT("</",2);
      OUTSTR(x->szName);
      OUT(">\n",2);
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
    pchar k,v;
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

static PXMLNode xmlCreateChild(void* Pool,PXMLNode x,pchar Name)
{
  PXMLNode c;
  PHashLink l;
  if(x->hChildren)
  {
    c=hashGet(x->hChildren,Name);
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

PXMLNode xmlAddItem(void* Pool,PXMLNode x,pchar szName)
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


PXMLNode xmlCreateItem(void* Pool,PXMLNode x,pchar szPath)
{
  pchar s;
  if(!x)return NULL;
  if(*szPath=='/')
  {
    while(x->pParent)x=x->pParent;
    szPath++;
  }
  for(;;)
  {
    s=xstrchr(szPath,'/');
    if(!s)
    {
      return xmlCreateChild(Pool,x,szPath);
    }
    *s=0;
    x=xmlCreateChild(Pool,x,szPath);
    x->eType=xmlNode;
    *s='/';
    szPath=s+1;
  }
}

int xmlDeleteItem(PXMLNode x,pchar szPath)
{
  PXMLNode t,q,p=xmlGetItem(x,szPath);
  char *name;
  int ok=0;
  if(!p)return 0;
  name=strrchr(szPath,'/');
  if(!name)name=szPath;
  else name++;
  p=p->pParent;
  hashDelete(p->hChildren,name);
  q=p->pChildren;
  t=NULL;
  while(q)
  {
    if(!strcmp(q->szName,name))
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

void xmlNewQuery(pchar queryString,PXMLQuery query)
{
  int i,cnt=1;
  pchar q;
  pchar p=strchr(queryString,'.');
  while(p)
  {
    cnt++;
  p++;
    p=strchr(p,'.');
  }
  query->query=(char**)malloc(sizeof(char*)*cnt);
  query->nodes=(PXMLNode*)malloc(sizeof(PXMLNode)*cnt);
  p=queryString;
  for(i=0;i<cnt;i++)
  {
    query->nodes[i]=0;
    q=strchr(p,'.');
    if(!q)
    {
      query->query[i]=strdup(p);
    }else
    {
      query->query[i]=(char*)malloc(q-p+1);
      memcpy(query->query[i],p,q-p);
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

static int xmlMatchPattern(pchar pattern,pchar name)
{
  while(*pattern)
  {

    if(*pattern=='*')
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
