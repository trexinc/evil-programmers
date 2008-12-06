#ifndef __mix_cpp
#define __mix_cpp

#include "myrtl.hpp"
#include "plugin.hpp"
#include "mix.hpp"
#include "XMLStrings.hpp"
#include "options.hpp"
#include "xmlite.hpp"
#include "lang.hpp"
#include "InitDialogItem.hpp"
#include "strcon.hpp"
#include "TArray.cpp"
#include "kserror.hpp"

extern BOOL IsNewMacro;
extern TArray<NODEDATA> *NodeData;
extern CRedBlackTree<ESCFileInfo> *FileInfoTree;
extern PluginStartupInfo Info;
extern FarStandardFunctions FSF;
extern char XMLFilePath[NM];
extern BOOL XMLLoadedOK, IsOldFar;
extern XMLStrings XMLStr;
extern struct OPTIONS Opt;
extern int ModuleNumber;
extern FARAPIEDITORCONTROL EditorControl;
extern FARAPICHARTABLE CharTable;
extern FARAPIMESSAGE FarMessage;
extern FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
extern FARSTDTRUNCPATHSTR TruncPathStr;
extern EditorSetPosition esp;
extern EditorGetString egs;
extern EditorSetString ess;
extern EditorInfo ei;
extern CharTableSet CharSet, tmpCharSet;
extern strcon *nlsStopChars;

extern "C" const int maxTabSize=512;

inline int atoi(const char *s) { return FSF.atoi(s); }

void InitNLS(const EditorInfo &ei, NODEDATA &nodedata)
{
   _D(SysLog("InitNLS: AnsiMode=%d/%d, TableNum=%d/%d", ei.AnsiMode,nodedata.AnsiMode,ei.TableNum,nodedata.TableNum));
   nlsSpace=0x20;
   nlsTab=0x09;
   nlsQuoteSym='>';
   nlsMinus='-';
   if(nlsStopChars)
     *nlsStopChars=nodedata.StopChar;
   BOOL ReSet=FALSE;
   if(ei.AnsiMode)
   {
     ReSet=TRUE;
     if(ei.AnsiMode!=nodedata.AnsiMode)
     {
       _D(SysLog("InitNLS: rebuild ansi table"));
       char toUpper[2], toLower[2], decode[2], encode[2];
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
       _D(SysLog("InitNLS: rebuild FAR table"));
       ReSet=(-1!=CharTable(ei.TableNum,(char*)&CharSet,sizeof(CharSet)));
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
}

int FARPostMacro(const KeySequence *KS)
{
  if(IsNewMacro)
  {
    static ActlKeyMacro KeyMacro;
    memset(&KeyMacro,0,sizeof(KeyMacro));
    KeyMacro.Command=MCMD_POSTMACROSTRING;
    KeyMacro.Param.PlainText.SequenceText=reinterpret_cast<char *>(KS->Sequence);
    KeyMacro.Param.PlainText.Flags=KS->Flags;
    _D(int res=Info.AdvControl(ModuleNumber,ACTL_KEYMACRO,&KeyMacro));
    _D(SysLog("FARPostMacro: [%s] Flags=%p, ExitCode=%d",
       KeyMacro.Param.PlainText.SequenceText, KS->Flags, res));
    _D(return res);
    return Info.AdvControl(ModuleNumber,ACTL_KEYMACRO,&KeyMacro);
  }
  return Info.AdvControl(ModuleNumber,ACTL_POSTKEYSEQUENCE,const_cast<KeySequence*>(KS));
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
int IsQuote(const char* pszStr, size_t nLength)
{
  size_t i=0;
  while( i<nLength && i<4 )
  {
    if( pszStr[i++]==nlsQuoteSym )
    {
      while(i<nLength && (pszStr[i]==nlsQuoteSym || IsCSpaceOrTab(pszStr[i])))
        i++;
      return i;
    }
  }
  return 0;
}

// CharSet must be initialized already!
int IsSameQuote(const char* pszQuote1, size_t nLen1, const char*pszQuote2,
                size_t nLen2)
{
  while( nLen1 )
    if( nlsSpace==pszQuote1[nLen1-1] ) nLen1--;
    else break;
  while( nLen2 )
    if( nlsSpace==pszQuote2[nLen2-1] ) nLen2--;
    else break;
  return nLen1==nLen2 && !memcmp(pszQuote1,pszQuote2,nLen1);
}

BOOL FileExists(const char *Name)
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

DWORD PackEOL(const char *eolstr)
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
    _D(SysLog("PackEOL: eolstr[%s], Res=%u", eolstr, Res));
  }
  return Res;
}

BOOL UnpackEOL(DWORD EOL, char *Dest)
{
   if(!Dest) return FALSE;
   *Dest=EOL & 0xFF;
   Dest[1]=(EOL & 0xFF00)/256;
   Dest[2]=(EOL & 0xFF0000)/65536;
   Dest[3]=(EOL & 0xFF000000)/16777216;
   Dest[4]=0;
   _D(SysLog("UnpackEOL: EOL=%u, Dest=[%s]", EOL, Dest));
   return TRUE;
}

void InitDialogItems(struct InitDialogItem *Init,
                     struct FarDialogItem *Item,
                     int ItemsNumber)
{
  for (int I = 0; I < ItemsNumber; ++I)
    {
      Item[I].Type = Init[I].Type;
      Item[I].X1 = Init[I].X1;
      Item[I].Y1 = Init[I].Y1;
      Item[I].X2 = Init[I].X2;
      Item[I].Y2 = Init[I].Y2;
      Item[I].Focus = Init[I].Focus;
      Item[I].Selected = Init[I].Selected;
      Item[I].Flags = Init[I].Flags;
      Item[I].DefaultButton = Init[I].DefaultButton;
      if ((unsigned int) Init[I].Data < 2000)
        lstrcpy(Item[I].Data, GetMsg((unsigned int) Init[I].Data));
      else
        lstrcpy(Item[I].Data, Init[I].Data);
    }
}
//возвращает строку из LNG в соответствие с языковыми настройками
const char *GetMsg(int MsgId) {return (Info.GetMsg(ModuleNumber, MsgId));}

int FarAllInOneMessage(const char *Message, unsigned int Flags)
{
  return FarMessage(ModuleNumber, Flags|FMSG_ALLINONE, NULL,
                      (const char * const *)Message, 1, 1);
}

void MessageFileError(const char *msg, const char *name)
{
  const char *MsgItems[4];
  strcon fn(name);
  TruncPathStr(fn.str,62);
  MsgItems[0] = GetMsg(MWarning);
  MsgItems[1] = msg;
  MsgItems[2] = fn.str;
  MsgItems[3] = GetMsg(MOk);
  FarMessage(ModuleNumber, FMSG_WARNING, NULL, MsgItems, 4, 1);
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
  const char *Attr=n.Attr(XMLStr.AutoWrap);
  if(*Attr)
  {
    if(Inherit)
      node.Options&=~E_AutoWrap_On;

    if(!stricmp(Attr, XMLStr.On))
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
      node.Options&=~E_Wrap_Percent;
    }

    int len=strlen(Attr)-1, newAttr;
    if(Attr[len]=='%')
    {
      newAttr=0;
      strcon tmp(Attr);
      if(tmp.str)
      {
        tmp.str[len]=0;
        newAttr=FSF.atoi(tmp.str);
        node.Options|=E_Wrap_Percent;
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
        node.Options&=~E_Wrap_Justify;

      if(!stricmp(Attr, XMLStr.On))
        node.Options|=E_Wrap_Justify;
    }
  }
  else
    node.Options&=~E_Wrap_Justify;
}

void SetOption(const char *Attr,DWORD &Options,DWORD Value,
               const int &Inherit, BOOL OnByDefault)
{
  if(*Attr)
  {
    if(Inherit)
      Options&=~Value;

    if(!stricmp(Attr, XMLStr.On))
      Options|=Value;
  }
  else if(!Inherit && OnByDefault)
    Options|=Value;
}

void SetOption(const char *Attr,DWORD &Options,
               DWORD ValueOn, DWORD ValueOff,
               const int &Inherit)
{
  if(*Attr)
  {
    if(Inherit)
    {
      Options&=~ValueOn;
      Options&=~ValueOff;
    }

    if(!stricmp(Attr, XMLStr.On))
      Options|=ValueOn;
    else if(!stricmp(Attr, XMLStr.Off))
      Options|=ValueOff;
  }
}

void SetNumOpt(const char *Attr, DWORD &Num, DWORD Default, const int &Inherit)
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

void SetStrOpt(const char *Attr, strcon &DestStr, const strcon &Default, const int &Inherit)
{
  if(*Attr)
    DestStr=Attr;
  else if(!Inherit)
    DestStr=Default;
}

void SetAddSymbolOpt(const char *Attr, CXMLNode &n, NODEDATA &node,
                     const int &Inherit, bool NewConfig)
{
  SetOption(n.Attr(const_cast<char*>(Attr)),node.Options,E_AddSymbol_On,Inherit,FALSE);
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
  const char *Attr=n.Attr(XMLStr.TabSize);
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
    }

    if(!stricmp(Attr, XMLStr.On))
      node.Options|=E_ExpandTabs_On;
    else if(!stricmp(Attr, XMLStr.Off))
      node.Options|=E_ExpandTabs_Off;
  }
}

bool InsertMacro(CXMLNode &n,CUserMacros &Macro,
  const DWORD Key, const DWORD ButtonState, const DWORD ControlState,
  DWORD &Flags, bool silent,BOOL Stop,int &Error,strcon &unknownKey)
{
  const char *Attr;
  if(!Flags)
  {
    Attr=n.Attr(XMLStr.Selection);
    if(!stricmp(Attr,XMLStr.On))
      Flags|=KSSF_SELECTION_ON;
    else if(!stricmp(Attr,XMLStr.Off))
      Flags|=KSSF_SELECTION_OFF;
    else if(!stricmp(Attr,XMLStr.Stream))
      Flags|=KSSF_SELECTION_ON|KSSF_SELECTION_STREAM;
    else if(!stricmp(Attr,XMLStr.Column))
      Flags|=KSSF_SELECTION_ON|KSSF_SELECTION_COLUMN;
    else
      Flags|=KSSF_SELECTION_ON|KSSF_SELECTION_OFF;
  }
  Attr=n.Attr(XMLStr.Sequence);
  UserMacroID id(Key,Flags,ButtonState,ControlState);
  return Macro.InsertMacro(id,Attr,silent,Stop,Error,unknownKey);
}

bool AddMacro(CXMLNode &n,NODEDATA &node,int &Error,strcon &unknownKey)
{
  bool res=true, silent=true, Stop=true;
  DWORD ButtonState=0, Flags=0;
  const char *Attr;

  if(!stricmp(n.Attr(XMLStr.Stop),XMLStr.Off))
    Stop=false;

  Attr=n.Attr(XMLStr.Silent);
  if(*Attr && 0!=stricmp(Attr,XMLStr.On))
    silent=false;

  // проверим автостартующие макросы
  TList<KeySequenceStorage> *Macros=NULL;
  Attr=n.Attr(XMLStr.Auto);
  if(!stricmp(Attr,XMLStr.onCreate))
  {
    _D(SysLog("AddMacro: OnCreateMacros"));
    Macros=&node.OnCreateMacros;
  }
  else if(!stricmp(Attr,XMLStr.onLoad))
  {
    _D(SysLog("AddMacro: OnLoadMacros"));
    Macros=&node.OnLoadMacros;
  }
  if(Macros)
  {
    KeySequenceStorage tmpMacro;
    if(tmpMacro.compile(n.Attr(XMLStr.Sequence), silent, 1, Stop, Error, unknownKey))
    {
      Macros->insert(tmpMacro);
      _D(SysLog("AddMacro: 'on'-macros: inserted"));
    }
    else
    {
      Error=KSE_UNKNOWNKEY;
      res=false;
      _D(SysLog("AddMacro: 'on'-macros: NOT inserted (unknownKey='%s')", unknownKey.str?unknownKey.str:"(null)"));
    }
  }

  if(res)
  {
    if(!stricmp(n.Attr(XMLStr.LClick),XMLStr.On))
      ButtonState|=EBS_LClick_On;
    if(!stricmp(n.Attr(XMLStr.RClick),XMLStr.On))
      ButtonState|=EBS_RClick_On;
    if(!stricmp(n.Attr(XMLStr.MClick),XMLStr.On))
      ButtonState|=EBS_MClick_On;

    if(0!=ButtonState) // есть мышиный макрос
    {
      DWORD ControlState=0;
      if(!stricmp(n.Attr(XMLStr.RCtrl),XMLStr.On))
        ControlState|=ECS_RCtrl_On;
      if(!stricmp(n.Attr(XMLStr.LCtrl),XMLStr.On))
        ControlState|=ECS_LCtrl_On;
      if(!stricmp(n.Attr(XMLStr.RAlt),XMLStr.On))
        ControlState|=ECS_RAlt_On;
      if(!stricmp(n.Attr(XMLStr.LAlt),XMLStr.On))
        ControlState|=ECS_LAlt_On;
      if(!stricmp(n.Attr(XMLStr.Shift),XMLStr.On))
        ControlState|=ECS_Shift_On;
      res=InsertMacro(n,node.MouseMacros,0,ButtonState,ControlState,Flags,
           silent,Stop,Error,unknownKey);
    }
  }

  if(res && *(Attr=n.Attr(XMLStr.Key)))
  {
    int Key=FSF.FarNameToKey(Attr);
    if(Key==-1)
    {
      Error=KSE_UNKNOWNKEY;
      unknownKey=Attr;
      res=false;
    }
    else
      res=InsertMacro(n,node.KeyMacros,Key,0,0,Flags,silent,Stop,
          Error,unknownKey);
  }
  return res;
}

void SetCharCodeBaseOpt(const char *Attr, DWORD &Options, const int &Inherit)
{
  if(*Attr)
  {
    if(Inherit)
    {
      Options&=~E_CharCodeBase_Oct;
      Options&=~E_CharCodeBase_Dec;
      Options&=~E_CharCodeBase_Hex;
    }

    if(!stricmp(Attr, XMLStr.Oct))
      Options|=E_CharCodeBase_Oct;
    else if(!stricmp(Attr, XMLStr.Dec))
      Options|=E_CharCodeBase_Dec;
    else if(!stricmp(Attr, XMLStr.Hex))
      Options|=E_CharCodeBase_Hex;
  }
}

void SetEOLOpt(const char *Attr, DWORD &EOL)
{
  if(*Attr)
    EOL=PackEOL(Attr);
}

void ShowCfgError(const char *FileName, const char *Type,
                  const char *Param, const char *ErrorMsg,
                  const char *Comment)
{
  const char *MsgItems[7];
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
  FarMessage(ModuleNumber, FMSG_WARNING, NULL, MsgItems, i, 1);
}

/* Обработка конфигурационных файлов
   Возвращается:
   Число > 0 - успех
    0        - файл содержит ошибки
   -1        - ошибка выделения памяти
   -2        - файл не найден
*/
int ParseFile(const pchar filename,CRedBlackTree<ESCFileInfo>&FITree,
              TArray<NODEDATA>&NodeData, const bool test)
{
  char FileName[MAX_PATH];
  ExpandEnvironmentStr(filename,FileName,sizeof(FileName));
  _D(SysLog("ParseFile: [%s] - start", FileName));
  int rc=0;
  ESCFileInfo fi(const_cast<const char*>(FileName));
  if (!FileName || FITree.findNode(fi))
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
    const char *Attr;
    BOOL Error=FALSE;
    {
      i=NULL;
      int newAttr, SameType;
      BOOL Inherit;
      while(!Error && NULL!=(i=p.EnumName(XMLStr.Type, i,n)))
      {
        #ifndef LITE_VERSION
        Attr=n.Attr(XMLStr.Include);
        if(*Attr)
        {
          _D(SysLog("ParseFile: [%s], include=[%s]", FileName, Attr));
          strcon newName;
          if((Attr[1]==':' && Attr[2]) || (Attr[0]=='\\' && Attr[1]=='\\'))
            newName=Attr;
          else
          {
            const char *slash=strrchr(FileName,'\\');
            int l=slash?slash-FileName+1:0;
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
        #endif // LITE_VERSION
        Attr=n.Attr(XMLStr.Name);
        _D(SysLog("ParseFile: [%s], type=[%s]", FileName, Attr));
        if(*Attr)
        {
          SameType=-1;
          #ifdef LITE_VERSION
          Inherit=FALSE;
          #endif
          NODEDATA node;
          node.Name=Attr;
          if(node.Name.str)
          {
           #ifndef LITE_VERSION
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
           #endif // LITE_VERSION

           Attr=n.Attr(XMLStr.Mask);
           _D(SysLog("ParseFile: [%s], Mask=[%s], SameType=%d",
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
                if(!*Attr || !stricmp(Attr, XMLStr.On))
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
              SetStrOpt(N.Attr(XMLStr.Name),node.TableName,"",Inherit);

              N=n.GetItem(XMLStr.AddSymbol);
              SetAddSymbolOpt(XMLStr.Value,N,node,Inherit,true);

              N=n.GetItem(XMLStr.LockMode);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_LockMode_On,E_LockMode_Off,Inherit);

              N=n.GetItem(XMLStr.LockFile);
              SetOption(N.Attr(XMLStr.Value),node.Options,E_LockFile_On,Inherit,FALSE);

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
                  if(!*Attr || !stricmp(Attr, XMLStr.On))
                  {
                    if(!AddMacro(macroN,node,macro_err,unknownKey))
                      {
                        Error=TRUE;
                        if(macro_err==KSE_UNKNOWNKEY)
                        {
                          comment.AddStrings("\"",unknownKey.str,"\"",0);
                          param=XMLStr.UserMacro;
                          if(*(Attr=macroN.Attr(XMLStr.Key)))
                            param.AddStrings("/",XMLStr.Macro,"/key=",Attr,0);
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
              SetStrOpt(N.Attr(XMLStr.Additional),node.AdditionalLetters,"",Inherit);

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

              //_D(SysLog("ReloadSettings: Name=[%s], tabsize=%d wrap=%d Justify=%s",
              //   node.Name, node.TabSize, node.WrapPos, node.Options&E_Wrap_Justify?"on":"off"));
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
            #ifndef LITE_VERSION
            if(!nd)
              Error=TRUE;
            #endif
          }
        }
        #ifdef LITE_VERSION
        break;
        #endif
      }
    }
  }
  _D(SysLog("ParseFile: [%s] - end (%d)", FileName, rc));
  return rc;
}

void TestCfgFiles()
{
  _D(SysLog("TestCfgFiles: start"));
  CRedBlackTree<ESCFileInfo> FITree(ESCFICompLT, ESCFICompEQ);
  TArray<NODEDATA> ND;
  int rc=ParseFile(XMLFilePath,FITree,ND,true);
  if(rc==-1)
    FarAllInOneMessage(GetMsg(MCannotAllocateAMemory), FMSG_WARNING);
  else if(rc>0)
    FarAllInOneMessage(GetMsg(MConfigIsOK), 0);
  _D(SysLog("TestCfgFiles: end = %d", rc));
}

BOOL ReloadSettings(BOOL Force)
{
  if(IsOldFar) return FALSE;
  _D(SysLog("ReloadSettings(%d) - start", Force));

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

  _D(SysLog("ReloadSettings(%d) = %d", Force, XMLLoadedOK));
  return XMLLoadedOK;
}

void InitESPandEGS(struct EditorSetPosition &esp, struct EditorGetString &egs)
{
   memset(&esp, -1, sizeof(esp));
   egs.StringNumber=-1;
}

int Size;
bool prc_Minuses;

inline bool IsMinusMinusSpace(const EditorGetString &egs)
{
  return egs.StringLength==3 && *egs.StringText==egs.StringText[1] &&
        nlsMinus==*egs.StringText && nlsSpace==egs.StringText[2];
}

void KillSpaces()
{
   EditorControl(ECTL_SETPOSITION,&esp);
   EditorControl(ECTL_GETSTRING,&egs);
   _D(SysLog("ks: StringLength=%d", egs.StringLength));
   if(egs.StringLength>0 && !(prc_Minuses && IsMinusMinusSpace(egs)))
   {
     Size=egs.StringLength-1;
     while(IsCSpaceOrTab(egs.StringText[Size])) --Size;
     ++Size;
     if(Size!=egs.StringLength)
     {
       _D(SysLog("ks: 1. esp.CurLine=%d", esp.CurLine));
       ess.StringNumber=-1;
       ess.StringText=const_cast<char*>(egs.StringText);
       ess.StringEOL =const_cast<char*>(egs.StringEOL);
       ess.StringLength=Size;
       EditorControl(ECTL_SETSTRING, &ess);
     }
     _D(else SysLog("ks: 2. esp.CurLine=%d", esp.CurLine));
   }
}

char GLOBAL_EOL[16];
void KillSpacesAndChangeEOL()
{
   EditorControl(ECTL_SETPOSITION,&esp);
   EditorControl(ECTL_GETSTRING,&egs);

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

   if(*egs.StringEOL && 0!=strcmp(GLOBAL_EOL,egs.StringEOL))
   {
     _D(SysLog("ksceol: 1. esp.CurLine=%d", esp.CurLine));
     ess.StringNumber=-1;
     ess.StringText=const_cast<char*>(egs.StringText);
     ess.StringEOL=GLOBAL_EOL;
     ess.StringLength=Size;
     EditorControl(ECTL_SETSTRING, &ess);
   }
   else if(Size!=egs.StringLength)
   {
     _D(SysLog("ksceol: 2. esp.CurLine=%d", esp.CurLine));
     ess.StringNumber=-1;
     ess.StringText=const_cast<char*>(egs.StringText);
     ess.StringEOL =const_cast<char*>(egs.StringEOL);
     ess.StringLength=Size;
     EditorControl(ECTL_SETSTRING, &ess);
   }
   _D(else SysLog("ksceol: 3. esp.CurLine=%d", esp.CurLine));
}

void ChangeEOL()
{
   EditorControl(ECTL_SETPOSITION,&esp);
   EditorControl(ECTL_GETSTRING,&egs);
   _D(SysLog("ceol: 0. myeol=[%s], original_eol=[%s]", GLOBAL_EOL, egs.StringEOL));
   if(*egs.StringEOL && 0!=strcmp(GLOBAL_EOL,egs.StringEOL))
   {
     _D(SysLog("ceol: 1. esp.CurLine=%d", esp.CurLine));
     ess.StringNumber=-1;
     ess.StringText=const_cast<char*>(egs.StringText);
     ess.StringEOL=GLOBAL_EOL;
     ess.StringLength=egs.StringLength;
     EditorControl(ECTL_SETSTRING, &ess);
   }
   _D(else SysLog("ceol: 2. esp.CurLine=%d", esp.CurLine));
}

char *TruncFromRigth(char *Str, unsigned int maxsize, BOOL AddSpaces)
{
  unsigned int len=strlen(Str);
  if (maxsize < 5) maxsize = 5;
  if (len > maxsize) strcpy(Str+maxsize-3, "...");
  else if(AddSpaces)
  {
     memset(Str+len, 0x20, maxsize-len);
     Str[maxsize]=0;
  }
  return Str;
}

int CalcWrapPos(const NODEDATA &Data, const EditorInfo &ei)
{
   if(Data.Options & E_Wrap_Percent)
   {
      int w=ei.WindowSizeX*Data.WrapPos/100;
      return (w>511)?511:w;
   }
   else
      return Data.WrapPos;
}

int GetPrevCoordX(EditorInfo &ei, int Lines, const char *StopChars)
{
   int oldX(ei.CurPos), oldY(ei.CurLine), CoordX(-1), LastCoordX(ei.CurPos),
       ForceRight(TRUE);
   if(oldX>0)
   {
     ei.CurPos=0;
     do
     {
       CoordX=GetNextCoordX(ei, Lines, StopChars);
       if(ForceRight)
       {
         int oldCoordX=CoordX, LastX=-1;
         if(CoordX!=-1) do
         {
            ei.CurPos=LastX=CoordX;
            CoordX=GetNextCoordX(ei, Lines, StopChars);
         } while(CoordX>0 && CoordX<oldX);
         CoordX=oldCoordX;
         if(LastX>-1 && LastX<oldX)
         {
           LastCoordX=LastX;
           _D(SysLog("GetPrevCoordX: LastX=%d", LastX));
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

   _D(SysLog("GetPrevCoordX = %d", LastCoordX));
   return LastCoordX;
}

int GetNextCoordX(const EditorInfo &EI, int Lines, const char *StopChars)
{
  int Ret=-1, tmpX, f, ExistNonSpace, LastExistNonSpace;
  _D(esp.CurLine=EI.CurLine-1);
  InitESPandEGS(esp, egs);

  for(esp.CurLine=EI.CurLine-1; Lines && esp.CurLine>-1; --esp.CurLine, --Lines)
    {
       EditorControl(ECTL_SETPOSITION,&esp);
       EditorControl(ECTL_GETSTRING,&egs);
       if(egs.StringLength)
         while(IsCSpaceOrTab(egs.StringText[egs.StringLength-1]))
            --egs.StringLength;

       _D(SysLog("GetNextCoordX(%d) egs.StringLength=%d", esp.CurLine, egs.StringLength));
       if(egs.StringLength>0)
       {
          tmpX=EI.CurPos;
          if(tmpX<egs.StringLength)
          {
            _D(SysLog("GetNextCoordX(%d): [%s]", esp.CurLine, egs.StringText));
            _D(SysLog("GetNextCoordX: StopChars=[%s]", StopChars));
            // проверим StopChars
            if(StopChars)
            {
              for(; tmpX<egs.StringLength &&
                  !IsCSpaceOrTab(egs.StringText[tmpX]); ++tmpX)
                if(NULL!=strchr(StopChars, egs.StringText[tmpX]))
                {
                  Ret=tmpX+1;
                  while(NULL!=strchr(StopChars, egs.StringText[Ret]))
                    ++Ret;
                  break;
                }
              if(Ret!=-1) // уже нашли, что искали
                break;
              _D(SysLog("GetNextCoordX: StopChars not found"));
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
              int lastX=tmpX; // запомним tmpX
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
  _D(SysLog("GetNextCoordX=%d Line=%d", Ret, espCurLine));
  return Ret;
}

void RestorePosition(const EditorInfo &EI)
{
   esp.CurLine=EI.CurLine;
   esp.CurPos=EI.CurPos;
   esp.TopScreenLine=EI.TopScreenLine;
   esp.LeftPos=EI.LeftPos;
   esp.CurTabPos=-1;
   EditorControl(ECTL_SETPOSITION,&esp);
}

BOOL ChangeCoordX(const EditorInfo &ei, EditorSetPosition &esp)
{
   EditorConvertPos ecp={-1, esp.CurPos, 0};
   EditorControl(ECTL_REALTOTAB, &ecp);
   if(ecp.DestPos>ei.WindowSizeX)
   {
      ecp.SrcPos=ecp.DestPos-ei.WindowSizeX/2;
      EditorControl(ECTL_TABTOREAL, &ecp);
      esp.LeftPos=ecp.DestPos;
   }

   if(EditorControl(ECTL_SETPOSITION, &esp))
   {
      if(!(ei.Options&EOPT_PERSISTENTBLOCKS))
        EditorUnmarkBlock();
      EditorControl(ECTL_REDRAW, NULL);
      return 1;
   }
   return 0;
}

BOOL GotoHome(const EditorInfo &ei, NODEDATA &nodedata)
{
   InitESPandEGS(esp, egs);
   InitNLS(ei, nodedata);
   EditorControl(ECTL_GETSTRING, &egs);
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

BOOL CmpWithFileMask(const char *Mask, const char *Name, bool SkipPath)
{
   DWORD Flags=PN_CMPNAMELIST;
   if(SkipPath)
     Flags|=PN_SKIPPATH;
   return FSF.ProcessName(Mask, const_cast<char*>(Name), Flags);
}

void ApplyEditorOptions(NODEDATA &Settings, const char *FileName)
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

  struct EditorGetString egs2;

  if( ei.CurPos>=egs.StringLength ) { // check if that's the last line of quote
      if( ei.CurLine>=ei.TotalLines || !isWrapQuoteEOL) return FALSE;
      egs2.StringNumber=ei.CurLine+1;
      EditorControl(ECTL_GETSTRING,&egs2);
      if( !IsSameQuote(egs.StringText,nQuote,egs2.StringText,
                       IsQuote(egs2.StringText,egs.StringLength))
        )
        return FALSE;
  }

  char *buff=static_cast<char*>(malloc(nQuote+1));
  if(buff==NULL) return FALSE;

  memcpy(buff,egs.StringText,nQuote);
  buff[nQuote]='\0';
  struct EditorConvertText ect={buff, nQuote};
  EditorControl(ECTL_EDITORTOOEM,&ect);

  BOOL RetCode=TRUE;
  for(int i=ei.CurPos; i<nQuote; i++ )
    if( buff[i]!=' ' )
    {
      RetCode=FALSE;
      break;
    }

  if(RetCode)
  {
    EditorControl(ECTL_INSERTSTRING,0);
    buff[nQuote]='\0';
    EditorControl(ECTL_INSERTTEXT,buff);
    esp.CurPos=nQuote;
    EditorControl(ECTL_SETPOSITION,&esp);
    EditorControl(ECTL_REDRAW,NULL);
  }

  free(buff);
  return RetCode;
}

BOOL ProcessKeyDelete(EditorGetString &egs, int nQuote)
{
  if(nQuote<1) return FALSE;

  char *buff=static_cast<char*>(malloc(nQuote+1));
  if(buff==NULL) return FALSE;

  memcpy(buff,egs.StringText,nQuote);
  buff[nQuote]='\0';

  EditorControl(ECTL_DELETECHAR,0);

  EditorControl(ECTL_GETSTRING,&egs);
  EditorControl(ECTL_GETINFO,&ei);

  int i=IsQuote(&egs.StringText[ei.CurPos],egs.StringLength-ei.CurPos);
  if( IsSameQuote(buff,nQuote,&egs.StringText[ei.CurPos],i) )
  {
    while( i-- ) EditorControl(ECTL_DELETECHAR,0);
    buff[0]=' ';
    buff[1]='\0';
    EditorControl(ECTL_INSERTTEXT,buff);
  }

  EditorControl(ECTL_REDRAW,NULL);
  free(buff);
  return TRUE;
}

BOOL InsertAdditionalSymbol(const EditorInfo &ei,
                            EditorSetPosition &esp,
                            EditorSetString &ess, EditorGetString &egs,
                            const strcon &AddSym_S,
                            const strcon &AddSym_E,
                            BYTE Symbol, BOOL IncreaseCoordX)
{
  BOOL RetCode=FALSE;
  char *AddSym_Pos=strchr(AddSym_S.str, static_cast<char>(Symbol));
  if(AddSym_Pos)
  {
    InitESPandEGS(esp, egs);
    EditorControl(ECTL_GETSTRING,&egs);
    if(ei.CurPos>=egs.StringLength || IsCSpaceOrTab(egs.StringText[ei.CurPos])
       || (!IncreaseCoordX && strchr(AddSym_E.str,
       ( ei.AnsiMode || ei.TableNum!=-1 )?
       CharSet.DecodeTable[static_cast<BYTE>(egs.StringText[ei.CurPos])]:
       egs.StringText[ei.CurPos]))
      )
    {
      ess.StringNumber=-1;
      ess.StringEOL=const_cast<char*>(egs.StringEOL);
      if(ei.CurPos<egs.StringLength)
        ess.StringLength=egs.StringLength+2;
      else
        ess.StringLength=ei.CurPos+2;
      ess.StringText=static_cast<char*>(malloc(ess.StringLength));
      if(ess.StringText)
      {
        if(egs.StringLength>ei.CurPos)
        {
          memcpy(ess.StringText, egs.StringText, ei.CurPos);
          memcpy(ess.StringText+ei.CurPos+2, egs.StringText+ei.CurPos,
                 egs.StringLength-ei.CurPos);
        }
        else
        {
          memcpy(ess.StringText, egs.StringText, egs.StringLength);
          memset(ess.StringText+egs.StringLength, nlsSpace,
                 ei.CurPos-egs.StringLength);
        }

        if( ei.AnsiMode || ei.TableNum!=-1 )
        {
          ess.StringText[ei.CurPos]=CharSet.EncodeTable[Symbol];
          ess.StringText[ei.CurPos+1]=CharSet.EncodeTable[static_cast<BYTE>(AddSym_E.str[AddSym_Pos-AddSym_S.str])];
        }
        else
        {
          ess.StringText[ei.CurPos]=Symbol;
          ess.StringText[ei.CurPos+1]=AddSym_E.str[AddSym_Pos-AddSym_S.str];
        }

        if(EditorControl(ECTL_SETSTRING,&ess))
        {
          RetCode=TRUE;
          InitESPandEGS(esp, egs);
          esp.CurPos=ei.CurPos+1;
          if(IncreaseCoordX) ++esp.CurPos;
          EditorControl(ECTL_SETPOSITION, &esp);
          EditorControl(ECTL_REDRAW, NULL);
        }

        free(ess.StringText);
      }
    }
  }
  return RetCode;
}

// Возвращает номер кодовой таблицы с именем name или -1, если не нашли таблицу
int FindCodeTable(const char *Mask)
{
  int ret = -1, ct = 0;
  if (*Mask)
    while (-1 < Info.CharTable(ct, reinterpret_cast<char *>(&tmpCharSet),
                               sizeof(tmpCharSet)))
      {
        if (CmpWithFileMask(Mask, tmpCharSet.TableName, FALSE))
          {
            ret = ct;
            break;
          }
        ++ct;
      }
  return ret;
}

void EditorUnmarkBlock()
{
  EditorSelect es={BTYPE_NONE,0,0,0,0};
  EditorControl(ECTL_SELECT, &es);
}

#endif // __mix_cpp
