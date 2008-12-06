#ifndef __CUserMacros_hpp
#define __CUserMacros_hpp

#include "rbtree.cpp"
#include "KeySequenceStorage.hpp"

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
    UserMacroID(DWORD key=0,DWORD selmode=0,DWORD buttonstate=0,
      DWORD controlstate=0): Key(key), SelectionMode(selmode),
      ButtonState(buttonstate), ControlState(controlstate)
    {}

    DWORD Key, SelectionMode, ButtonState, ControlState;
    bool operator==(const UserMacroID &rhs) const
     {
       return Key==rhs.Key && SelectionMode==rhs.SelectionMode &&
         ButtonState==rhs.ButtonState && ControlState==rhs.ControlState;
     }
    bool operator<(const UserMacroID &rhs) const
     {
       return (Key==rhs.Key)?
        (((SelectionMode==rhs.SelectionMode)?
          ((ButtonState==rhs.ButtonState)?
            ControlState<rhs.ControlState:ButtonState<rhs.ButtonState
          ):SelectionMode<rhs.SelectionMode
         )
        ):(Key<rhs.Key);
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
    bool InsertMacro(const UserMacroID &ID, const char *Str, bool silent,
                     bool stop, int &Error, strcon &unknownKey);
    bool DeleteMacro(const UserMacroID &ID);
    const KeySequence *GetMacro(const UserMacroID &ID, BOOL &Stop);
    void Empty() { Tree->Empty(); }
    bool IsEmpty() const { return Tree->IsEmpty(); }

  private:
    CRedBlackTree<OneUserMacro> *Tree;

  private:
    static BOOL WINAPI CompLT(const OneUserMacro &a,const OneUserMacro &b);
    static BOOL WINAPI CompEQ(const OneUserMacro &a,const OneUserMacro &b);

    Node *Find(const OneUserMacro &ID);
};


#endif // __CUserMacros_hpp
