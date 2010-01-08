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

  Non validating XML parser for well-formed canonical XML.
  Can be used to read/write config files stored in XML format.
  Very fast. Very memory effecient.
*/

#ifndef __XMLITE_HPP__
#define __XMLITE_HPP__
#include "xmem.h"
#include "xmlite.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
  inline bool Next(CXMLNode& res);
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
    char *buf=new char[size+1];
    fread(buf,size,1,f);
    buf[size]=0;
    fclose(f);
    pPool=xmemNewPool(32+size*4);
    pNode=xmlNew(pPool);
    int res=xmlParse(pPool,pNode,buf,0);
    delete [] buf;
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

  bool AttrExists(const pchar name)
  {
    if(!pNode)return false;
    return tableGet(pNode->tAttrs,name)!=NULL;
  }

  int AttrAsInt(const pchar name)
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
  friend struct CXMLQuery;

  void AttrFirst()
  {
    if(!pNode)return;
    tableFirst(pNode->tAttrs);
  }

  bool AttrNext(pchar& name,pchar& value)
  {
    if(!pNode)return false;
    return tableNext(pNode->tAttrs,&name,&value)!=0;
  }

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
