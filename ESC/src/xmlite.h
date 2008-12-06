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

#ifndef __XML_H__
#define __XML_H__

#include "hash.h"
#include "table.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_xmlLeafType{
  xmlRoot,xmlNode,xmlLeaf,xmlEmpty,xmlContent,xmlComment,xmlQuest,xmlExcl
}xmlNodeType;

typedef struct tag_XMLNode{
  wchar_t *szName;
  wchar_t *szCont;
  struct tag_XMLNode *pNext;
  struct tag_XMLNode *pParent;
  struct tag_XMLNode *pChildren;
  PTable tAttrs;
  PHash hChildren;
  xmlNodeType eType;
}SXMLNode, *PXMLNode;

typedef int (*xmlOutStream)(void*,const wchar_t*,int);

PXMLNode xmlNew(void* Pool);
void xmlFree(PXMLNode x);
int xmlParse(void* Pool,PXMLNode x,wchar_t *src,int flags);
PXMLNode xmlGetItem(PXMLNode x,const wchar_t *szPath);
PHashLink xmlEnumNode(PXMLNode x,const wchar_t *Key,PHashLink lnk,PXMLNode *val);
const wchar_t *xmlGetItemAttr(PXMLNode x,const wchar_t *Name);
void xmlSetItemAttr(void* Pool,PXMLNode x,const wchar_t *Name,const wchar_t *Value);
const wchar_t *xmlGetItemValue(PXMLNode x,const wchar_t *szPath,const wchar_t *szAttr);
void xmlSetItemContent(void *Pool,PXMLNode x,const wchar_t *Content);
void xmlSaveToStream(PXMLNode x,xmlOutStream out,void* data,int levelshift);
PXMLNode xmlClone(void* Pool,PXMLNode x);
PXMLNode xmlCreateItem(void* Pool,PXMLNode x,const wchar_t *szPath);
int xmlDeleteItem(PXMLNode x,const wchar_t *szPath);
PXMLNode xmlAddItem(void* Pool,PXMLNode x,const wchar_t *szName);

typedef struct tag_XMLQuery{
  wchar_t **query;
  PXMLNode *nodes;
  int count;
}SXMLQuery,*PXMLQuery;

void xmlNewQuery(const wchar_t *queryString,PXMLQuery query);
void xmlResetQuery(PXMLQuery query);
void xmlFreeQuery(PXMLQuery query);
PXMLNode xmlQueryNext(PXMLNode node,PXMLQuery query);

#ifdef __cplusplus
}
#endif

#endif
