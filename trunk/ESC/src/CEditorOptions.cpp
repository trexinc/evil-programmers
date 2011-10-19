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
#ifndef __CEditorOptions_cpp
#define __CEditorOptions_cpp

#include "myrtl.hpp"
#include "plugin.hpp"
#include "CEditorOptions.hpp"
#include "syslog.hpp"

extern PluginStartupInfo Info;
extern FARAPIMESSAGE FarMessage;
extern FARAPIEDITORCONTROL EditorControl;
extern struct FarStandardFunctions FSF;
extern const int maxTabSize;

CEditorOptions::CEditorOptions(const NODEDATA &Data)
{
  CEditorOptions::Data=Data;
  memset(&ESPar, 0, sizeof(ESPar));
}

CEditorOptions::~CEditorOptions()
{
}

int CEditorOptions::ApplyOption(EDITOR_SETPARAMETER_TYPES type)
{
   int RetCode=0;
   switch(type)
   {
     case ESPT_TABSIZE:
          if(Data.TabSize>0 && Data.TabSize<maxTabSize)
          {
            ESPar.Type=ESPT_TABSIZE;
            ESPar.iParam=Data.TabSize;
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     case ESPT_EXPANDTABS:
          if(Data.Options&(E_ExpandTabs_On|E_ExpandTabs_Off|E_ExpandTabs_OnlyNew))
          {
            ESPar.Type=ESPT_EXPANDTABS;
            if(Data.Options&E_ExpandTabs_On)
              ESPar.iParam=EXPAND_ALLTABS;
            else if(Data.Options&E_ExpandTabs_OnlyNew)
              ESPar.iParam=EXPAND_NEWTABS;
            else
              ESPar.iParam=EXPAND_NOTABS;
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     case ESPT_AUTOINDENT:
          if((Data.Options&E_AutoIndent_On) ||
             (Data.Options&E_AutoIndent_Off))
          {
            ESPar.Type=ESPT_AUTOINDENT;
            ESPar.iParam=(Data.Options&E_AutoIndent_On)?TRUE:FALSE;
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     case ESPT_CURSORBEYONDEOL:
          if((Data.Options&E_CursorBeyondEOL_On) ||
             (Data.Options&E_CursorBeyondEOL_Off)
            )
          {
            ESPar.Type=ESPT_CURSORBEYONDEOL;
            ESPar.iParam=(Data.Options&E_CursorBeyondEOL_On)?TRUE:FALSE;
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     case ESPT_CHARCODEBASE:
          if((Data.Options&E_CharCodeBase_Dec) ||
             (Data.Options&E_CharCodeBase_Oct) ||
             (Data.Options&E_CharCodeBase_Hex)
            )
          {
            ESPar.Type=ESPT_CHARCODEBASE;
            ESPar.iParam=(Data.Options&E_CharCodeBase_Oct)?0:
                          ((Data.Options&E_CharCodeBase_Dec)?1:2);
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     case ESPT_SAVEFILEPOSITION:
          if((Data.Options&E_SaveFilePos_On) ||
             (Data.Options&E_SaveFilePos_Off)
            )
          {
             ESPar.Type=ESPT_SAVEFILEPOSITION;
             ESPar.iParam=(Data.Options&E_SaveFilePos_On)?TRUE:FALSE;
             EditorInfo EI;
             EditorControl(-1, ECTL_GETINFO, 0, &EI);
             if(ESPar.iParam && static_cast<DWORD>(EI.TotalLines)<Data.MinLinesNum)
                ESPar.iParam=FALSE;
             EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
             _D(SysLog(L"_SaveFilePos: %s, TotalLines=%d, MinLinesNum=%d", ESPar.iParam?L"on":L"off", EI.TotalLines, Data.MinLinesNum));
          }
          _D(else SysLog(L"_SaveFilePos: ignore"));
          RetCode=TRUE;
          break;
     case ESPT_LOCKMODE:
          if((Data.Options&E_LockMode_On) || (Data.Options&E_LockMode_Off))
          {
            ESPar.Type=ESPT_LOCKMODE;
            ESPar.iParam=(Data.Options&E_LockMode_On)?TRUE:FALSE;
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     case ESPT_SETWORDDIV:
          ESPar.Type=ESPT_SETWORDDIV;
          static wchar_t WordDiv[256];
          if(Data.Options2&E_WordSym_On)
          {
            MakeWordDiv(Data.Options2&E_AlphaNum_On,Data.AdditionalLetters.str,WordDiv);
            ESPar.wszParam=WordDiv;
            _D(SysLog(L"ESPT_SETWORDDIV: ON [%s]", WordDiv));
          }
          else
          {
            _D(SysLog(L"ESPT_SETWORDDIV: OFF"));
            ESPar.wszParam=NULL;
          }
          RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          _D(SysLog(L"ESPT_SETWORDDIV: retcode=%d",RetCode));
          break;
     case ESPT_SHOWWHITESPACE:
          if((Data.Options2&E_Show_White_Space_On) || (Data.Options2&E_Show_White_Space_Off))
          {
            ESPar.Type=ESPT_SHOWWHITESPACE;
            ESPar.iParam=(Data.Options2&E_Show_White_Space_On)?TRUE:FALSE;
            RetCode=EditorControl(-1, ECTL_SETPARAM, 0, &ESPar);
          }
          break;
     default:
          break;
  }

  return RetCode;
}

void CEditorOptions::ApplyAllOptions()
{
  ApplyOption(ESPT_TABSIZE);
  ApplyOption(ESPT_EXPANDTABS);
  ApplyOption(ESPT_AUTOINDENT);
  ApplyOption(ESPT_CURSORBEYONDEOL);
  ApplyOption(ESPT_CHARCODEBASE);
  ApplyOption(ESPT_SAVEFILEPOSITION);
  ApplyOption(ESPT_LOCKMODE);
  ApplyOption(ESPT_SETWORDDIV);
  ApplyOption(ESPT_SHOWWHITESPACE);
}

//FIXME: надо понять что тут происходит и правильно для юникода сделать
void CEditorOptions::MakeWordDiv(bool alphanum, const wchar_t *additional, wchar_t *dest)
{
  FARSTDLOCALISALPHA alphafunc=alphanum?FSF.LIsAlpha:FSF.LIsAlphanum;
  const wchar_t *other=additional?additional:L"";
  for(unsigned int i=1; i<256; ++i)
  {
    if(!alphafunc(i) && NULL==wstrchr(other, i)
       && i!=0x20) // если убрать 0x20, т.е. разрешить пробелу быть символом-
                   // разделителем слов, то стандартные CtrlT, CtrlBS и CtrlDel
                   // не будут удалять сразу все пробелы, а только по одному
    {
      *dest=i;
      ++dest;
    }
  }
  *dest=0;
}

#endif // __CEditorOptions_cpp
