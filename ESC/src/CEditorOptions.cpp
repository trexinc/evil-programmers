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
extern int ModuleNumber;
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
            ESPar.Param.iParam=Data.TabSize;
            RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          }
          break;
     case ESPT_EXPANDTABS:
          if((Data.Options&E_ExpandTabs_On) ||
             (Data.Options&E_ExpandTabs_Off)
            )
          {
            ESPar.Type=ESPT_EXPANDTABS;
            ESPar.Param.iParam=(Data.Options&E_ExpandTabs_On)?TRUE:FALSE;
            RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          }
          break;
     case ESPT_AUTOINDENT:
          if((Data.Options&E_AutoIndent_On) ||
             (Data.Options&E_AutoIndent_Off))
          {
            ESPar.Type=ESPT_AUTOINDENT;
            ESPar.Param.iParam=(Data.Options&E_AutoIndent_On)?TRUE:FALSE;
            RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          }
          break;
     case ESPT_CURSORBEYONDEOL:
          if((Data.Options&E_CursorBeyondEOL_On) ||
             (Data.Options&E_CursorBeyondEOL_Off)
            )
          {
            ESPar.Type=ESPT_CURSORBEYONDEOL;
            ESPar.Param.iParam=(Data.Options&E_CursorBeyondEOL_On)?TRUE:FALSE;
            RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          }
          break;
     case ESPT_CHARCODEBASE:
          if((Data.Options&E_CharCodeBase_Dec) ||
             (Data.Options&E_CharCodeBase_Oct) ||
             (Data.Options&E_CharCodeBase_Hex)
            )
          {
            ESPar.Type=ESPT_CHARCODEBASE;
            ESPar.Param.iParam=(Data.Options&E_CharCodeBase_Oct)?0:
                          ((Data.Options&E_CharCodeBase_Dec)?1:2);
            RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          }
          break;
     case ESPT_SAVEFILEPOSITION:
          if((Data.Options&E_SaveFilePos_On) ||
             (Data.Options&E_SaveFilePos_Off)
            )
          {
             ESPar.Type=ESPT_SAVEFILEPOSITION;
             ESPar.Param.iParam=(Data.Options&E_SaveFilePos_On)?TRUE:FALSE;
             EditorInfo EI;
             EditorControl(ECTL_GETINFO, &EI);
             if(ESPar.Param.iParam && static_cast<DWORD>(EI.TotalLines)<Data.MinLinesNum)
                ESPar.Param.iParam=FALSE;
             EditorControl(ECTL_SETPARAM, &ESPar);
             _D(SysLog("_SaveFilePos: %s, TotalLines=%d, MinLinesNum=%d", ESPar.Param.iParam?"on":"off", EI.TotalLines, Data.MinLinesNum));
          }
          _D(else SysLog("_SaveFilePos: ignore"));
          RetCode=TRUE;
          break;
     case ESPT_LOCKMODE:
          if((Data.Options&E_LockMode_On) || (Data.Options&E_LockMode_Off))
          {
            ESPar.Type=ESPT_LOCKMODE;
            ESPar.Param.iParam=(Data.Options&E_LockMode_On)?TRUE:FALSE;
            RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          }
          break;
     case ESPT_SETWORDDIV:
          ESPar.Type=ESPT_SETWORDDIV;
          static char WordDiv[256];
          if(Data.Options2&E_WordSym_On)
          {
            MakeWordDiv(Data.Options2&E_AlphaNum_On,Data.AdditionalLetters.str,WordDiv);
            ESPar.Param.cParam=WordDiv;
            _D(SysLog("ESPT_SETWORDDIV: ON [%s]", WordDiv));
          }
          else
          {
            _D(SysLog("ESPT_SETWORDDIV: OFF"));
            ESPar.Param.cParam=NULL;
          }
          RetCode=EditorControl(ECTL_SETPARAM, &ESPar);
          _D(SysLog("ESPT_SETWORDDIV: retcode=%d",RetCode));
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
}

void CEditorOptions::MakeWordDiv(bool alphanum, const char *additional, char *dest)
{
  FARSTDLOCALISALPHA alphafunc=alphanum?FSF.LIsAlpha:FSF.LIsAlphanum;
  const char *other=additional?additional:"";
  for(unsigned int i=1; i<256; ++i)
  {
    if(!alphafunc(i) && NULL==strchr(other, i))
    {
      *dest=i;
      ++dest;
    }
  }
  *dest=0;
}

#endif // __CEditorOptions_cpp
