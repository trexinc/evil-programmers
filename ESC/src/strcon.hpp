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
#ifndef __strcon_hpp
#define __strcon_hpp

class strcon
{
   public:
     strcon(const wchar_t *ptr, int size=-1);
     strcon(const strcon &ptr);
     strcon();
     ~strcon();

   public:
     bool operator==(const strcon &ptr) const;
     bool operator==(const wchar_t * ptr) const;
     strcon& operator=(const wchar_t *ptr);
     strcon& operator=(const strcon &ptr);
     const strcon& AddStr(const wchar_t *s, int size=-1);
     const strcon& AddStr(const strcon &s);
     const strcon& AddChar(wchar_t Chr);
     const strcon& AddStrings(const wchar_t *firstStr, ...);
     unsigned int getLength() const { return Len; }
     bool setLength(unsigned newLen);
     wchar_t *setStr(const wchar_t *s, int size=-1);

   private:
     void Free();

   public:
     wchar_t *str;

   private:
     unsigned int Len;
};

#endif //__strcon_hpp
