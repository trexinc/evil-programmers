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
  pchar szName;
  pchar szCont;
  struct tag_XMLNode *pNext;
  struct tag_XMLNode *pParent;
  struct tag_XMLNode *pChildren;
  PTable tAttrs;
  PHash hChildren;
  xmlNodeType eType;
}SXMLNode, *PXMLNode;

typedef int (*xmlOutStream)(void*,pchar,int);

PXMLNode xmlNew(void* Pool);
void xmlFree(PXMLNode x);
int xmlParse(void* Pool,PXMLNode x,pchar src,int flags);
PXMLNode xmlGetItem(PXMLNode x,pchar szPath);
PHashLink xmlEnumNode(PXMLNode x,pchar Key,PHashLink lnk,PXMLNode *val);
pchar xmlGetItemAttr(PXMLNode x,pchar Name);
void xmlSetItemAttr(void* Pool,PXMLNode x,pchar Name,pchar Value);
pchar xmlGetItemValue(PXMLNode x,pchar szPath,pchar szAttr);
void xmlSetItemContent(void *Pool,PXMLNode x,pchar Content);
void xmlSaveToStream(PXMLNode x,xmlOutStream out,void* data,int levelshift);
PXMLNode xmlClone(void* Pool,PXMLNode x);
PXMLNode xmlCreateItem(void* Pool,PXMLNode x,pchar szPath);
int xmlDeleteItem(PXMLNode x,pchar szPath);
PXMLNode xmlAddItem(void* Pool,PXMLNode x,pchar szName);

typedef struct tag_XMLQuery{
  char **query;
  PXMLNode *nodes;
  int count;
}SXMLQuery,*PXMLQuery;

void xmlNewQuery(pchar queryString,PXMLQuery query);
void xmlResetQuery(PXMLQuery query);
void xmlFreeQuery(PXMLQuery query);
PXMLNode xmlQueryNext(PXMLNode node,PXMLQuery query);

#ifdef __cplusplus
}
#endif

#endif
