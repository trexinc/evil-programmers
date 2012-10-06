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
#ifndef __mix_cpp
#define __mix_cpp

#include <CRT/crt.hpp>
#include "plugin.hpp"
#include "mix.hpp"
#include "XMLStrings.hpp"
#include "options.hpp"
#include "xmlite.hpp"
#include "lang.hpp"
#include "strcon.hpp"
#include "TArray.cpp"
#include "kserror.hpp"
#include "guid.h"

extern TArray<NODEDATA> *NodeData;
extern CRedBlackTree<ESCFileInfo> *FileInfoTree;
extern PluginStartupInfo Info;
extern FarStandardFunctions FSF;
extern wchar_t XMLFilePath[MAX_PATH];
extern BOOL XMLLoadedOK, IsOldFar;
extern XMLStrings XMLStr;
extern struct OPTIONS Opt;
extern FARAPIEDITORCONTROL EditorControl;
extern FARAPIMESSAGE FarMessage;
extern FARSTDTRUNCPATHSTR TruncPathStr;
extern EditorSetPosition esp;
extern EditorGetString egs;
extern EditorSetString ess;
extern EditorInfo ei;
extern strcon *nlsStopChars;

extern const int maxTabSize=512;

void InitNLS(const EditorInfo &ei, NODEDATA &nodedata)
{
//   _D(SysLog(L"InitNLS: AnsiMode=%d/%d, TableNum=%d/%d", ei.AnsiMode,nodedata.AnsiMode,ei.TableNum,nodedata.TableNum));
   nlsSpace=0x20;
   nlsTab=0x09;
   nlsQuoteSym=L'>';
   nlsMinus=L'-';
   if(nlsStopChars)
     *nlsStopChars=nodedata.StopChar;
/*
   BOOL ReSet=FALSE;
   if(ei.AnsiMode)
   {
     ReSet=TRUE;
     if(ei.AnsiMode!=nodedata.AnsiMode)
     {
       _D(SysLog(L"InitNLS: rebuild ansi table"));
       wchar_t toUpper[2], toLower[2], decode[2], encode[2];
       toUpper[1] = toLower[1] = decode[1] = encode[1] = 0;
       for (unsigned int m = 0; m < 256; ++m)
          {
            *toUpper=*toLower=*decode=*encode=m;
            CharToOem(decode, decode);
            OemToChar(encode, encode);
            if(IsCharAlpha(m))
            {
              CharUpper(toUpper);
              CharLower(toLower);
            }
            CharSet.EncodeTable[m] = *encode;
            CharSet.DecodeTable[m] = *decode;
            CharSet.UpperTable[m] = *toUpper;
            CharSet.LowerTable[m] = *toLower;
          }
     }
   }

   if(!ReSet && ei.TableNum!=-1)
   {
     ReSet=TRUE;
     if(ei.TableNum!=nodedata.TableNum)
     {
       _D(SysLog(L"InitNLS: rebuild FAR table"));
       ReSet=(-1!=CharTable(ei.TableNum,(wchar_t*)&CharSet,sizeof(CharSet)));
     }
   }

   if(ReSet)
   {
     nlsSpace=CharSet.EncodeTable[0x20];
     nlsTab=CharSet.EncodeTable[0x09];
     nlsQuoteSym=CharSet.EncodeTable['>'];
     nlsMinus=CharSet.EncodeTable['-'];
     if(nlsStopChars && nlsStopChars->str)
       for(int i=0; nlsStopChars->str[i]; ++i)
         nlsStopChars->str[i]=CharSet.EncodeTable[static_cast<BYTE>
           (nlsStopChars->str[i])];
   }

   nodedata.AnsiMode=ei.AnsiMode;
   nodedata.TableNum=ei.TableNum;
*/
}

int FARPostMacro(const KeySequence *KS)
{
  MacroSendMacroText KeyMacro={sizeof(MacroSendMacroText),KS->Flags,{0},KS->Sequence};
  _D(int res=Info.MacroControl(0,MCTL_SENDSTRING,MSSC_POST,&KeyMacro));
  _D(SysLog(L"FARPostMacro: [%s] Flags=%p, ExitCode=%d",
      KeyMacro.SequenceText, KS->Flags, res));
  _D(return res);
  return (int)Info.MacroControl(0,MCTL_SENDSTRING,MSSC_POST,&KeyMacro);
}

BOOL EditorPostMacro(CUserMacros &macros,UserMacroID &id,const EditorInfo &ei, BOOL &Stop)
{
  const KeySequence *macro;
  if(ei.BlockType==BTYPE_NONE)
  {
    id.SelectionMode=KSSF_SELECTION_OFF;
    macro=macros.GetMacro(id, Stop);
  }
  else
  {
    if(ei.BlockType==BTYPE_STREAM)
    {
      id.SelectionMode=KSSF_SELECTION_ON|KSSF_SELECTION_STREAM;
      macro=macros.GetMacro(id, Stop);
    }
    else
    {
      id.SelectionMode=KSSF_SELECTION_ON|KSSF_SELECTION_COLUMN;
      macro=macros.GetMacro(id, Stop);
    }
    if(!macro)
    {
      id.SelectionMode=KSSF_SELECTION_ON;
      macro=macros.GetMacro(id, Stop);
    }
  }
  if(!macro)
  {
    id.SelectionMode=KSSF_SELECTION_ON|KSSF_SELECTION_OFF;
    macro=macros.GetMacro(id, Stop);
  }
  return (macro && FARPostMacro(macro));
}

// returns the length of initial string part which is a quote or 0 otherwize
// CharSet must be initialized already!
int IsQuote(const wchar_t* pszStr, size_t nLength)
{
  size_t i=0;
  while( i<nLength && i<4 )
  {
    if( pszStr[i++]==nlsQuoteSym )
    {
      while(i<nLength && (pszStr[i]==nlsQuoteSym || IsCSpaceOrTab(pszStr[i])))
        i++;
      return (int)i;
    }
  }
  return 0;
}

// CharSet must be initialized already!
int IsSameQuote(const wchar_t* pszQuote1, size_t nLen1, const wchar_t*pszQuote2,
                size_t nLen2)
{
  while( nLen1 )
    if( nlsSpace==pszQuote1[nLen1-1] ) nLen1--;
    else break;
  while( nLen2 )
    if( nlsSpace==pszQuote2[nLen2-1] ) nLen2--;
    else break;
  return nLen1==nLen2 && !memcmp(pszQuote1,pszQuote2,nLen1*sizeof(wchar_t));
}

BOOL FileExists(const wchar_t *Name)
{
  // Если GetFileAttributes вернул ошибку, но она ERROR_ACCESS_DENIED,
  // то файл все-таки существует
  return GetFileAttributes(Name)!=INVALID_FILE_ATTRIBUTES ||
         GetLastError()==ERROR_ACCESS_DENIED;
}

BOOL CheckForEsc(void)
{
  BOOL EC = FALSE;
  INPUT_RECORD rec;
  static HANDLE hConInp = GetStdHandle(STD_INPUT_HANDLE);
  DWORD ReadCount;

  for(;;)
    {
      PeekConsoleInput(hConInp, &rec, 1, &ReadCount);
      if (ReadCount == 0)
        break;
      ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
      if (rec.EventType == KEY_EVENT)
        if (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
            rec.Event.KeyEvent.bKeyDown)
          EC = TRUE;
    }
  return (EC);
}

DWORD PackEOL(const wchar_t *eolstr)
{
  DWORD Res=0;
  if(eolstr)
  {
    DWORD i=0, Mul=1;
    while(eolstr[i] && i<4)
    {
      Res+=static_cast<BYTE>(eolstr[i])*Mul;
      Mul*=256;
      ++i;
    }
    _D(SysLog(L"PackEOL: eolstr[%s], Res=%u", eolstr, Res));
  }
  return Res;
}

BOOL UnpackEOL(DWORD EOL, wchar_t *Dest)
{
   if(!Dest) return FALSE;
   *Dest=EOL & 0xFF;
   Dest[1]=(wchar_t)((EOL & 0xFF00)/256);
   Dest[2]=(wchar_t)((EOL & 0xFF0000)/65536);
   Dest[3]=(wchar_t)((EOL & 0xFF000000)/16777216);
   Dest[4]=0;
   _D(SysLog(L"UnpackEOL: EOL=%u, Dest=[%s]", EOL, Dest));
   return TRUE;
}

//возвращает строку из LNG в соответствие с языковыми настройками
const wchar_t *GetMsg(int MsgId) {return (Info.GetMsg(&MainGuid, MsgId));}

int FarAllInOneMessage(const wchar_t *Message, unsigned int Flags)
{
  return (int)FarMessage(&MainGuid, &AllInOneMessageGuid, Flags|FMSG_ALLINONE, NULL,
                      (const wchar_t * const *)Message, 1, 1);
}

void MessageFileError(const wchar_t *msg, const wchar_t *name)
{
  const wchar_t *MsgItems[4];
  strcon fn(name);
  TruncPathStr(fn.str,62);
  MsgItems[0] = GetMsg(MWarning);
  MsgItems[1] = msg;
  MsgItems[2] = fn.str;
  MsgItems[3] = GetMsg(MOk);
  FarMessage(&MainGuid, &FileErrorMessageGuid, FMSG_WARNING, NULL, MsgItems, 4, 1);
}

BOOL IsFilesChanged()
{
  if(IsOldFar || !Opt.ReloadSettingsAutomatically) return FALSE;

  WIN32_FIND_DATA wfd;

  FileInfoTree->First();
  ESCFileInfo *fi;
  HANDLE hXml;
  BOOL rc;
  if(FileInfoTree->IsEmpty())
  {
    rc=TRUE;
  }
  else
  {
    rc=FALSE;
    while(NULL!=(fi=FileInfoTree->GetNext()))
    {
      if(!fi->Name.str)
        break;
      hXml=FindFirstFile(fi->Name.str, &wfd);
      if(hXml==INVALID_HANDLE_VALUE)
      {
        rc=TRUE;
        break;
      }
      FindClose(hXml);

      if(CompareFileTime(&wfd.ftLastWriteTime, &fi->Time)!=0 ||
         wfd.nFileSizeLow!=fi->SizeLow || wfd.nFileSizeHigh!=fi->SizeHigh)
      {
        rc=TRUE;
        break;
      }
    }
  }
  return rc;
}

void SetMarginOpt(CXMLNode &n, NODEDATA &node,
                  const int &Inherit)
{
  const wchar_t *Attr=n.Attr(XMLStr.AutoWrap);
  if(*Attr)
  {
    if(Inherit)
      node.Options&=~E_AutoWrap_On;

    if(!FSF.LStricmp(Attr, XMLStr.On))
      node.Options|=E_AutoWrap_On;
  }
  else if(!Inherit)
    node.Options|=E_AutoWrap_On;

  Attr=n.Attr(XMLStr.Wrap);
  if(*Attr)
  {
    if(Inherit)
    {
      node.WrapPos=0;
      node.Options2&=~E_Wrap_Percent;
    }

    int len=lstrlen(Attr)-1, newAttr;
    if(Attr[len]==L'%')
    {
      newAttr=0;
      strcon tmp(Attr);
      if(tmp.str)
      {
        tmp.str[len]=0;
        newAttr=FSF.atoi(tmp.str);
        node.Options2|=E_Wrap_Percent;
      }
    }
    else
      newAttr=FSF.atoi(Attr);

     if(newAttr>0 && newAttr<maxTabSize)
       node.WrapPos=newAttr;
  }

  if(node.WrapPos>0 && node.WrapPos<maxTabSize)
  {
    Attr=n.Attr(XMLStr.Justify);
    if(*Attr)
    {
      if(Inherit)
        node.Options2&=~E_Wrap_Justify;

      if(!FSF.LStricmp(Attr, XMLStr.On))
        node.Options2|=E_Wrap_Justify;
    }
  }
  else
    node.Options2&=~E_Wrap_Justify;
}

void SetOption(const wchar_t *Attr,DWORD &Options,DWORD Value,
               const int &Inherit, BOOL OnByDefault)
{
  if(*Attr)
  {
    if(Inherit)
      Options&=~Value;

    if(!FSF.LStricmp(Attr, XMLStr.On))
      Options|=Value;
  }
  else if(!Inherit && OnByDefault)
    Options|=Value;
}

void SetOption(const wchar_t *Attr,DWORD &Options,
               E_OPTIONS ValueOn, E_OPTIONS ValueOff,
               const int &Inherit)
{
  if(*Attr)
  {
    if(Inherit)
    {
      Options&=~ValueOn;
      Options&=~ValueOff;
    }

    if(!FSF.LStricmp(Attr, XMLStr.On))
      Options|=ValueOn;
    else if(!FSF.LStricmp(Attr, XMLStr.Off))
      Options|=ValueOff;
  }
}

void SetNumOpt(const wchar_t *Attr, DWORD &Num, DWORD Default, const int &Inherit)
{
  if(*Attr)
  {
    Num=0;
    int newAttr=FSF.atoi(Attr);
    if(newAttr>0) Num=newAttr;
  }
  else if(!Inherit)
    Num=Default;
}

void SetStrOpt(const wchar_t *Attr, strcon &DestStr, const strcon &Default, const int &Inherit)
{
  if(*Attr)
    DestStr=Attr;
  else if(!Inherit)
    DestStr=Default;
}

void SetAddSymbolOpt(const wchar_t *Attr, CXMLNode &n, NODEDATA &node,
                     const int &Inherit, bool NewConfig)
{
  SetOption(n.Attr(const_cast<wchar_t*>(Attr)),node.Options,E_AddSymbol_On,Inherit,FALSE);
  Attr=n.Attr(NewConfig?XMLStr.Start:XMLStr.AddSym_S);
  if(*Attr)
    node.AddSym_S=Attr;
  Attr=n.Attr(NewConfig?XMLStr.End:XMLStr.AddSym_E);
  if(*Attr)
    node.AddSym_E=Attr;
  Attr=n.Attr(NewConfig?XMLStr.Start_B:XMLStr.AddSym_S_B);
  if(*Attr)
    node.AddSym_S_B=Attr;
  Attr=n.Attr(NewConfig?XMLStr.End_B:XMLStr.AddSym_E_B);
  if(*Attr)
    node.AddSym_E_B=Attr;
}

void SetTabOpt(CXMLNode &n, NODEDATA &node, const int &Inherit)
{
  const wchar_t *Attr=n.Attr(XMLStr.TabSize);
  if(*Attr)
  {
    node.TabSize=0;
    int newAttr=FSF.atoi(Attr);
    if(newAttr>0 && newAttr<maxTabSize)
      node.TabSize=newAttr;
  }

  Attr=n.Attr(XMLStr.ExpandTabs);
  if(*Attr)
  {
    if(Inherit)
    {
      node.Options&=~E_ExpandTabs_On;
      node.Options&=~E_ExpandTabs_Off;
      node.Options&=~E_ExpandTabs_OnlyNew;
    }

    if(!FSF.LStricmp(Attr, XMLStr.On))
      node.Options|=E_ExpandTabs_On;
    else if(!FSF.LStricmp(Attr, XMLStr.Off))
      node.Options|=E_ExpandTabs_Off;
    else if(!FSF.LStricmp(Attr, XMLStr.OnlyNew))
      node.Options|=E_ExpandTabs_OnlyNew;

    _D(SysLog(L"XML EXPANDTABS: Name:[%s] mask:[%s] Attr:[%s] Options:[%x]", node.Name.str, node.mask.str, Attr, node.Options));
  }
}

bool InsertMacro(CXMLNode &n,CUserMacros &Macro,
  const DWORD VirtualKeyCode,const DWORD ControlKeyState, const DWORD ButtonState, const DWORD ControlState,
  DWORD &Flags, bool silent,BOOL Stop,int &Error,strcon &unknownKey)
{
  const wchar_t *Attr;
  if(!Flags)
  {
    Attr=n.Attr(XMLStr.Selection);
    if(!FSF.LStricmp(Attr,XMLStr.On))
      Flags|=KSSF_SELECTION_ON;
    else if(!FSF.LStricmp(Attr,XMLStr.Off))
      Flags|=KSSF_SELECTION_OFF;
    else if(!FSF.LStricmp(Attr,XMLStr.Stream))
      Flags|=KSSF_SELECTION_ON|KSSF_SELECTION_STREAM;
    else if(!FSF.LStricmp(Attr,XMLStr.Column))
      Flags|=KSSF_SELECTION_ON|KSSF_SELECTION_COLUMN;
    else
      Flags|=KSSF_SELECTION_ON|KSSF_SELECTION_OFF;
  }
  Attr=n.Attr(XMLStr.Sequence);
  UserMacroID id(VirtualKeyCode,ControlKeyState,Flags,ButtonState,ControlState);
  return Macro.InsertMacro(id,Attr,silent,Stop!=FALSE,Error,unknownKey);
}

bool AddMacro(CXMLNode &n,NODEDATA &node,int &Error,strcon &unknownKey)
{
  bool res=true, silent=true, Stop=false;
  DWORD ButtonState=0, Flags=0;
  const wchar_t *Attr;

  if(!FSF.LStricmp(n.Attr(XMLStr.Stop),XMLStr.On))
    Stop=true;

  Attr=n.Attr(XMLStr.Silent);
  if(*Attr && 0!=FSF.LStricmp(Attr,XMLStr.On))
    silent=false;

  // проверим автостартующие макросы
  TList<KeySequenceStorage> *Macros=NULL;
  Attr=n.Attr(XMLStr.Auto);
  if(!FSF.LStricmp(Attr,XMLStr.onCreate))
  {
    _D(SysLog(L"AddMacro: OnCreateMacros"));
    Macros=&node.OnCreateMacros;
  }
  else if(!FSF.LStricmp(Attr,XMLStr.onLoad))
  {
    _D(SysLog(L"AddMacro: OnLoadMacros"));
    Macros=&node.OnLoadMacros;
  }
  if(Macros)
  {
    KeySequenceStorage tmpMacro;
    if(tmpMacro.compile(n.Attr(XMLStr.Sequence), silent, 1, Stop, Error, unknownKey))
    {
      Macros->insert(tmpMacro);
      _D(SysLog(L"AddMacro: 'on'-macros: inserted"));
    }
    else
    {
      Error=KSE_UNKNOWNKEY;
      res=false;
      _D(SysLog(L"AddMacro: 'on'-macros: NOT inserted (unknownKey='%s')", unknownKey.str?unknownKey.str:L"(null)"));
    }
  }

  if(res)
  {
    if(!FSF.LStricmp(n.Attr(XMLStr.LClick),XMLStr.On))
      ButtonState|=EBS_LClick_On;
    if(!FSF.LStricmp(n.Attr(XMLStr.RClick),XMLStr.On))
      ButtonState|=EBS_RClick_On;
    if(!FSF.LStricmp(n.Attr(XMLStr.MClick),XMLStr.On))
      ButtonState|=EBS_MClick_On;

    if(0!=ButtonState) // есть мышиный макрос
    {
      _D(SysLog(L"AddMacro: mouse macro exists"));
      DWORD ControlState=0;
      if(!FSF.LStricmp(n.Attr(XMLStr.RCtrl),XMLStr.On))
        ControlState|=ECS_RCtrl_On;
      if(!FSF.LStricmp(n.Attr(XMLStr.LCtrl),XMLStr.On))
        ControlState|=ECS_LCtrl_On;
      if(!FSF.LStricmp(n.Attr(XMLStr.RAlt),XMLStr.On))
        ControlState|=ECS_RAlt_On;
      if(!FSF.LStricmp(n.Attr(XMLStr.LAlt),XMLStr.On))
        ControlState|=ECS_LAlt_On;
      if(!FSF.LStricmp(n.Attr(XMLStr.Shift),XMLStr.On))
        ControlState|=ECS_Shift_On;
      res=InsertMacro(n,node.MouseMacros,0,0,ButtonState,ControlState,Flags,
           silent,Stop,Error,unknownKey);
    }
  }

  if(res && *(Attr=n.Attr(XMLStr.Key)))
  {
    _D(SysLog(L"AddMacro: Key=[%s]", Attr));
    INPUT_RECORD Rec;
    if(!FSF.FarNameToInputRecord(Attr,&Rec))
    {
      Error=KSE_UNKNOWNKEY;
      unknownKey=Attr;
      res=false;
    }
    else
      res=InsertMacro(n,node.KeyMacros,Rec.Event.KeyEvent.wVirtualKeyCode,Rec.Event.KeyEvent.dwControlKeyState,0,0,Flags,silent,Stop,
          Error,unknownKey);
  }
  return res;
}

void SetCharCodeBaseOpt(const wchar_t *Attr, DWORD &Options, const int &Inherit)
{
  if(*Attr)
  {
    if(Inherit)
    {
      Options&=~E_CharCodeBase_Oct;
      Options&=~E_CharCodeBase_Dec;
      Options&=~E_CharCodeBase_Hex;
    }

    if(!FSF.LStricmp(Attr, XMLStr.Oct))
      Options|=E_CharCodeBase_Oct;
    else if(!FSF.LStricmp(Attr, XMLStr.Dec))
      Options|=E_CharCodeBase_Dec;
    else if(!FSF.LStricmp(Attr, XMLStr.Hex))
      Options|=E_CharCodeBase_Hex;
  }
}

void SetEOLOpt(const wchar_t *Attr, DWORD &EOL)
{
  if(*Attr)
    EOL=PackEOL(Attr);
}

void ShowCfgError(const wchar_t *FileName, const wchar_t *Type,
                  const wchar_t *Param, const wchar_t *ErrorMsg,
                  const wchar_t *Comment)
{
  const wchar_t *MsgItems[7];
  strcon fn(FileName);
  TruncPathStr(fn.str,62);
  strcon type(GetMsg(MType)), param(GetMsg(MParam));
  type.AddStr(Type);
  param.AddStr(Param);
  MsgItems[0] = GetMsg(MProblemInTheFile);
  MsgItems[1] = fn.str;
  MsgItems[2] = type.str;
  MsgItems[3] = param.str;
  MsgItems[4] = ErrorMsg;
  int i=5;
  if(Comment)
    MsgItems[i++] = Comment;
  MsgItems[i++] = GetMsg(MOk);
  FarMessage(&MainGuid, &CfgErrorMessageGuid, FMSG_WARNING, NULL, MsgItems, i, 1);
}

/* Обработка конфигурационных файлов
   Возвращается:
   Число > 0 - успех
    0        - файл содержит ошибки
   -1        - ошибка выделения памяти
   -2        - файл не найден
*/
int ParseFile(const wchar_t *filename,CRedBlackTree<ESCFileInfo>&FITree,
              TArray<NODEDATA>&NodeData, const bool test)
{
  wchar_t FileName[MAX_PATH];
  ExpandEnvironmentStrings(filename,FileName,sizeof(FileName));
  _D(SysLog(L"ParseFile: [%s] - start", FileName));
  int rc=0;
  ESCFileInfo fi(const_cast<const wchar_t*>(FileName));
  if (!*FileName || FITree.findNode(fi))
    rc=1;
  else
  {
    WIN32_FIND_DATA wfd;
    HANDLE hFile=FindFirstFile(FileName,&wfd);
    if(hFile!=INVALID_HANDLE_VALUE)
    {
      FindClose(hFile);
      fi.Time=wfd.ftLastWriteTime;
      fi.SizeLow=wfd.nFileSizeLow;
      fi.SizeHigh=wfd.nFileSizeHigh;
      if (!FITree.insertNode(fi))
        rc=-1;
    }
    else
    {
      rc=-2;
      if(test)
        MessageFileError(GetMsg(MConfigFileNotFound), FileName);
    }
  }

  CXMLNode xmlnode;
  if(rc==0)
  {
    if(xmlnode.ParseFile(FileName))
      rc=1;
    else
      MessageFileError(GetMsg(MConfigXMLIsBad), FileName);
  }
  if(rc==1)
  {
    rc=1;
    NODEDATA *nd;
    CXMLNode n, N, macroN, p=xmlnode.GetItem(XMLStr.Settings);
    CXMLNode::Iterator i=NULL, macroI;
    const wchar_t *Attr;
    BOOL Error=FALSE;
    {
      i=NULL;
      int SameType;
      BOOL Inherit;
      while(!Error && NULL!=(i=p.EnumName(XMLStr.Type, i,n)))
      {
        Attr=n.Attr(XMLStr.Include);
        if(*Attr)
        {
          _D(SysLog(L"ParseFile: [%s], include=[%s]", FileName, Attr));
          strcon newName;
          if((Attr[1]==L':' && Attr[2]) || (Attr[0]==L'\\' && Attr[1]==L'\\'))
            newName=Attr;
          else
          {
            const wchar_t *slash=wcsrchr(FileName,L'\\');
            int l=slash?(int)(slash-FileName+1):0;
            newName.setStr(FileName,l);
            newName.AddStr(Attr);
          }
          rc=ParseFile(newName.str,FITree,NodeData,test);
          if(rc==0 || rc==-1) // файл с ошибками или ошибки выделения памяти
            break;
          if(rc==-2 && test)
            break;
          rc=1;
          continue;
        }
        Attr=n.Attr(XMLStr.Name);
        _D(SysLog(L"ParseFile: [%s], type=[%s]", FileName, Attr));
        if(*Attr)
        {
          SameType=-1;
          NODEDATA node;
          node.Name=Attr;
          if(node.Name.str)
          {
           SameType=NodeData.getIndex(node);
           if(SameType!=-1 && NULL!=(nd=NodeData.getItem(SameType)))
           {
             node=*nd;
             Inherit=TRUE;
           }
           else
           {
             SameType=-1;
             Inherit=FALSE;
           }

           Attr=n.Attr(XMLStr.Mask);
           _D(SysLog(L"ParseFile: [%s], Mask=[%s], SameType=%d",
              FileName, Attr, SameType));
           if(*Attr || SameType!=-1)
           {
            if(*Attr)
              node.mask=Attr;
            if(node.mask.str)
            {
              Attr=n.Attr(XMLStr.Inherit);
              if(*Attr)
              {
                DWORD c=0;
                while(NULL!=(nd=NodeData.getItem(c)))
                {
                  if(nd->Name==Attr && c!=static_cast<DWORD>(SameType))
                  {
                    Inherit=TRUE;
                    nd->Inherit(node);
                    break;
                  }
                  ++c;
                }
              }

              if(-1==SameType)
              {
                node.Options&=~E_SkipPath_On;
                Attr=n.Attr(XMLStr.SkipPath);
                if(!*Attr || !FSF.LStricmp(Attr, XMLStr.On))
                  node.Options|=E_SkipPath_On;
              }

              node.Options&=~E_OverLoad_On;
              SetOption(n.Attr(XMLStr.OverLoad),node.Options,E_OverLoad_On,FALSE,FALSE);

              N=n.GetItem(XMLStr.Margin);
              SetMarginOpt(N,node,Inherit);

              N=n.GetItem(XMLStr.Quote);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_ProcessQuote_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.QuoteEOL),node.Options,E_QuoteEOL_On,Inherit,FALSE);

              N=n.GetItem(XMLStr.Tab);
              SetTabOpt(N,node,Inherit);

              N=n.GetItem(XMLStr.SmartKey);
              SetOption(N.Attr(XMLStr.Tab),node.Options,E_SmartTab_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.BackSpace),node.Options,E_SmartBackSpace_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.Home),node.Options,E_SmartHome_On,Inherit,FALSE);
              SetNumOpt(N.Attr(XMLStr.Lines),node.Lines,64,Inherit);
              node.StopChar=N.Attr(XMLStr.StopChar);

              N=n.GetItem(XMLStr.Kill);
              SetOption(N.Attr(XMLStr.KillSpaces),node.Options,E_KillSpaces_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.p_Minuses),node.Options,E_Process_Minuses_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.KillEmptyLines),node.Options,E_KillEmptyLines_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.ProcessKeyEnd),node.Options,E_ProcessKeyEnd_On,Inherit,TRUE);
              SetOption(N.Attr(XMLStr.ForceKillEmptyLines),node.Options,E_ForceKillEL_On,Inherit,TRUE);

              N=n.GetItem(XMLStr.SaveFilePos);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_SaveFilePos_On,E_SaveFilePos_Off,Inherit);
              SetNumOpt(N.Attr(XMLStr.MinLinesNum),node.MinLinesNum,0,Inherit);

              N=n.GetItem(XMLStr.AutoIndent);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_AutoIndent_On,E_AutoIndent_Off,Inherit);

              N=n.GetItem(XMLStr.CursorBEOL);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_CursorBeyondEOL_On,E_CursorBeyondEOL_Off,Inherit);

              N=n.GetItem(XMLStr.CharCodeBase);
              SetCharCodeBaseOpt(N.Attr(XMLStr.Value),node.Options,Inherit);

              N=n.GetItem(XMLStr.eol);
              SetEOLOpt(N.Attr(XMLStr.Value),node.EOL);

              N=n.GetItem(XMLStr.Table);
              SetNumOpt(N.Attr(XMLStr.Value),node.Table,node.Table,Inherit);
              SetStrOpt(N.Attr(XMLStr.Name),node.TableName,L"",Inherit);

              N=n.GetItem(XMLStr.AddSymbol);
              SetAddSymbolOpt(XMLStr.Value,N,node,Inherit,true);

              N=n.GetItem(XMLStr.LockMode);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_LockMode_On,E_LockMode_Off,Inherit);

              N=n.GetItem(XMLStr.LockFile);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_LockFile_On,Inherit,FALSE);

              N=n.GetItem(XMLStr.ShowWhiteSpace);
              SetOption(N.Attr(XMLStr.Value),node.Options2,(E_OPTIONS)E_Show_White_Space_On,(E_OPTIONS)E_Show_White_Space_Off,Inherit);

              N=n.GetItem(XMLStr.Bom);
              SetOption(N.Attr(XMLStr.Value),node.Options2,(E_OPTIONS)E_Bom_On,(E_OPTIONS)E_Bom_Off,Inherit);

              macroI=NULL;
              if(n.EnumName(XMLStr.UserMacro, macroI, N))
                SetOption(N.Attr(XMLStr.Value),node.Options,E_UserMacro_On,Inherit,FALSE);

              if(node.Options&E_UserMacro_On)
              {
                macroI=NULL;
                int macro_err;
                strcon unknownKey, param, comment;
                while(NULL!=(macroI=N.EnumName(XMLStr.Macro, macroI, macroN)))
                {
                  Attr=macroN.Attr(XMLStr.Enable);
                  if(!*Attr || !FSF.LStricmp(Attr, XMLStr.On))
                  {
                    if(!AddMacro(macroN,node,macro_err,unknownKey))
                      {
                        Error=TRUE;
                        if(macro_err==KSE_UNKNOWNKEY)
                        {
                          comment.AddStrings(L"\"",unknownKey.str,L"\"",0);
                          param=XMLStr.UserMacro;
                          if(*(Attr=macroN.Attr(XMLStr.Key)))
                            param.AddStrings(L"/",XMLStr.Macro,L"/key=",Attr,0);
                          ShowCfgError(FileName,node.Name.str,
                            param.str, GetMsg(MMacroUnknownKey),
                            comment.str);
                        }
                        else if(macro_err==KSE_ALLOC)
                          FarAllInOneMessage(GetMsg(MCannotAllocateAMemory),
                            FMSG_WARNING);
                        break;
                      }
                  }
                }
                if(node.KeyMacros.IsEmpty() && node.MouseMacros.IsEmpty())
                  node.Options&=~E_UserMacro_On;
              }

              N=n.GetItem(XMLStr.WordSym);
              SetOption(N.Attr(XMLStr.Value),node.Options2,E_WordSym_On,Inherit,FALSE);
              SetOption(N.Attr(XMLStr.AlphaNum),node.Options2,E_AlphaNum_On,Inherit,FALSE);
              SetStrOpt(N.Attr(XMLStr.Additional),node.AdditionalLetters,L"",Inherit);

              // отследим взаимосвязи
              if(!(node.Options&E_KillSpaces_On))
                node.Options&=~E_ProcessKeyEnd_On;

              if(node.Options&E_ExpandTabs_Off)
              {
                node.Options&=~E_SmartTab_On;
                node.Options&=~E_SmartBackSpace_On;
              }

              Error=Error || (!node.Name.str ||
                     !node.mask.str ||
                     !node.AddSym_S.str ||
                     !node.AddSym_E.str ||
                     !node.AddSym_S_B.str ||
                     !node.AddSym_E_B.str ||
                     !node.StopChar.str ||
                     !node.AdditionalLetters.str ||
                     !node.TableName.str
                    );

              if(!Error && node.Options&E_AddSymbol_On)
              {
                unsigned int l;
                if(node.AddSym_S.getLength()!=node.AddSym_E.getLength())
                {
                  l=Min(node.AddSym_S.getLength(),
                        node.AddSym_E.getLength());
                  node.AddSym_S.setLength(l);
                  node.AddSym_E.setLength(l);
                }
                if(node.AddSym_S_B.getLength()!=node.AddSym_E_B.getLength())
                {
                  l=Min(node.AddSym_S_B.getLength(),
                        node.AddSym_E_B.getLength());
                  node.AddSym_S_B.setLength(l);
                  node.AddSym_E_B.setLength(l);
                }
              }
              // закончили отслеживание взаимосвязей

              //_D(SysLog(L"ReloadSettings: Name=[%s], tabsize=%d wrap=%d Justify=%s",
              //   node.Name, node.TabSize, node.WrapPos, node.Options2&E_Wrap_Justify?L"on":L"off"));
            }
            else
              Error=TRUE;
           }
          }
          else
            Error=TRUE;
          if(!Error && !test)
          {
            nd=(SameType==-1)?NodeData.addItem(node):
               NodeData.setItem(SameType,node);
            if(!nd)
              Error=TRUE;
          }
        }
      }
    }
  }
  _D(SysLog(L"ParseFile: [%s] - end (%d)", FileName, rc));
  return rc;
}

void TestCfgFiles()
{
  _D(SysLog(L"TestCfgFiles: start"));
  CRedBlackTree<ESCFileInfo> FITree(ESCFICompLT, ESCFICompEQ);
  TArray<NODEDATA> ND;
  int rc=ParseFile(XMLFilePath,FITree,ND,true);
  if(rc==-1)
    FarAllInOneMessage(GetMsg(MCannotAllocateAMemory), FMSG_WARNING);
  else if(rc>0)
    FarAllInOneMessage(GetMsg(MConfigIsOK), 0);
  _D(SysLog(L"TestCfgFiles: end = %d", rc));
}

BOOL ReloadSettings(BOOL Force)
{
  if(IsOldFar) return FALSE;
  _D(SysLog(L"ReloadSettings(%d) - start", Force));

  if(Opt.TurnOnPluginModule || Force)
  {
    if(Force || IsFilesChanged())
    {
      FreeMem();
      FileInfoTree->Empty();
      NodeData=new TArray<NODEDATA>;
      if(NodeData && 0<ParseFile(XMLFilePath,*FileInfoTree,*NodeData, false) &&
         NodeData->getSize())
        XMLLoadedOK=TRUE;
      else
        FreeMem();
    }
  }
  else
    FreeMem();

  _D(SysLog(L"ReloadSettings(%d) = %d", Force, XMLLoadedOK));
  return XMLLoadedOK;
}

void InitESPandEGS(struct EditorSetPosition &esp, struct EditorGetString &egs)
{
   memset(&esp, -1, sizeof(esp));
   esp.StructSize=sizeof(EditorSetPosition);
   egs.StructSize=sizeof(EditorGetString);
   egs.StringNumber=-1;
}

intptr_t Size;
bool prc_Minuses;

inline bool IsMinusMinusSpace(const EditorGetString &egs)
{
  return egs.StringLength==3 && *egs.StringText==egs.StringText[1] &&
        nlsMinus==*egs.StringText && nlsSpace==egs.StringText[2];
}

void KillSpaces()
{
   EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
   EditorControl(-1, ECTL_GETSTRING, 0, &egs);
   _D(SysLog(L"ks: StringLength=%d", egs.StringLength));
   if(egs.StringLength>0 && !(prc_Minuses && IsMinusMinusSpace(egs)))
   {
     Size=egs.StringLength-1;
     while(IsCSpaceOrTab(egs.StringText[Size])) --Size;
     ++Size;
     if(Size!=egs.StringLength)
     {
       _D(SysLog(L"ks: 1. esp.CurLine=%d", esp.CurLine));
       ess.StringNumber=-1;
       ess.StringText=const_cast<wchar_t*>(egs.StringText);
       ess.StringEOL =const_cast<wchar_t*>(egs.StringEOL);
       ess.StringLength=Size;
       EditorControl(-1, ECTL_SETSTRING, 0, &ess);
     }
     _D(else SysLog(L"ks: 2. esp.CurLine=%d", esp.CurLine));
   }
}

wchar_t GLOBAL_EOL[16];
void KillSpacesAndChangeEOL()
{
   EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
   EditorControl(-1, ECTL_GETSTRING, 0, &egs);

   if(egs.StringLength>0)
   {
     if(prc_Minuses && IsMinusMinusSpace(egs))
       Size=egs.StringLength;
     else
     {
       Size=egs.StringLength-1;
       while(IsCSpaceOrTab(egs.StringText[Size])) --Size;
       ++Size;
     }
   }
   else
     Size=0;

   if(*egs.StringEOL && 0!=lstrcmp(GLOBAL_EOL,egs.StringEOL))
   {
     _D(SysLog(L"ksceol: 1. esp.CurLine=%d", esp.CurLine));
     ess.StringNumber=-1;
     ess.StringText=const_cast<wchar_t*>(egs.StringText);
     ess.StringEOL=GLOBAL_EOL;
     ess.StringLength=Size;
     EditorControl(-1, ECTL_SETSTRING, 0, &ess);
   }
   else if(Size!=egs.StringLength)
   {
     _D(SysLog(L"ksceol: 2. esp.CurLine=%d", esp.CurLine));
     ess.StringNumber=-1;
     ess.StringText=const_cast<wchar_t*>(egs.StringText);
     ess.StringEOL =const_cast<wchar_t*>(egs.StringEOL);
     ess.StringLength=Size;
     EditorControl(-1, ECTL_SETSTRING, 0, &ess);
   }
   _D(else SysLog(L"ksceol: 3. esp.CurLine=%d", esp.CurLine));
}

void ChangeEOL()
{
   EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
   EditorControl(-1, ECTL_GETSTRING, 0, &egs);
   _D(SysLog(L"ceol: 0. myeol=[%s], original_eol=[%s]", GLOBAL_EOL, egs.StringEOL));
   if(*egs.StringEOL && 0!=lstrcmp(GLOBAL_EOL,egs.StringEOL))
   {
     _D(SysLog(L"ceol: 1. esp.CurLine=%d", esp.CurLine));
     ess.StringNumber=-1;
     ess.StringText=const_cast<wchar_t*>(egs.StringText);
     ess.StringEOL=GLOBAL_EOL;
     ess.StringLength=egs.StringLength;
     EditorControl(-1, ECTL_SETSTRING, 0, &ess);
   }
   _D(else SysLog(L"ceol: 2. esp.CurLine=%d", esp.CurLine));
}

wchar_t *TruncFromRigth(wchar_t *Str, unsigned int maxsize, BOOL AddSpaces)
{
  unsigned int len=lstrlen(Str);
  if (maxsize < 5) maxsize = 5;
  if (len > maxsize) wcscpy(Str+maxsize-3, L"...");
  else if(AddSpaces)
  {
     wmemset(Str+len, 0x20, (maxsize-len));
     Str[maxsize]=0;
  }
  return Str;
}

int CalcWrapPos(const NODEDATA &Data, const EditorInfo &ei)
{
   if(Data.Options2 & E_Wrap_Percent)
   {
      int w=(int)(ei.WindowSizeX*Data.WrapPos/100);
      return (w>511)?511:w;
   }
   else
      return Data.WrapPos;
}

intptr_t GetPrevCoordX(EditorInfo &ei, int Lines, const wchar_t *StopChars)
{
   intptr_t oldX(ei.CurPos), oldY(ei.CurLine), CoordX(-1), LastCoordX(ei.CurPos);
   int ForceRight(TRUE);
   if(oldX>0)
   {
     ei.CurPos=0;
     do
     {
       CoordX=GetNextCoordX(ei, Lines, StopChars);
       if(ForceRight)
       {
         intptr_t oldCoordX=CoordX, LastX=-1;
         if(CoordX!=-1) do
         {
            ei.CurPos=LastX=CoordX;
            CoordX=GetNextCoordX(ei, Lines, StopChars);
         } while(CoordX>0 && CoordX<oldX);
         CoordX=oldCoordX;
         if(LastX>-1 && LastX<oldX)
         {
           LastCoordX=LastX;
           _D(SysLog(L"GetPrevCoordX: LastX=%d", LastX));
           break;
         }
         ei.CurPos=0;
         ForceRight=FALSE;
       }
       if(CoordX!=-1)
       {
         LastCoordX=CoordX;
         --ei.CurLine;
         --Lines;
       }
     } while(CoordX>0 && CoordX<=LastCoordX && CoordX>=oldX);

     ei.CurPos=oldX;
     ei.CurLine=oldY;
     RestorePosition(ei);
     if(LastCoordX>=oldX)
       LastCoordX=CoordX=0;
   }

   if(CoordX<0 || LastCoordX<0)
     LastCoordX=-1;

   _D(SysLog(L"GetPrevCoordX = %d", LastCoordX));
   return LastCoordX;
}

intptr_t GetNextCoordX(const EditorInfo &EI, int Lines, const wchar_t *StopChars)
{
  int ExistNonSpace, LastExistNonSpace;
  intptr_t Ret=-1, tmpX, f;
  _D(esp.CurLine=EI.CurLine-1);
  InitESPandEGS(esp, egs);

  for(esp.CurLine=EI.CurLine-1; Lines && esp.CurLine>-1; --esp.CurLine, --Lines)
    {
       EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
       EditorControl(-1, ECTL_GETSTRING, 0, &egs);
       if(egs.StringLength)
         while(IsCSpaceOrTab(egs.StringText[egs.StringLength-1]))
            --egs.StringLength;

       _D(SysLog(L"GetNextCoordX(%d) egs.StringLength=%d", esp.CurLine, egs.StringLength));
       if(egs.StringLength>0)
       {
          tmpX=EI.CurPos;
          if(tmpX<egs.StringLength)
          {
            _D(SysLog(L"GetNextCoordX(%d): [%s]", esp.CurLine, egs.StringText));
            _D(SysLog(L"GetNextCoordX: StopChars=[%s]", StopChars));
            // проверим StopChars
            if(StopChars)
            {
              for(; tmpX<egs.StringLength &&
                  !IsCSpaceOrTab(egs.StringText[tmpX]); ++tmpX)
                if(NULL!=wcschr(StopChars, egs.StringText[tmpX]))
                {
                  Ret=tmpX+1;
                  while(NULL!=wcschr(StopChars, egs.StringText[Ret]))
                    ++Ret;
                  break;
                }
              if(Ret!=-1) // уже нашли, что искали
                break;
              _D(SysLog(L"GetNextCoordX: StopChars not found"));
            }

            f=tmpX+1; // tmpX указывает на следующий символ
            LastExistNonSpace=ExistNonSpace=FALSE;
            if(!IsCSpaceOrTab(egs.StringText[f-1])) // символ в текущей позиции
            {                                       // не пробел
              if(f<egs.StringLength)     // запомним позицию непробельного
                 LastExistNonSpace=TRUE; // символа
              for(; f<egs.StringLength; ++f) // ищем первый пробел
              {
                if(IsCSpaceOrTab(egs.StringText[f]))
                  break;
              }
            }

            tmpX=f; // теперь tmpX гарантированно указывает на пробел
            if(f<egs.StringLength)
            {
              intptr_t lastX=tmpX; // запомним tmpX
              for(f=tmpX; f<egs.StringLength; ++f)// проверим следующие символы
              {
                if(!IsCSpaceOrTab(egs.StringText[f])) // нашли "непробел" -
                {                                     // можно остановиться
                  ExistNonSpace=TRUE;
                  break;
                }
              }
              if(!ExistNonSpace) // не было найдено непробельного символа -
              {                  // проверим тот, с которого начали искать
                ExistNonSpace=LastExistNonSpace;
                if(ExistNonSpace)
                  f=lastX;
              }
            }

            if(ExistNonSpace) // нашли ближайшую правую позицию!
            {
              Ret=f;
            }
          }
          break;
       }
    }

  _D(int espCurLine=esp.CurLine;)
  RestorePosition(EI); // восстановим позицию курсора
  _D(SysLog(L"GetNextCoordX=%d Line=%d", Ret, espCurLine));
  return Ret;
}

void RestorePosition(const EditorInfo &EI)
{
   esp.CurLine=EI.CurLine;
   esp.CurPos=EI.CurPos;
   esp.TopScreenLine=EI.TopScreenLine;
   esp.LeftPos=EI.LeftPos;
   esp.CurTabPos=-1;
   EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
}

BOOL ChangeCoordX(const EditorInfo &ei, EditorSetPosition &esp)
{
   EditorConvertPos ecp={sizeof(EditorConvertPos), -1, esp.CurPos, 0};
   EditorControl(-1, ECTL_REALTOTAB, 0, &ecp);
   if(ecp.DestPos>ei.WindowSizeX)
   {
      ecp.SrcPos=ecp.DestPos-ei.WindowSizeX/2;
      EditorControl(-1, ECTL_TABTOREAL, 0, &ecp);
      esp.LeftPos=ecp.DestPos;
   }

   if(EditorControl(-1, ECTL_SETPOSITION, 0, &esp))
   {
      if(!(ei.Options&EOPT_PERSISTENTBLOCKS))
        EditorUnmarkBlock();
      EditorControl(-1, ECTL_REDRAW, 0, 0);
      return 1;
   }
   return 0;
}

BOOL GotoHome(const EditorInfo &ei, NODEDATA &nodedata)
{
   InitESPandEGS(esp, egs);
   InitNLS(ei, nodedata);
   EditorControl(-1, ECTL_GETSTRING, 0, &egs);
   esp.LeftPos=esp.CurPos=0;
   for(int i=0;i<egs.StringLength;++i)
     if(!IsCSpaceOrTab(egs.StringText[i]))
     {
       esp.CurPos=i;
       break;
     }

   if(ei.CurPos && esp.CurPos>=ei.CurPos) esp.CurPos=0;
   return esp.CurPos?ChangeCoordX(ei, esp):FALSE;
}

BOOL CmpWithFileMask(const wchar_t *Mask, const wchar_t *Name, bool SkipPath)
{
   if(0==lstrcmp(Mask,L":abstract:")) return FALSE;
   DWORD Flags=PN_CMPNAMELIST;
   if(SkipPath)
     Flags|=PN_SKIPPATH;
   BOOL Result = (BOOL)FSF.ProcessName(Mask, const_cast<wchar_t*>(Name), 0, Flags);
   _D(SysLog(L"CmpWithFileMask: %s <> %s => %s", Mask, Name, Result ? L"YES" : L"NO"));
   return Result;
}

void ApplyEditorOptions(NODEDATA &Settings, const wchar_t *FileName)
{
   CEditorOptions eo(Settings);
   eo.ApplyAllOptions();
   Settings.LockFile.Off();
   if(Settings.Options&E_LockFile_On)
     Settings.LockFile.On(FileName);
}

BOOL ProcessKeyEnter(const EditorInfo &ei, EditorSetPosition &esp,
                     EditorGetString &egs, int nQuote, bool isWrapQuoteEOL)
{
  if(nQuote<1) return FALSE;

  struct EditorGetString egs2={sizeof(EditorGetString)};

  if( ei.CurPos>=egs.StringLength ) { // check if that's the last line of quote
      if( ei.CurLine>=ei.TotalLines || !isWrapQuoteEOL) return FALSE;
      egs2.StringNumber=ei.CurLine+1;
      EditorControl(-1, ECTL_GETSTRING, 0, &egs2);
      if( !IsSameQuote(egs.StringText,nQuote,egs2.StringText,
                       IsQuote(egs2.StringText,egs.StringLength))
        )
        return FALSE;
  }

  wchar_t *buff=static_cast<wchar_t*>(malloc((nQuote+1)*sizeof(wchar_t)));
  if(buff==NULL) return FALSE;

  memcpy(buff,egs.StringText,nQuote*sizeof(wchar_t));
  buff[nQuote]=L'\0';

  BOOL RetCode=TRUE;
  for(intptr_t i=ei.CurPos; i<nQuote; i++ )
    if( buff[i]!=L' ' )
    {
      RetCode=FALSE;
      break;
    }

  if(RetCode)
  {
    EditorControl(-1, ECTL_INSERTSTRING, 0, 0);
    buff[nQuote]=L'\0';
    EditorControl(-1, ECTL_INSERTTEXT, 0, buff);
    esp.CurPos=nQuote;
    EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
    EditorControl(-1, ECTL_REDRAW, 0, 0);
  }

  free(buff);
  return RetCode;
}

BOOL ProcessKeyDelete(EditorGetString &egs, int nQuote)
{
  if(nQuote<1) return FALSE;

  wchar_t *buff=static_cast<wchar_t*>(malloc((nQuote+1)*sizeof(wchar_t)));
  if(buff==NULL) return FALSE;

  memcpy(buff,egs.StringText,nQuote*sizeof(wchar_t));
  buff[nQuote]=L'\0';

  EditorControl(-1, ECTL_DELETECHAR, 0, 0);

  EditorControl(-1, ECTL_GETSTRING, 0, &egs);
  EditorControl(-1, ECTL_GETINFO, 0, &ei);

  int i=IsQuote(&egs.StringText[ei.CurPos],egs.StringLength-ei.CurPos);
  if( IsSameQuote(buff,nQuote,&egs.StringText[ei.CurPos],i) )
  {
    while( i-- ) EditorControl(-1, ECTL_DELETECHAR, 0, 0);
    buff[0]=L' ';
    buff[1]=L'\0';
    EditorControl(-1, ECTL_INSERTTEXT, 0, buff);
  }

  EditorControl(-1, ECTL_REDRAW, 0, 0);
  free(buff);
  return TRUE;
}

BOOL InsertAdditionalSymbol(const EditorInfo &ei,
                            EditorSetPosition &esp,
                            EditorSetString &ess, EditorGetString &egs,
                            const strcon &AddSym_S,
                            const strcon &AddSym_E,
                            wchar_t Symbol, BOOL IncreaseCoordX)
{
  BOOL RetCode=FALSE;
  wchar_t *AddSym_Pos=wcschr(AddSym_S.str, Symbol);
  if(AddSym_Pos)
  {
    InitESPandEGS(esp, egs);
    EditorControl(-1, ECTL_GETSTRING, 0, &egs);
    if(ei.CurPos>=egs.StringLength || IsCSpaceOrTab(egs.StringText[ei.CurPos])
       || (!IncreaseCoordX && wcschr(AddSym_E.str,egs.StringText[ei.CurPos]))
      )
    {
      ess.StringNumber=-1;
      ess.StringEOL=const_cast<wchar_t*>(egs.StringEOL);
      if(ei.CurPos<egs.StringLength)
        ess.StringLength=egs.StringLength+2;
      else
        ess.StringLength=ei.CurPos+2;
      ess.StringText=static_cast<wchar_t*>(malloc(ess.StringLength*sizeof(wchar_t)));
      if(ess.StringText)
      {
        if(egs.StringLength>ei.CurPos)
        {
          memcpy((wchar_t*)ess.StringText, egs.StringText, ei.CurPos*sizeof(wchar_t));
          memcpy((wchar_t*)ess.StringText+ei.CurPos+2, egs.StringText+ei.CurPos,
                 (egs.StringLength-ei.CurPos)*sizeof(wchar_t));
        }
        else
        {
          memcpy((wchar_t*)ess.StringText, egs.StringText, egs.StringLength*sizeof(wchar_t));
          wmemset((wchar_t*)ess.StringText+egs.StringLength, nlsSpace,
                  ei.CurPos-egs.StringLength);
        }

        ((wchar_t *)ess.StringText)[ei.CurPos]=Symbol;
        ((wchar_t *)ess.StringText)[ei.CurPos+1]=AddSym_E.str[AddSym_Pos-AddSym_S.str];

        if(EditorControl(-1, ECTL_SETSTRING, 0, &ess))
        {
          RetCode=TRUE;
          InitESPandEGS(esp, egs);
          esp.CurPos=ei.CurPos+1;
          if(IncreaseCoordX) ++esp.CurPos;
          EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
          EditorControl(-1, ECTL_REDRAW, 0, 0);
        }

        free((wchar_t*)ess.StringText);
      }
    }
  }
  return RetCode;
}

// Возвращает номер кодовой таблицы с именем name или -1, если не нашли таблицу
int FindCodeTable(const wchar_t *Mask)
{
  int ret = -1;
/*
  int ct = 0;
  if (*Mask)
    while (-1 < Info.CharTable(ct, reinterpret_cast<wchar_t *>(&tmpCharSet),
                               sizeof(tmpCharSet)))
      {
        if (CmpWithFileMask(Mask, tmpCharSet.TableName, FALSE))
          {
            ret = ct;
            break;
          }
        ++ct;
      }
*/
  return ret;
}

void EditorUnmarkBlock()
{
  EditorSelect es={sizeof(EditorSelect),BTYPE_NONE,0,0,0,0};
  EditorControl(-1, ECTL_SELECT, 0, &es);
}

DWORD NormalizeControlState(DWORD State)
{
  DWORD Result=0;
  if(State&(RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)) Result|=LEFT_ALT_PRESSED;
  if(State&(RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED)) Result|=LEFT_CTRL_PRESSED;
  if(State&SHIFT_PRESSED) Result|=SHIFT_PRESSED;
  if(State&ENHANCED_KEY) Result|=ENHANCED_KEY;
  return Result;
}

#endif // __mix_cpp
