/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group

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
#include "farmail.hpp"

HeaderList::HeaderList()
{
  name.list=attach.list=NULL;
  value.list=NULL;
  name.count=value.count=attach.count=0;
  related=false;
}

const char *HeaderList::GetStr(struct HEADERSTRUCT *ptr, size_t index)
{
  return (char *)(index<ptr->count?*(ptr->list+index):NULL);
}

const char *HeaderList::GetSubStr(struct HEADERPTRSTRUCT *ptr, size_t index, size_t sub_index)
{
  return (char *)((index<ptr->count)?GetStr(&ptr->list[index],sub_index):NULL);
}

const char *HeaderList::GetName(size_t index)
{
  return GetStr(&name, index);
}

const char *HeaderList::GetValue(size_t index, size_t sub_index)
{
  return GetSubStr(&value, index, sub_index);
}

const char *HeaderList::GetAttach(size_t index)
{
  return GetStr(&attach, index);
}

size_t HeaderList::Count(struct HEADERSTRUCT *ptr)
{
  return ptr->count;
}

size_t HeaderList::CountSub(struct HEADERPTRSTRUCT *ptr)
{
  return ptr->count;
}

size_t HeaderList::CountHeader(void)
{
  return (Count(&name) == CountSub(&value) ? Count(&name) : 0);
}

size_t HeaderList::CountValues(size_t index)
{
  return (index<CountHeader()?Count(&value.list[index]):0);
}

size_t HeaderList::CountAttach(void)
{
  return Count(&attach);
}

void HeaderList::DecodeName(FARMail *obj, char *charset, size_t index)
{
  if (!name.count || index>=name.count)
    return;
  obj->DecodeStr8(name.list[index], charset);
}

void HeaderList::DecodeValues(FARMail *obj, char *charset, size_t index)
{
  if (!value.count || index>=value.count)
    return;
  for (size_t i=0; i < CountValues(index); i++)
    obj->DecodeStr8(value.list[index].list[i], charset);
}

void HeaderList::DecodeAttach(FARMail *obj, char *charset, size_t index)
{
  if (!attach.count || index>=attach.count)
    return;
  obj->DecodeStr8(attach.list[index], charset);
}

void HeaderList::EncodeName(FARMail *obj, char *charset, size_t index)
{
  if (!name.count || index>=name.count)
    return;
  obj->EncodeStr8(name.list[index], charset);
}

void HeaderList::EncodeValues(FARMail *obj, char *charset, size_t index)
{
  if (!value.count || index>=value.count)
    return;
  for (size_t i=0; i < CountValues(index); i++)
    obj->EncodeStr8(value.list[index].list[i], charset);
}

void HeaderList::EncodeAttach(FARMail *obj, char *charset, size_t index)
{
  if (!attach.count || index>=attach.count)
    return;
  obj->DecodeStr8(attach.list[index], charset);
}

BOOL HeaderList::Insert(struct HEADERSTRUCT *ptr, size_t index, const char *str)
{
  if (!ptr->count || index>=ptr->count || !str)
    return FALSE;
  if (!Append(ptr, ptr->list[ptr->count-1]))
    return FALSE;
  for (size_t i=ptr->count-2; i > index; i--)
    Swap(ptr, i-1, i);
  char *temp = (char *)z_realloc(ptr->list[index], (lstrlen(str)+1)*sizeof(char));
  if (!temp)
  {
    Delete(ptr, index);
    return FALSE;
  }
  ptr->list[index] = temp;
  lstrcpy(ptr->list[index], str);
  return TRUE;
}

BOOL HeaderList::InsertSub(struct HEADERPTRSTRUCT *ptr, size_t index, const char *str)
{
  if (!ptr->count || index>=ptr->count || !str)
    return FALSE;
  if (!AppendSub(ptr, str))
    return FALSE;
  for (size_t i=ptr->count-1; i > index; i--)
    SwapSub(ptr, i-1, i);
  return TRUE;
}

BOOL HeaderList::InsertHeader(size_t index,const char *NewName, const char* NewValue)
{
  if (!Insert(&name, index, NewName))
    return FALSE;
  if (!InsertSub(&value, index, NewValue))
  {
    Delete(&name, index);
    return FALSE;
  }
  return TRUE;
}

BOOL HeaderList::InsertHeader(size_t index,const char *NewName)
{
  return InsertHeader(index, NewName, NULLSTR);
}

BOOL HeaderList::InsertValue(size_t index, size_t sub_index, const char* NewValue)
{
  return ((index<CountHeader() && Insert(&value.list[index], sub_index, NewValue))?TRUE:FALSE);
}

BOOL HeaderList::InsertAttach(size_t index,const char *NewAttach)
{
  return Insert(&attach, index, NewAttach);
}

BOOL HeaderList::Append(struct HEADERSTRUCT *ptr, const char *str)
{
  if (!str)
    return FALSE;
  char **temp = (char **)z_realloc(ptr->list, (ptr->count+1)*sizeof(char *));
  if (!temp)
  {
    ptr->list = (char **)z_realloc(ptr->list, ptr->count*sizeof(char *));
    return FALSE;
  }
  ptr->list = temp;
  ptr->list[ptr->count] = (char *)z_calloc(lstrlen(str)+1, sizeof(char));
  if (!ptr->list[ptr->count])
    return FALSE;
  lstrcpy(ptr->list[ptr->count], str);
  ptr->count++;
  return TRUE;
}

BOOL HeaderList::AppendSub(struct HEADERPTRSTRUCT *ptr, const char *str)
{
  if (!str)
    return FALSE;
  struct HEADERSTRUCT *temp = (struct HEADERSTRUCT *)z_realloc(ptr->list,(ptr->count+1)*sizeof(struct HEADERSTRUCT ));
  if (!temp)
  {
    ptr->list = (struct HEADERSTRUCT *)z_realloc(ptr->list, ptr->count*sizeof(struct HEADERSTRUCT ));
    return FALSE;
  }
  ptr->list = temp;
  ptr->list[ptr->count].count = 0;
  ptr->list[ptr->count].list = NULL;
  if (!Append(&ptr->list[ptr->count],str))
  {
    ptr->list = (struct HEADERSTRUCT *)z_realloc(ptr->list, ptr->count*sizeof(struct HEADERSTRUCT ));
    return FALSE;
  }
  ptr->count++;
  return TRUE;
}

BOOL HeaderList::AppendHeader(const char *NewName,const char *NewValue)
{
  if (!Append(&name, NewName))
    return FALSE;
  if (!AppendSub(&value, NewValue))
  {
    Delete(&name, name.count-1);
    return FALSE;
  }
  return TRUE;
}

BOOL HeaderList::AppendHeader(const char *NewName)
{
  return AppendHeader(NewName, NULLSTR);
}

BOOL HeaderList::AppendValue(size_t index, const char *NewValue)
{
  return (index<CountHeader() && Append(&value.list[index], NewValue)?TRUE:FALSE);
}

BOOL HeaderList::AppendAttach(const char *NewAttach)
{
  return Append(&attach, NewAttach);
}

void HeaderList::Delete(struct HEADERSTRUCT *ptr, size_t index)
{
  if (!ptr->count || index>=ptr->count)
    return;
  ptr->count--;
  for (size_t i=index; i<ptr->count; i++)
    Swap(ptr, i, i+1);
  z_free(ptr->list[ptr->count]);
  ptr->list = (char **) z_realloc(ptr->list, ptr->count*sizeof(char *));
}

void HeaderList::DeleteSub(struct HEADERPTRSTRUCT *ptr, size_t index)
{
  if (!ptr->count || index>=ptr->count)
    return;
  ptr->count--;
  for (size_t i=index; i<ptr->count; i++)
    SwapSub(ptr, i, i+1);
  Clear(&ptr->list[ptr->count]);
  ptr->list = (struct HEADERSTRUCT *) z_realloc(ptr->list, ptr->count*sizeof(struct HEADERSTRUCT ));
}

void HeaderList::DeleteHeader(size_t index)
{
  Delete(&name, index);
  DeleteSub(&value, index);
}

void HeaderList::DeleteValue(size_t index, size_t sub_index)
{
  if (index < CountHeader())
    Delete(&value.list[index], sub_index);
}

void HeaderList::DeleteAttach(size_t index)
{
  Delete(&attach, index);
}

BOOL HeaderList::SetStr(struct HEADERSTRUCT *ptr, size_t index, const char *str)
{
  if (!ptr->count || index>=ptr->count || !str)
    return FALSE;
  char *temp = (char *) z_realloc(ptr->list[index], (lstrlen(str)+1)*sizeof(char));
  if (!temp)
    return FALSE;
  ptr->list[index] = temp;
  lstrcpy(ptr->list[index], str);
  return TRUE;
}

BOOL HeaderList::SetSubStr(struct HEADERPTRSTRUCT *ptr, size_t index, size_t sub_index, const char *str)
{
  if (!ptr->count || index>=ptr->count || !str)
    return FALSE;
  return SetStr(&ptr->list[index], sub_index, str);
}

BOOL HeaderList::SetValue(size_t index, size_t sub_index, const char *NewValue)
{
  return SetSubStr(&value, index, sub_index, NewValue);
}

BOOL HeaderList::SetValue(const char *Name, const char *NewValue)
{
  if (!name.count || !Name || !NewValue)
    return FALSE;
  for (size_t i=0; i<name.count; i++)
  {
    if (!lstrcmp(name.list[i], Name))
    {
      Clear(&value.list[i]);
      Append(&value.list[i], NewValue);
    }
  }
  return TRUE;
}

void HeaderList::Swap(struct HEADERSTRUCT *ptr, size_t index1, size_t index2)
{
  if (!ptr->count || index1>=ptr->count || index2>=ptr->count || index1==index2)
    return;
  char *temp = ptr->list[index1];
  ptr->list[index1] = ptr->list[index2];
  ptr->list[index2] = temp;
}

void HeaderList::SwapSub(struct HEADERPTRSTRUCT *ptr, size_t index1, size_t index2)
{
  if (!ptr->count || index1>=ptr->count || index2>=ptr->count || index1==index2)
    return;
  struct HEADERSTRUCT temp = ptr->list[index1];
  ptr->list[index1] = ptr->list[index2];
  ptr->list[index2] = temp;
}

void HeaderList::Clear(struct HEADERSTRUCT *ptr)
{
  for (size_t i=0; i<ptr->count; i++)
  {
    z_free(ptr->list[i]);
  }
  z_free(ptr->list);
  ptr->list = NULL;
  ptr->count = 0;
}

void HeaderList::ClearSub(struct HEADERPTRSTRUCT *ptr)
{
  for (size_t i=0; i<ptr->count; i++)
  {
    Clear(&ptr->list[i]);
  }
  z_free(ptr->list);
  ptr->list = NULL;
  ptr->count = 0;
}

void HeaderList::ClearAll(void)
{
  Clear(&name);
  ClearSub(&value);
  Clear(&attach);
}

HeaderList::~HeaderList()
{
  ClearAll();
}

bool HeaderList::GetRelated(void)
{
  return related;
}

void HeaderList::SetRelated(bool Value)
{
  related=Value;
}
