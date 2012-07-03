/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
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
*/
#ifndef __CLockFile_hpp
#define __CLockFile_hpp

class CLockFile
{
  friend class NODEDATA;
  public:
    CLockFile(const wchar_t *FileName):hFile(INVALID_HANDLE_VALUE)
    {
      On(FileName);
    }

    ~CLockFile() { Off(); }

    void On(const wchar_t *FileName)
    {
      Off();
      Name=FileName;
      hFile=(FileName && *FileName)?
        CreateFile(FileName,GENERIC_READ,0,NULL,
          OPEN_EXISTING,0,NULL):INVALID_HANDLE_VALUE;
    }

    void Off()
    {
      if(IsOn())
      {
        CloseHandle(hFile);
        hFile=INVALID_HANDLE_VALUE;
      }
    }

    bool IsOn() const { return hFile!=INVALID_HANDLE_VALUE; }
    void On() { if(!IsOn()) On(Name.str); }

    CLockFile& operator=(const CLockFile &rhs)
    {
      if(this!=&rhs)
      {
        Off();
        On(rhs.Name.str);
      }
      return *this;
    }

  private:
    HANDLE hFile;
    strcon Name;

  private:
    CLockFile(const CLockFile& rhs);

};


#endif // __CLockFile_hpp
