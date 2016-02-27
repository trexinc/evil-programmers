#include "winapi.hpp"
/*
void chroot(LPCWSTR root=NULL)
{
	if(root)
		*pStrRegRoot=root;
	else
		*pStrRegRoot=Info.RootKey;
}
*/

bool IsQuotePair(LPCWSTR Str)
{
	int count=0;
	for(int i=0;Str[i];i++)if(Str[i]==L'"')count++;
	return(count%2)?false:true;
}

bool CheckIsDirCmd(LPCWSTR Str)
{
	Tokenizer t(*Opt.DirCmds,L";");
	while(!t.IsEnd())
	{
		string cmd=t.Next();
		if(cmd==Str)
			return true;
	}
	return false;
}

BYTE CheckIsDir(LPCWSTR _Str)
{
	if(!_Str||!*_Str)return 0;
	LPCWSTR p=_Str;
	while(*p)p++;
	if(IsSlash(p[-1]))return 1;

	string Str=_Str;
	Unquote(Str);
	if((Str.At(0)==L'\\') && (Str.At(1)==L'\\'))
	{
		int SlashCount=0;
		for(UINT i=0;i<Str.GetLength();i++)
			if(IsSlash(Str.At(i)))SlashCount++;
		if((SlashCount<3)||IsSlash(Str.At(Str.GetLength()-1)))return 1;
	}
	string ExpStr;
	apiExpandEnvironmentStrings(Str,ExpStr);
	DWORD dwAttrs=apiGetFileAttributes(ExpStr);
	if(dwAttrs==INVALID_FILE_ATTRIBUTES)
		return 0; //none
	if(dwAttrs&FILE_ATTRIBUTE_DIRECTORY)
		return 1; //dir
	else
		return 2; //file
}

bool QuoteRight(wchar_t* Str)
{
	if(!CheckQuotedSymbols(Str))
		return false;
	size_t l=wcslen(Str);
	if(IsSlash(Str[l-1]))
	{
		Str[l+1]=0;
		Str[l]=Str[l-1];
		Str[l-1]=L'"';
	}
	else
	{
		Str[l+1]=0;
		Str[l]=L'"';
	}
	return true;
}

bool QuoteLeft(wchar_t* Str)
{
	if((!CheckQuotedSymbols(Str)&&!NeedFirstQuote)||*Str==L'"')
		return false;
	memmove(Str+1,Str,(wcslen(Str)+1)*sizeof(wchar_t));
	*Str=L'"';
	return true;
}

bool QuoteRight(string &Str)
{
	wchar_t* buffer=Str.GetBuffer(Str.GetLength()+2);
	bool ret=QuoteRight(buffer);
	Str.ReleaseBuffer();
	return ret;
}

bool QuoteLeft(string &Str)
{
	wchar_t* buffer=Str.GetBuffer(Str.GetLength()+2);
	bool ret=QuoteLeft(buffer);
	Str.ReleaseBuffer();
	return ret;
}

void PlayBeep(DWORD beep)
{
	int type=(beep&0x0000f000)>>12;
	switch(type)
	{
	case 0:
		break;
	case 1:
		Beep(HIWORD(beep),beep&0x00000FFF);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		MessageBeep((type-3)<<4);
	}
}

void ResetState()
{
	PrevState->line==L"";
	PrevState->pos=0;
	PrevState->selstart=-1;
	PrevState->selend=0;
	PrevState->PanelDir=L"";
	PrevState->Workspace=0;
	PrevState->hDlg=0;
}

void SaveState()
{
	PrevState->line=ThisState->line;
	PrevState->pos=ThisState->pos;
	PrevState->selstart=ThisState->selstart;
	PrevState->selend=ThisState->selend;
	PrevState->PanelDir=ThisState->PanelDir;
	PrevState->Workspace=ThisState->Workspace;
	PrevState->hDlg=ThisState->hDlg;
}

bool IsStateChanged()
{
	return
		!(
			(ThisState->pos==PrevState->pos)&&
			(ThisState->Workspace==PrevState->Workspace)&&
			(ThisState->selstart==PrevState->selstart)&&
			(ThisState->selend==PrevState->selend)&&
			(ThisState->line==PrevState->line)&&
			(ThisState->PanelDir==PrevState->PanelDir)&&
			(ThisState->hDlg==PrevState->hDlg)&&
			GlobalContainer
		);
}

bool GetState(INT_PTR Item)
{
	int Size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,NULL);
	FarPanelDirectory* Dir = static_cast<FarPanelDirectory*>(malloc(Size));
	Dir->StructSize = sizeof(FarPanelDirectory);
	Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,Size,Dir);
	ThisState->PanelDir = Dir->Name;
	
	WindowType wt={sizeof(wt)};
	Info.AdvControl(YacGuid, ACTL_GETWINDOWTYPE, 0, &wt);
	switch(wt.Type)
	{
		case WTYPE_PANELS: ThisState->Workspace=W_SHELL; break;
		case WTYPE_EDITOR: ThisState->Workspace=W_EDITOR; break;
		case WTYPE_DIALOG: ThisState->Workspace=W_DIALOG; break;
		default: ThisState->Workspace=W_SHELL;
	}
	switch(ThisState->Workspace)
	{
		case W_EDITOR:
		{
			//pos
			EditorInfo ei;
			Info.EditorControl(-1, ECTL_GETINFO, 0, &ei);
			if(ei.CurState&ECSTATE_LOCKED)
				return false;
			ThisState->pos=ei.CurPos;
			//data
			EditorGetString gs={-1};
			Info.EditorControl(-1, ECTL_GETSTRING,0, &gs);
			ThisState->line=gs.StringText;
			//selection
			ThisState->selstart=gs.SelStart;
			ThisState->selend=gs.SelEnd;
		}
		break;
		case W_SHELL:
		{
			//pos
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETCMDLINEPOS,0,&ThisState->pos);
			//data
			ThisState->line=L"";
			int Size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETCMDLINE,0,NULL);
			if(Size>1)
			{
				wchar_t* buffer=ThisState->line.GetBuffer(Size);
				Info.PanelControl(PANEL_ACTIVE,FCTL_GETCMDLINE,Size,buffer);
			}
			ThisState->line.ReleaseBuffer();
			if(ThisState->line.Empty())
				return false;
			//selection
			CmdLineSelect cls;
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETCMDLINESELECTION,0,&cls);
			ThisState->selstart=cls.SelStart;
			ThisState->selend=cls.SelEnd;
		}
		break;
		case W_DIALOG:
		{
			//pos
			ThisState->hDlg=reinterpret_cast<OpenDlgPluginData*>(Item)->hDlg;
			COORD Pos;
			ID=(int)Info.SendDlgMessage(ThisState->hDlg,DM_GETFOCUS,0,0);
			size_t Size=Info.SendDlgMessage(ThisState->hDlg,DM_GETDLGITEM,ID,NULL);
			FarGetDialogItem di={sizeof(di), Size, reinterpret_cast<FarDialogItem*>(malloc(Size))};
			Info.SendDlgMessage(ThisState->hDlg, DM_GETDLGITEM, ID, &di);
			int Type=di.Item->Type;
			FARDIALOGITEMFLAGS Flags=di.Item->Flags;
			if((Type==DI_EDIT)||(Type==DI_FIXEDIT))
				*DlgItemHistory=di.Item->History;
			free(di.Item);
			if(!((Type==DI_EDIT)||(Type==DI_FIXEDIT)||((Type==DI_COMBOBOX)&&!(Flags&DIF_DROPDOWNLIST))))
				return false;
			Info.SendDlgMessage(ThisState->hDlg, DM_GETCURSORPOS, ID, &Pos);
			ThisState->pos=Pos.X;
			//data
			size_t length=Info.SendDlgMessage(ThisState->hDlg,DM_GETTEXTLENGTH,ID,0);
			if(!length)
				ThisState->line=L"";
			else
			{
				LPTSTR buffer=new TCHAR[length+1];
				Info.SendDlgMessage(ThisState->hDlg, DM_GETTEXTPTR, ID, buffer);
				ThisState->line=buffer;
				delete[] buffer;
			}
			//selection
			EditorSelect es;
			Info.SendDlgMessage(ThisState->hDlg, DM_GETSELECTION, ID, &es);
			ThisState->selstart=es.BlockStartPos;
			ThisState->selend=es.BlockStartPos+es.BlockWidth;
		}
		break;
	}
	return true;
}

void Commit(LPCWSTR line,size_t pos,int SelStart=-1,int SelEnd=0,bool redraw=true)
{
	switch(ThisState->Workspace)
	{
		case W_EDITOR:
		{
			EditorSetString ess={-1, static_cast<int>(wcslen(line)), line, 0};
			Info.EditorControl(-1, ECTL_SETSTRING, 0, &ess);
			EditorSelect es={BTYPE_STREAM,0,SelStart,SelEnd-SelStart,1};
			Info.EditorControl(-1,ECTL_SELECT,0,&es);
			EditorSetPosition esp={-1,(int)pos,-1,-1,-1,-1};
			Info.EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
			if(redraw)
			{
				Info.EditorControl(-1, ECTL_REDRAW,0,NULL);
				Info.Text(0,0,0,0);
			}
		}
		break;
//-------------------------------------------------------------------------
		case W_SHELL:
		{
			Info.PanelControl(PANEL_ACTIVE, FCTL_SETCMDLINE, 0, const_cast<wchar_t*>(line));
			Info.PanelControl(PANEL_ACTIVE, FCTL_SETCMDLINEPOS, static_cast<int>(pos), nullptr);
			CmdLineSelect cls={SelStart,SelEnd};
			Info.PanelControl(PANEL_ACTIVE, FCTL_SETCMDLINESELECTION, 0, &cls);
			
			if(redraw)
			{
				//Info.AdvControl(YacGuid, ACTL_COMMIT,0);
				Info.Text(0,0,0,0);
			}
		}
		break;
//-------------------------------------------------------------------------
		case W_DIALOG:
		{
			Info.SendDlgMessage(ThisState->hDlg, DM_SETTEXTPTR, ID, const_cast<wchar_t*>(line));
			EditorSelect es={BTYPE_STREAM,0,SelStart,SelEnd-SelStart,1};
			Info.SendDlgMessage(ThisState->hDlg, DM_SETSELECTION, ID, &es);
			COORD p={static_cast<short>(pos),0};
			Info.SendDlgMessage(ThisState->hDlg, DM_SETCURSORPOS, ID, &p);
			if(redraw)
			{
				Info.SendDlgMessage(ThisState->hDlg,DM_REDRAW,0,0);
				Info.Text(0,0,0,0);
			}
		}
		break;
	}
	SaveState();
}

bool CheckExt(LPCWSTR Str,LPCWSTR mask)
{
	Tokenizer t(mask,L";,");
	while(!t.IsEnd())
	{
		if(t.Next()==Str)
			return true;
	}
	return false;
}

bool CheckPathExt(LPCWSTR Str)
{
	string Pathext;
	apiExpandEnvironmentStrings(L"%PATHEXT%",Pathext);
	return CheckExt(Str,Pathext);
}

bool IsNamePathExt(LPCWSTR Str)
{
	string name=Str;
	size_t DotPos=0;
	if(!name.RPos(DotPos,L'.'))
		return false;
	name.LShift(DotPos);
	return CheckPathExt(name);
}

LPCWSTR GetExtension(LPCWSTR name)
{
	LPCWSTR Ptr=name;
	while(*Ptr)
		Ptr++;
	while(Ptr>name && Ptr[-1]!=L'.')
		Ptr--;
	return Ptr>name?Ptr:L"";
}

string GetName(string& name)
{
	string NameEntry=name;
	size_t DotPos=0;
	if(NameEntry.RPos(DotPos,L'.'))
		NameEntry.SetLength(DotPos);
	return NameEntry;
}

void StdErrMsg(DWORD Error=0)
{
	if(!SilentEnum)
	{
		DWORD LastError=ERROR_SUCCESS;
		if(Error)
		{
			LastError=GetLastError();
			SetLastError(Error);
		}
		LPCTSTR Msg[]={Info.GetMsg(YacGuid, MError),L"\x1"};
		Info.Message(YacGuid, nullptr, FMSG_WARNING|FMSG_ERRORTYPE|FMSG_MB_OK,0,Msg,ARRAYSIZE(Msg),0);
		if(Error)
		{
			SetLastError(LastError);
		}
	}
}

void PushMultiString(string& str)
{
	Tokenizer st(str,L" \t");
	st.ToEnd();
	while(!st.IsStart())
		SStack->push(st.Prev());
}

void Clean()
{
	if(GlobalContainer)
	{
		delete GlobalContainer;
		GlobalContainer=0;
	}
}
/*
void AddOptSpace(string& str)
{
	if(Opt.AddSpace&&(CheckIsDir(ResumeData->result)!=1))
		final+=L" ";
}
*/

bool CheckCompletion(UINT Type)
{
	
	DWORD Cmp=LocalCompletion;
	if(ResumeData->prefix.Empty()) // сие - команда
		switch(ThisState->Workspace)
		{
		case W_SHELL:
			Cmp=Opt.Cmd.CmdCompletion;
			break;
		case W_EDITOR:
			Cmp=Opt.Edit.CmdCompletion;
			break;
		case W_DIALOG:
			Cmp=Opt.Dlg.CmdCompletion;
			break;
		}
		return (Cmp&CmplRules[Type].Value)!=0;
}

DWORD printfW(LPCWSTR fmt,...)
{
	va_list vl;
	va_start(vl,fmt);
	string str;
	str.vFormat(fmt,vl);
	va_end(vl);
	DWORD n=0;
	Info.PanelControl(PANEL_ACTIVE, FCTL_GETUSERSCREEN, 0, nullptr);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),str,(DWORD)str.GetLength(),&n,NULL);
	Info.PanelControl(PANEL_ACTIVE, FCTL_SETUSERSCREEN, 0, nullptr);
	return n;
}
