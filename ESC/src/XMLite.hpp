/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@aurorisoft.com>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Non validating XML parser for well-formed canonical XML.
  Can be used to read/write config files stored in XML format.
  Very fast. Very memory effecient.
*/

#ifndef __XMLITE_HPP__
#define __XMLITE_HPP__
#include "xmem.h"
#include "xmlite.h"
// #include <string.h>
// #include <stdio.h>
// #include <stdlib.h>
#include "myrtl.hpp"
#include "mix.hpp"
#include "syslog.hpp"

#define NULLCHECK if(!pNode)return NULL

#ifdef __ATTRCANRETURN_NULL
#error __ATTRCANRETURN_NULL already defined!
#endif
//#define __ATTRCANRETURN_NULL

class CXMLNode;

struct CXMLQuery{
  SXMLQuery query;
  CXMLNode* node;
  CXMLQuery()
  {
    node=0;
    query.query=0;
    query.nodes=0;
    query.count=0;
  }
  ~CXMLQuery()
  {
    xmlFreeQuery(&query);
  }
  bool Next(CXMLNode& res);
protected:
  CXMLQuery(const CXMLQuery&);
  void operator=(const CXMLQuery&);
};

class CXMLNode{
protected:
  void *pPool;
  PXMLNode pNode;
  int *refcount;

  void AddRef()
  {
    if(!refcount)
    {
      refcount=new int;
      *refcount=0;
    }
    (*refcount)++;
  }
static inline int __savetofile(void *f,pchar data,int length)
{
  return fwrite(data,length,1,(FILE*)f);
}
public:
  typedef PHashLink Iterator;

  CXMLNode()
  {
    pPool=NULL;
    pNode=NULL;
    refcount=NULL;
    AddRef();
  }
  CXMLNode(const CXMLNode& src)
  {
    pPool=src.pPool;
    pNode=src.pNode;
    refcount=src.refcount;
    AddRef();
  }
  virtual ~CXMLNode()
  {
    Free();
  }
  void Free()
  {
    (*refcount)--;
    if(*refcount)return;
    if(pPool)
    {
      xmemFreePool(pPool);
    }
    pPool=NULL;
    pNode=NULL;
    delete refcount;
    refcount=NULL;
  }

  bool IsValid(void) const {return pNode != 0;}

  void operator=(const CXMLNode& src)
  {
    Free();
    pPool=src.pPool;
    pNode=src.pNode;
    refcount=src.refcount;
    AddRef();
  }

  int Parse(const pchar src)
  {
    int l=strlen(src);
    pPool=xmemNewPool(32+l*4);
    pNode=xmlNew(pPool);
    return xmlParse(pPool,pNode,src,0);
  }

  int ParseFile(const char* filename)
  {
    FILE *f=fopen(filename,"rt");
    if(!f)return 0;
    fseek(f,0,SEEK_END);
    int size=ftell(f);
    fseek(f,0,SEEK_SET);
    char *buf=(char*)malloc(size+1);
    int res=0;
    if(buf)
    {
      size_t FREAD=fread(buf,size,1,f);
      fclose(f);
      if(FREAD)
      {
        buf[size]=0;
        pPool=xmemNewPool(32+size*4);
        pNode=xmlNew(pPool);
        res=xmlParse(pPool,pNode,buf,0);
      }
      free (buf);
    }
    else
      fclose(f);
    return res;
  }

  pchar Name()
  {
    NULLCHECK;
    return pNode->szName;
  }
  pchar Content()
  {
    NULLCHECK;
    return pNode->szCont;
  }
  int AttrAsInt(const char* name)
  {
    if(!pNode)return 0;
    pchar tmp=xmlGetItemAttr(pNode,(char*)name);
    if(!tmp)return 0;
    return atoi(tmp);
  }
  pchar Attr(const char* name)
  {
    #ifdef __ATTRCANRETURN_NULL
      NULLCHECK;
      return xmlGetItemAttr(pNode,(char*)name);
    #else
      if(!pNode) return "";
      pchar tmp=xmlGetItemAttr(pNode,(char*)name);
      return tmp?tmp:"";
    #endif
  }

  xmlNodeType Type()
  {
    return pNode->eType;
  }

  CXMLNode GetItem(const pchar path)
  {
    CXMLNode tmp(*this);
    tmp.pNode=xmlGetItem(pNode,path);
    return tmp;
  }
  Iterator EnumName(const pchar name,Iterator lnk,CXMLNode& node)
  {
    NULLCHECK;
    return xmlEnumNode(pNode,name,lnk,&node.pNode);
  }
  Iterator Enum(Iterator lnk,CXMLNode& node)
  {
    NULLCHECK;
    if(lnk==NULL)
    {
      node.pNode=pNode->pChildren;
      return (Iterator)pNode->pChildren;
    }else
    {
      node.pNode=((PXMLNode)lnk)->pNext;
      return (Iterator)node.pNode;
    }
  }
  CXMLNode Parent()
  {
    CXMLNode tmp=*this;
    if(!pNode)return tmp;
    tmp.pNode=tmp.pNode->pParent;
    return tmp;
  }
  CXMLNode CreateItem(const pchar path)
  {
    if(!pPool || !pNode)
    {
      CXMLNode tmp;
      return tmp;
    }
    CXMLNode tmp=*this;
    tmp.pNode=xmlCreateItem(pPool,pNode,path);
    return tmp;
  }

  CXMLNode AddChild(const pchar name)
  {
    if(!pPool || !pNode)
    {
      CXMLNode tmp;
      return tmp;
    }
    CXMLNode tmp=*this;
    tmp.pNode=xmlAddItem(pPool,pNode,name);
    return tmp;
  }

  int DeleteItem(const pchar path)
  {
    return xmlDeleteItem(pNode,path);
  }

  CXMLNode& SetAttr(const pchar name,const pchar value)
  {
    if(!pPool || !pNode)return *this;
    xmlSetItemAttr(pPool,pNode,name,value);
    return *this;
  }

  Iterator EnumAttrs(Iterator iter,pchar& name,pchar& value)
  {
    if(!iter)tableFirst(pNode->tAttrs);
    return (Iterator)tableNext(pNode->tAttrs,&name,&value);
  }

  void SetContent(const pchar cont)
  {
    if(!pPool || !pNode)return;
    xmlSetItemContent(pPool,pNode,cont);
  }
  pchar GetValue(pchar path,pchar attr=NULL)
  {
    NULLCHECK;
    return xmlGetItemValue(pNode,path,attr);
  }
  void Clone(CXMLNode& dst)
  {
    dst.Free();
    dst.pPool=xmemNewPool(xmemUsage(pPool)/4);
    dst.pNode=xmlClone(dst.pPool,pNode);
    dst.refcount=new int;
    (*dst.refcount)=1;
  }
  int SaveToFile(const pchar filename,int levelshift=2)
  {
    FILE *f=fopen(filename,"wt+");
    if(!f)return 0;
    xmlSaveToStream(pNode,__savetofile,(void*)f,levelshift);
    fclose(f);
    return 1;
  }
  void CreateQuery(const pchar queryString,CXMLQuery& query)
  {
    xmlNewQuery(queryString,&query.query);
    query.node=this;
  }
  friend class CXMLQuery;
};

bool CXMLQuery::Next(CXMLNode& res)
{
  if(!node)return false;
  res=*node;
  res.pNode=xmlQueryNext(node->pNode,&query);
  return res.pNode!=NULL;
}


#undef __ATTRCANRETURN_NULL

#endif
