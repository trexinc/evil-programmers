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

  This is collection of auxilary functions that use almost
  all 'Classes'.
*/

#include "XTools.hpp"
#include <stdio.h>

namespace XClasses{

int Split(const String& source,const String& pattern,StrList& dest,int maxnum)
{
  int pos,oldpos=0;
  String tmp;
  dest.Clean();
  while((pos=source.Index(pattern,oldpos))!=-1)
  {
    tmp.Set(source,oldpos,pos-oldpos);
    dest<<tmp;
    oldpos=pos+pattern.Length();
    maxnum--;
    if(maxnum==0)break;
  }
  tmp.Set(source,oldpos,source.Length()-oldpos);
  dest<<tmp;
  return dest.Count();
}

int StrList::LoadFromFile(const char* filename)
{
  String s;
  char xbuf[512];
  char *buf=xbuf;
  int size=sizeof(xbuf);
  int islocal=1;
  FILE *f=fopen(filename,"rt");
  if(!f)return 0;
  int pos=0;
  while(fgets(buf,size,f))
  {
    int len=strlen(buf);
    if(buf[len-1]!=0x0a && !feof(f))
    {
      if(!islocal)
      {
        delete [] buf;
      }
      size*=2;
      buf=new char[size];
      islocal=0;
      fseek(f,pos,SEEK_SET);
      continue;
    }
    Append("").Set(buf,0,buf[len-1]==0x0a?len-1:len);
    pos=ftell(f);
  }
  fclose(f);
  return 1;
}

int StrList::SaveToFile(const char* filename)
{
  FILE *f=fopen(filename,"wt+");
  if(!f)return 0;
  for(int i=0;i<Count();i++)
  {
    Goto(i);
    fwrite(Get().Str(),Get().Length(),1,f);
    fwrite("\n",1,1,f);
  }
  fclose(f);
  return 1;
}

bool StrList::Find(const String& str)
{
  StrList::ListSave ls;
  Save(ls);
  for(int i=0;i<Count();i++)
  {
    if(Goto(i).Get()==str)return true;
  }
  return false;
}


static int szcmp(const void* a,const void* b)
{
  return strcmp(*((char**)a),*((char**)b));
}


void StrList::Sort(StrList& dest)
{
  char**arr=new char*[Count()];
  int i;
  for(i=0;i<Count();i++)
  {
    arr[i]=const_cast<char*>((*this)[i].Str());
  }
  qsort(arr,Count(),sizeof(char*),szcmp);
  dest.Clean();
  for(i=0;i<Count();i++)
  {
    dest<<arr[i];
  }
  delete [] arr;
}


int RegScanf(const String& str,const char* rx,StrList& dst)
{
  RegExp r;
  if(!r.Compile(rx))return -1;
  int n=r.GetBracketsCount();
  SMatch *m=new SMatch[n];
  if(!r.Match(str,((const char*)str)+str.Length(),m,n))
  {
    delete [] m;
    return 0;
  }
  dst.Clean();
  for(int i=0;i<n;i++)
  {
    dst<<"";
    if(m[i].start!=-1 && m[i].end!=-1)
    {
      dst.Last().Get().Set(str,m[i].start,m[i].end-m[i].start);
    }
  }
  return dst.Count();
}

}//namespace XClasses
