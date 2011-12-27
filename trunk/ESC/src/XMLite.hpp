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

#ifndef __XMLITE_HPP__
#define __XMLITE_HPP__
#include "xmem.h"
#include "xmlite.h"
#include "mix.hpp"
#include "syslog.hpp"
#include "plugin.hpp"
#include "myrtl.hpp"

extern struct FarStandardFunctions FSF;

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
static inline int __savetofile(void *f,const wchar_t *data,int length)
{
  return (int)fwrite(data,length*sizeof(wchar_t),1,(FILE*)f);
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

  int Parse(wchar_t *src)
  {
    int l=lstrlen(src);
    pPool=xmemNewPool((32+l*4)*sizeof(wchar_t));
    pNode=xmlNew(pPool);
    return xmlParse(pPool,pNode,src,0);
  }

  int ParseFile(const wchar_t* filename)
  {
    FILE *f=wfopen(filename,L"rt");
    if(!f)return 0;
    fseek(f,0,SEEK_END);
    int size=ftell(f)/sizeof(wchar_t);
    fseek(f,0,SEEK_SET);
    wchar_t *buf=(wchar_t*)malloc((size+1)*sizeof(wchar_t));
    int res=0;
    if(buf)
    {
      size_t FREAD=fread(buf,size*sizeof(wchar_t),1,f);
      fclose(f);
      if(FREAD)
      {
        buf[size]=0;
        pPool=xmemNewPool((32+size*4)*sizeof(wchar_t));
        pNode=xmlNew(pPool);
        res=xmlParse(pPool,pNode,buf,0);
      }
      free (buf);
    }
    else
      fclose(f);
    return res;
  }

  const wchar_t *Name()
  {
    NULLCHECK;
    return pNode->szName;
  }
  const wchar_t *Content()
  {
    NULLCHECK;
    return pNode->szCont;
  }
  int AttrAsInt(const wchar_t *name)
  {
    if(!pNode)return 0;
    const wchar_t *tmp=xmlGetItemAttr(pNode,name);
    if(!tmp)return 0;
    return FSF.atoi(tmp);
  }
  const wchar_t *Attr(const wchar_t *name)
  {
    #ifdef __ATTRCANRETURN_NULL
      NULLCHECK;
      return xmlGetItemAttr(pNode,(wchar_t*)name);
    #else
      static wchar_t empty[] = L"";
      if(!pNode) return empty;
      const wchar_t *tmp=xmlGetItemAttr(pNode,name);
      return tmp?tmp:empty;
    #endif
  }

  xmlNodeType Type()
  {
    return pNode->eType;
  }

  CXMLNode GetItem(const wchar_t *path)
  {
    CXMLNode tmp(*this);
    tmp.pNode=xmlGetItem(pNode,path);
    return tmp;
  }
  Iterator EnumName(const wchar_t *name,Iterator lnk,CXMLNode& node)
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
  CXMLNode CreateItem(const wchar_t *path)
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

  CXMLNode AddChild(const wchar_t *name)
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

  int DeleteItem(const wchar_t *path)
  {
    return xmlDeleteItem(pNode,path);
  }

  CXMLNode& SetAttr(const wchar_t *name,const wchar_t *value)
  {
    if(!pPool || !pNode)return *this;
    xmlSetItemAttr(pPool,pNode,name,value);
    return *this;
  }

  /*
  Iterator EnumAttrs(Iterator iter,wchar_t *& name,wchar_t *& value)
  {
    if(!iter)tableFirst(pNode->tAttrs);
    return (Iterator)tableNext(pNode->tAttrs,&name,&value);
  }*/

  void SetContent(const wchar_t *cont)
  {
    if(!pPool || !pNode)return;
    xmlSetItemContent(pPool,pNode,cont);
  }
  const wchar_t *GetValue(const wchar_t *path,const wchar_t *attr=NULL)
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
  int SaveToFile(const wchar_t *filename,int levelshift=2)
  {
    FILE *f=wfopen(filename,L"wt+");
    if(!f)return 0;
    xmlSaveToStream(pNode,__savetofile,(void*)f,levelshift);
    fclose(f);
    return 1;
  }
  void CreateQuery(const wchar_t *queryString,CXMLQuery& query)
  {
    xmlNewQuery(queryString,&query.query);
    query.node=this;
  }
  friend struct CXMLQuery;
};

#undef __ATTRCANRETURN_NULL

#endif
