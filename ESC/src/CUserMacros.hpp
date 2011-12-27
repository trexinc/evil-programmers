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
#ifndef __CUserMacros_hpp
#define __CUserMacros_hpp

#include "rbtree.cpp"
#include "KeySequenceStorage.hpp"

extern "C"
{
  DWORD NormalizeControlState(DWORD State);
}

enum ESC_ButtonState
{
  EBS_LClick_On     = 0x00000001,
  EBS_RClick_On     = 0x00000010,
  EBS_MClick_On     = 0x00000100,
};

enum ESC_ControlState
{
  ECS_LCtrl_On      = 0x00000001,
  ECS_RCtrl_On      = 0x00000010,

  ECS_LAlt_On       = 0x00000100,
  ECS_RAlt_On       = 0x00001000,

  ECS_Shift_On      = 0x00010000,
};

class UserMacroID
{
  public:
    UserMacroID(DWORD virtualKeyCode=0,DWORD controlKeyState=0,DWORD selmode=0,DWORD buttonstate=0,
      DWORD controlstate=0): VirtualKeyCode(virtualKeyCode), ControlKeyState(NormalizeControlState(controlKeyState)), SelectionMode(selmode),
      ButtonState(buttonstate), ControlState(controlstate)
    {}

    DWORD VirtualKeyCode, ControlKeyState, SelectionMode, ButtonState, ControlState;
    bool operator==(const UserMacroID &rhs) const
     {
       return VirtualKeyCode==rhs.VirtualKeyCode && ControlKeyState==rhs.ControlKeyState && SelectionMode==rhs.SelectionMode &&
         ButtonState==rhs.ButtonState && ControlState==rhs.ControlState;
     }
    bool operator<(const UserMacroID &rhs) const
     {
       return (VirtualKeyCode==rhs.VirtualKeyCode)?((ControlKeyState==rhs.ControlKeyState)?
        (((SelectionMode==rhs.SelectionMode)?
          ((ButtonState==rhs.ButtonState)?
            ControlState<rhs.ControlState:ButtonState<rhs.ButtonState
          ):SelectionMode<rhs.SelectionMode
         )
        ):(ControlKeyState<rhs.ControlKeyState)):(VirtualKeyCode<rhs.VirtualKeyCode);
     }
};

class OneUserMacro
{
  public:
    OneUserMacro() {}
    OneUserMacro(const UserMacroID &id):ID(id) {}
    OneUserMacro(const OneUserMacro &rhs) { *this=rhs; }
    OneUserMacro& operator=(const OneUserMacro &rhs)
    {
      ID=rhs.ID;
      Macro=rhs.Macro;
      return *this;
    }
    bool operator==(const OneUserMacro &rhs) const
     {
       return ID==rhs.ID;
     }
    bool operator<(const OneUserMacro &rhs) const
     {
       return ID<rhs.ID;
     }
  public:
    UserMacroID ID;
    KeySequenceStorage Macro;
};

class CUserMacros
{
  protected:
    typedef Node_<OneUserMacro> Node;
  public:
    CUserMacros();
    CUserMacros(const CUserMacros& rhs);
    ~CUserMacros() { delete Tree; }
    CUserMacros& operator=(const CUserMacros &rhs);

  public:
    bool InsertMacro(const UserMacroID &ID, const wchar_t *Str, bool silent,
                     bool stop, int &Error, strcon &unknownKey);
    bool DeleteMacro(const UserMacroID &ID);
    const KeySequence *GetMacro(const UserMacroID &ID, BOOL &Stop);
    void Empty() { Tree->Empty(); }
    bool IsEmpty() const { return Tree->IsEmpty()!=FALSE; }

  private:
    CRedBlackTree<OneUserMacro> *Tree;

  private:
    static BOOL WINAPI CompLT(const OneUserMacro &a,const OneUserMacro &b);
    static BOOL WINAPI CompEQ(const OneUserMacro &a,const OneUserMacro &b);

    Node *Find(const OneUserMacro &ID);
};


#endif // __CUserMacros_hpp
