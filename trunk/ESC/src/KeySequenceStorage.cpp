#ifndef __KeySequenceStorage_cpp
#define __KeySequenceStorage_cpp

#include "myrtl.hpp"
#include "KeySequenceStorage.hpp"
#include "kserror.hpp"
#include "syslog.hpp"

extern FarStandardFunctions FSF;
extern BOOL IsNewMacro;

KeySequenceStorage::KeySequenceStorage(const char *str,bool silent,DWORD total,
                                       BOOL stop)
{
  Init();
  if(str)
  {
    int error;
    strcon unknownKey;
    compile(str,silent,total,stop,error,unknownKey);
  }
}

KeySequenceStorage::KeySequenceStorage(const KeySequenceStorage& Value)
{
  Init();
  Copy(Value);
}

void KeySequenceStorage::Init()
{
  Stop=TRUE;
  Sequence.Sequence=NULL;
  Free();
}

KeySequenceStorage& KeySequenceStorage::operator=(const KeySequenceStorage& Value)
{
  if(this!=&Value)
  {
    Free();
    Copy(Value);
  }
  return *this;
}

void KeySequenceStorage::Free()
{
  if(Sequence.Sequence)
  {
    free(Sequence.Sequence);
    Sequence.Sequence=NULL;
  }
  Sequence.Count=0;
  Sequence.Flags=0;
}

void KeySequenceStorage::Copy(const KeySequenceStorage& Value)
{
  Sequence.Flags=Value.Sequence.Flags;
  if(Value.Sequence.Sequence)
  {
    if(IsNewMacro)
      Sequence.Sequence=static_cast<DWORD *>(malloc(Value.Sequence.Count+1));
    else
      Sequence.Sequence=static_cast<DWORD *>(malloc(Value.Sequence.Count*sizeof(DWORD)));

    if(Sequence.Sequence)
    {
      Stop=Value.Stop;
      Sequence.Count=Value.Sequence.Count;
      if(IsNewMacro)
        memcpy(Sequence.Sequence,Value.Sequence.Sequence,Value.Sequence.Count+1);
      else
        memcpy(Sequence.Sequence,Value.Sequence.Sequence,Value.Sequence.Count*sizeof(DWORD));
    }
  }
}

bool KeySequenceStorage::compile(const char *BufPtr, bool silent, DWORD total,
                                 BOOL stop,int &Error,strcon &unknownKey)
{
  _D(SysLog("compile: start total=%d, (unknownKey='%s') [%s]",
     total, unknownKey.str?unknownKey.str:"(null)", BufPtr?BufPtr:"(null)"));
  Free();
  Error=KSE_SUCCESS;
  unknownKey="";
  if(!BufPtr || !*BufPtr)
  {
    _D(SysLog("compile: error Buf (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
    return false;
  }
  if(silent) Sequence.Flags=KSFLAGS_DISABLEOUTPUT;
  Stop=stop;

  //parse
  if(IsNewMacro)
  {
    DWORD tmpSize=lstrlen(BufPtr);
    Sequence.Count=0;
    Sequence.Sequence=static_cast<DWORD *>(malloc((tmpSize+1)*total+1));
    _D(SysLog("compile: new size=%d", (tmpSize+1)*total+1));
    if(Sequence.Sequence)
    {
      for(DWORD I=1;I<=total;++I)
      {
        memcpy(reinterpret_cast<char *>(Sequence.Sequence)+Sequence.Count,BufPtr,tmpSize);
        if(I<total)
        {
          *(reinterpret_cast<char *>(Sequence.Sequence)+Sequence.Count+tmpSize)=0x20;
          ++Sequence.Count;
        }
        Sequence.Count+=tmpSize;
      }
      *(reinterpret_cast<char *>(Sequence.Sequence)+Sequence.Count)=0;
      _D(SysLog("compile: compile newmacro [%s])", Sequence.Sequence));
      return true;
    }
    else
    {
      Free();
      Error=KSE_ALLOC;
      _D(SysLog("compile: error newmacro total alloc (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
      return false;
    }
  }

  DWORD KeyCode, Size;
  char CurKeyText[NM*2];

  // здесь структура сформирована, начинаем разбор последовательности,
  // которая находится в Buffer
  while (1)
  {
    // пропускаем ведущие пробельные символы
    while (IsSpace(*BufPtr))
      BufPtr++;
    if (*BufPtr==0)
      break;

    const char *CurBufPtr=BufPtr;

    // ищем конец очередного названия клавиши
    while (*BufPtr && !IsSpace(*BufPtr))
      BufPtr++;
    int Length=BufPtr-CurBufPtr;

    memcpy(CurKeyText,CurBufPtr,Length);
    CurKeyText[Length]=0;

    // в CurKeyText - название клавиши. Попробуем отыскать ее код...
    if((KeyCode=FSF.FarNameToKey(CurKeyText)) == 0xFFFFFFFF)
    {
      Free();
      Error=KSE_UNKNOWNKEY;
      unknownKey=CurKeyText;
      _D(SysLog("compile: error CurKey (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
      return false;
    }

    Size=1;
    /* $Date
       0: KEY_MACRODATE
       1: Строка, выровненная на 4 байта
          Если строка пуста, то следующий за KEY_MACRODATE DWORD = 0
    */
    switch(KeyCode)
    {
      case KEY_MACRODATE: // СПЕЦ.Обработка $Date (KEY_MACRODATE)
      {
        const char *BufPtr2=BufPtr;
        memset(CurKeyText,0,sizeof(CurKeyText));
        // ищем первую кавычку
        while (*BufPtr && *BufPtr != '"')
          BufPtr++;
        if(*BufPtr)
        {
            ++BufPtr;
          // ищем конечную кавычку
          char *PtrCurKeyText=CurKeyText;
          while (*BufPtr)
          {
            if(*BufPtr == '\\' && BufPtr[1] == '"')
            {
              *PtrCurKeyText++='\\';
              *PtrCurKeyText++='"';
              BufPtr+=2;
            }
            else if(*BufPtr == '"')
            {
              *PtrCurKeyText=0;
              BufPtr++;
              break;
            }
            else
              *PtrCurKeyText++=*BufPtr++;
          }
          if(*BufPtr)
            BufPtr++;
        }
        else
          BufPtr=BufPtr2;
        Length=strlen(CurKeyText)+1;
        // строка должна быть выровнена на 4
        Size+=Length/sizeof(DWORD);
        if(Length==1 || (Length%sizeof(DWORD)) != 0) // дополнение до sizeof(DWORD) нулями.
          Size++;
        break;
      }
    }

    // код найден, добавим этот код в буфер последовательности.
    if (KeyCode!=0xFFFFFFFF)
    {
      Sequence.Sequence=static_cast<DWORD *>(realloc(Sequence.Sequence,sizeof(*Sequence.Sequence)*(Sequence.Count+Size)));
      if (Sequence.Sequence==NULL)
      {
        Free();
        Error=KSE_ALLOC;
        _D(SysLog("compile: error alloc (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
        return false;
      }
      Sequence.Sequence[Sequence.Count]=KeyCode;

      if(KeyCode == KEY_MACRODATE) // СПЕЦ.Обработка $Date (KEY_MACRODATE)
      {
        memcpy(&Sequence.Sequence[Sequence.Count+1],CurKeyText,(Size-1)*sizeof(DWORD));
      }

      Sequence.Count+=Size;
    }
  }
  if(total>1)
  {
     Sequence.Sequence=static_cast<DWORD *>(realloc(Sequence.Sequence,sizeof(*Sequence.Sequence)*(Sequence.Count*total)));
     if(Sequence.Sequence)
     {
       for(DWORD I=1;I<total;++I)
       {
         memcpy(&Sequence.Sequence[Sequence.Count*I],Sequence.Sequence,
           sizeof(*Sequence.Sequence)*Sequence.Count);
       }
       Sequence.Count*=total;
     }
     else
     {
       Free();
       Error=KSE_ALLOC;
       _D(SysLog("compile: error total alloc (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
       return false;
     }
  }
  _D(SysLog("compile: success (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
  return true;
}

#endif // __KeySequenceStorage_cpp
