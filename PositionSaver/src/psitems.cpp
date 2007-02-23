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
#include "psitems.hpp"
#include "memory.hpp"
#include "crt.hpp"

PSItems::PSItems(void)
{
  base = 0;
  *LastFileName = 0;
  memset(&LastFileTime,0,sizeof(LastFileTime));
}

PSItems::~PSItems(void)
{
  if (base) delete base;
}

bool PSItems::LoadFile(char *FileName, PSTree *tree)
{
  HANDLE fp = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  char *buf;
  DWORD sz;
  if (fp!=INVALID_HANDLE_VALUE)
  {
    FILETIME ft;
    BOOL gottime = GetFileTime(fp,NULL,NULL,&ft);
    if (!gottime || FSF.LStricmp(LastFileName,FileName) || CompareFileTime(&ft,&LastFileTime)>0)
    {
      lstrcpy(LastFileName,FileName);
      if (gottime)
        LastFileTime = ft;
      else
        memset(&LastFileTime,0,sizeof(LastFileTime));
      if (!base)
      {
        sz = GetFileSize(fp,NULL);
        if (sz == INVALID_FILE_SIZE)
          sz = 0;
        buf = (char *)malloc(sz+1);
        if (buf)
        {
          DWORD transfered;
          if (sz)
          {
            ReadFile(fp,buf,sz,&transfered,NULL);
            sz = transfered;
          }
          buf[sz]=0;
        }
      }
      CloseHandle(fp);
    }
    else
    {
      CloseHandle(fp);
      return true;
    }
  }
  else
  {
    *LastFileName = 0;
    memset(&LastFileTime,0,sizeof(LastFileTime));
    if (!base)
    {
      sz = 0;
      buf = (char *)malloc(1);
      buf[0] = 0;
    }
  }

  #if 0
  {
    HANDLE fp = CreateFile("e:\\download\\temp\\alex.alex",GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
    SetFilePointer(fp,0,NULL,FILE_END);
    DWORD t;
    WriteFile(fp,"read\r\n",6,&t,NULL);
    CloseHandle(fp);
  }
  #endif

  if (!buf && !base)
  {
    return false;
  }
  if (!base)
  {
    base = new CSgmlExt;
    if (!base)
    {
      free(buf);
      return false;
    }
    if (!base->parse(buf,sz))
    {
      delete base;
      free(buf);
      return false;
    }
    free(buf);
  }
  else
  {
    {
      PSItems temp;
      if (!temp.LoadFile(FileName,NULL))
      {
        delete base;
        return false;
      }
      struct EditorSetPosition esp;
      #ifdef ALLOW_VIEWER_API
      struct ViewerSetPosition vsp;
      #endif
      char str[_MAX_PATH];
      PSInfo info = {"editor",str,&esp};
      int type = TI_EDITOR;
      #ifdef ALLOW_VIEWER_API
      for (int i=0; i<2; i++)
      #endif
      {
        PSgmlExt Item = base->next();
        while (Item)
        {
          if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("type") && !FSF.LStricmp(Item->GetChrParam("type"),info.Type) && Item->GetChrParam("path"))
          {
            //GetItem(Item,info);
            lstrcpy(info.FileName,Item->GetChrParam("path"));
            if (!temp.FindItem(info))
            {
              if (tree)
              {
                PSTrackInfo ti;
                lstrcpy(ti.FileName,info.FileName);
                ti.Type = type;
                while (tree->GetFN(&ti))
                  tree->Delete(&ti);
              }
              Item = Item->prev();
              DelItem(info);
              #if 0
              {
                HANDLE fp = CreateFile("e:\\download\\temp\\alex.alex",GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
                SetFilePointer(fp,0,NULL,FILE_END);
                DWORD t;
                WriteFile(fp,"del\r\n",5,&t,NULL);
                CloseHandle(fp);
              }
              #endif
            }
            else
            {
              PSTrackInfo ti;
              lstrcpy(ti.FileName,info.FileName);
              ti.Type = type;
              if (!tree || !tree->FindFN(&ti))
              {
                PSgmlExt tmp = temp.PFindItem(info);
                temp.GetItem(tmp,info);
                lstrcpy(info.FileName,tmp->GetChrParam("path"));
                SetItem(info);
              }
            }
          }
          Item = Item->next();
        }
        #ifdef ALLOW_VIEWER_API
        type = TI_VIEWER;
        info.Type = "viewer";
        info.vsp = &vsp;
        #endif
      }
      #ifdef ALLOW_VIEWER_API
      info.Type = "editor";
      for (int i=0; i<2; i++)
      #endif
      {
        PSgmlExt Item = temp.Base()->next();
        while (Item)
        {
          if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("type") && !FSF.LStricmp(Item->GetChrParam("type"),info.Type) && Item->GetChrParam("path"))
          {
            GetItem(Item,info);
            lstrcpy(info.FileName,Item->GetChrParam("path"));
            if (!FindItem(info))
            {
              NewItem(info);
            }
          }
          Item = Item->next();
        }
        #ifdef ALLOW_VIEWER_API
        info.Type = "viewer";
        info.vsp = &vsp;
        #endif
      }
    }
  }
  return true;
}

bool PSItems::SaveFile(char *FileName, PSTree *tree, bool Load)
{
  if (!base)
    return false;
  if (Load)
    if (!LoadFile(FileName,tree))
      return false;
  HANDLE fp = CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
  if (fp==INVALID_HANDLE_VALUE)
    return false;
  int size = base->getlevelsize(0);
  char *buf = (char *)malloc(size+1);
  if (!buf)
  {
    CloseHandle(fp);
    return false;
  }
  base->savelevel(buf,0);
  DWORD transfered;
  WriteFile(fp,buf,size,&transfered,NULL);
  free(buf);
  CloseHandle(fp);
  return true;
}

PSgmlExt PSItems::PFindItem(PSInfo Info)
{
  if (!base)
    return NULL;
  PSgmlExt Item = base->next();
  while (Item)
  {
    if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("path") && Item->GetChrParam("type"))
    {
      if (!FSF.LStricmp(Item->GetChrParam("path"),Info.FileName) && !FSF.LStricmp(Item->GetChrParam("type"),Info.Type))
      {
        return Item;
      }
    }
    Item = Item->next();
  }
  return NULL;
}

bool PSItems::GetItem(PSgmlExt Item, PSInfo Info)
{
  if (!Item)
    return false;
  //Info.Type = Item->GetChrParam("type");
  //Info.FileName = Item->GetChrParam("path");
  if (!FSF.LStricmp(Info.Type,"editor"))
  {
    Item->GetIntParam("line",&Info.esp->CurLine);
    Item->GetIntParam("column",&Info.esp->CurPos);
    Item->GetIntParam("top",&Info.esp->TopScreenLine);
    Item->GetIntParam("left",&Info.esp->LeftPos);
    Item->GetIntParam("overtype",&Info.esp->Overtype);
    Info.esp->CurTabPos = -1;
  }
  #ifdef ALLOW_VIEWER_API
  else
  {
    Item->GetInt64Param("position",&(Info.vsp->StartPos.i64));
    Item->GetIntParam("left",&Info.vsp->LeftPos);
    Info.vsp->Flags = 0;
  }
  #endif
  return true;
}

bool PSItems::FindItem(PSInfo Info)
{
  if (!base)
    return false;
  PSgmlExt Item = base->next();
  while (Item)
  {
    if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("path") && Item->GetChrParam("type"))
    {
      if (!FSF.LStricmp(Item->GetChrParam("path"),Info.FileName) && !FSF.LStricmp(Item->GetChrParam("type"),Info.Type))
      {
        return true;
      }
    }
    Item = Item->next();
  }
  return false;
}

bool PSItems::GetItem(PSInfo Info)
{
  PSgmlExt Item = PFindItem(Info);
  return GetItem(Item,Info);
}

bool PSItems::SetItem(PSgmlExt Item, PSInfo Info)
{
  if (!Item)
    return false;
  bool ok=true;
  if (!FSF.LStricmp(Info.Type,"editor"))
  {
    ok &= Item->addparam("type",Info.Type);
    ok &= Item->addparam("path",Info.FileName);
    ok &= Item->addparam("line",Info.esp->CurLine);
    ok &= Item->addparam("column",Info.esp->CurPos);
    ok &= Item->addparam("top",Info.esp->TopScreenLine);
    ok &= Item->addparam("left",Info.esp->LeftPos);
    ok &= Item->addparam("overtype",Info.esp->Overtype);
  }
  #ifdef ALLOW_VIEWER_API
  else
  {
    ok &= Item->addparam("type",Info.Type);
    ok &= Item->addparam("path",Info.FileName);
    ok &= Item->addparam("position",Info.vsp->StartPos.i64);
    ok &= Item->addparam("left",Info.vsp->LeftPos);
  }
  #endif
  return ok;
}

bool PSItems::SetItem(PSInfo Info)
{
  PSgmlExt Item = PFindItem(Info);
  return SetItem(Item,Info);
}

bool PSItems::NewItem(PSInfo Info)
{
  if (!base)
    return false;
  char buf[] = "<item type=\"\">";
  PSgmlExt Item = base->addnewnext(buf,lstrlen(buf));
  if (!SetItem(Item,Info))
  {
    if (Item) delete Item;
    return false;
  }
  return true;
}

void PSItems::DelItem(PSInfo Info)
{
  PSgmlExt Item = PFindItem(Info);
  if (Item)
    Item->Delete();
}

PSgmlExt PSItems::Base()
{
  return base;
}

int PSItems::CountItems(const char *type)
{
  int i = 0;
  if (!base || !type)
    return i;
  PSgmlExt Item = base->next();
  while (Item)
  {
    if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("path") && Item->GetChrParam("type") && !FSF.LStricmp(Item->GetChrParam("type"),type))
      i++;
    Item = Item->next();
  }
  return i;
}

int PSItems::RemoveOld(PSTree *tree, char *FileName)
{
  int count = 0;
  if (!base)
    return count;
  //struct EditorSetPosition esp;
  char FN[_MAX_PATH];
  char type[20];
  PSInfo info = {type, FN, NULL};
  bool del = false;
  LoadFile(FileName,tree);
  PSgmlExt Item = base->next();
  while (Item)
  {
    if (Item->getname() && !FSF.LStricmp(Item->getname(),"item") && Item->GetChrParam("path") && Item->GetChrParam("type"))
    {
      lstrcpy(info.Type,Item->GetChrParam("type"));
      lstrcpy(info.FileName,Item->GetChrParam("path"));
      WIN32_FIND_DATA fd;
      HANDLE h = FindFirstFile(info.FileName,&fd);
      if (h == INVALID_HANDLE_VALUE)
      {
        if (tree)
        {
          PSTrackInfo ti;
          lstrcpy(ti.FileName,info.FileName);
          ti.Type = TI_UNKNOWN;
          if (!FSF.LStricmp(info.Type,"editor"))
            ti.Type = TI_EDITOR;
          #ifdef ALLOW_VIEWER_API
          else
            if (!FSF.LStricmp(info.Type,"viewer"))
              ti.Type = TI_VIEWER;
          #endif
          if (ti.Type!=TI_UNKNOWN && !tree->FindFN(&ti))
            del = true;
        }
        else
        {
          del = true;
        }
      }
      else
       FindClose(h);
    }
    Item = Item->next();
    if (del)
    {
      DelItem(info);
      del = false;
      count++;
    }
  }
  SaveFile(FileName,tree,false);
  return count;
}
