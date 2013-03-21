#define DM_SETFILTER DM_USER+1
#define C_DLG_LIST 0

bool NeedHint=false;
DWORD TickCount=NULL;

string FormHint(HANDLE hParent)
{
	int pos=static_cast<int>(Info.SendDlgMessage(hParent,DM_LISTGETDATA,C_DLG_LIST,(void*)Info.SendDlgMessage(hParent,DM_LISTGETCURPOS,0,0)));
	string ret;
	//if(GlobalContainer->Item[pos]->Type>=T_FIL && GlobalContainer->Item[pos]->Type<T_NMS)
	{
		int Num=CMPNUM(GlobalContainer->Item[pos]->Type);
		if(Num!=-1)
			ret+=GetMsg(MArea);
		if(GlobalContainer->Item[pos]->Type&0x10000000)
		{
			bool AnyAreaAdded=false;
			for(int i=0;i<ARRAYSIZE(CmplRules);i++)
			{
				if(GlobalContainer->Item[pos]->Type&CmplRules[i].Value)
				{
					if(AnyAreaAdded)
						ret+=L", ";
					Num=CMPNUM(CmplRules[i].Value);
					if(Num!=-1)
					{
						ret+=GetMsg(MComplFilenames+Num);
						AnyAreaAdded=true;
					}
				}
			}
		}
		else
		{
			if(Num!=-1)
				ret+=GetMsg(MComplFilenames+Num);

			if(!GlobalContainer->Item[pos]->Data1.Empty())
			{
				ret+=L"\n";
				ret+=GlobalContainer->Item[pos]->Data1;
			}
			//ret+=L" ";
		}
	}
	return ret;
}

struct POSTMSG
{
	int Msg,Param1;
	void* Param2;
	bool Key,Mouse;
	MOUSE_EVENT_RECORD r;
}
PostMsg/*={0,0,0,false}*/;

INT_PTR WINAPI HintProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if(!PostMsg.Key && !PostMsg.Mouse)
	{
		PostMsg.Msg=Msg;
		PostMsg.Param1=Param1;
		PostMsg.Param2=Param2;
		PostMsg.Key=false;
		PostMsg.Mouse=false;
	}

	switch(Msg)
	{
	case DN_INITDIALOG:
		Info.SendDlgMessage(hDlg,DM_SETMOUSEEVENTNOTIFY,1,0);
		break;
	case DN_CONTROLINPUT:
		{
			PINPUT_RECORD r = reinterpret_cast<PINPUT_RECORD>(Param2);
			if(r->EventType == KEY_EVENT)
			{
				PostMsg.Key=true;
				Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
				return TRUE;
			}
			else if(r->EventType == MOUSE_EVENT && r->Event.MouseEvent.dwButtonState)
			{
				PostMsg.Mouse=true;
				PostMsg.r=*reinterpret_cast<PMOUSE_EVENT_RECORD>(Param2);
				Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
				return TRUE;
			}
		}
		break;
	case DN_RESIZECONSOLE:
		Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
		break;
	case DN_DRAGGED:
		return FALSE;
	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CreateHint(HANDLE hParent)
{
	FarListInfo li;
	Info.SendDlgMessage(hParent,DM_LISTINFO,0,&li);
	
	string HintMsg=FormHint(hParent);

	if(HintMsg.Empty())
		return;

	SMALL_RECT sr;
	Info.SendDlgMessage(hParent,DM_GETDLGRECT,0,&sr);
	SMALL_RECT Rect;
	Info.AdvControl(YacGuid, ACTL_GETFARRECT, 0, &Rect);
	
	int LeftSpace=sr.Left;
	int RightSpace=(Rect.Right-Rect.Left+1)-sr.Right;
	int HintX=0,HintY=0,HintW=0,HintH=0;


	HintW=static_cast<int>(HintMsg.GetLength());

	bool ToRight=(RightSpace>HintW);
	if(!ToRight)
	{
		ToRight=(RightSpace>LeftSpace);
	}
	if(ToRight)
	{
		HintX=sr.Right+2;
		HintW=Min(HintW,RightSpace-3);
	}
	else
	{
		HintX=Max(sr.Left-HintW-1,1);
		HintW=Min(HintW,LeftSpace-3);
	}
	if(!HintW)
	{
		return;
	}
	Tokenizer t(HintMsg,L"\n");

	int MaxHintTextWidth=0;
	while(!t.IsEnd())
	{
		string line=t.Next();
		MaxHintTextWidth=Max(MaxHintTextWidth,(int)line.GetLength());
		HintH+=static_cast<int>(((line.GetLength()/HintW)+((line.GetLength()%HintW)?1:0)));
	}
	HintW=Min(MaxHintTextWidth,HintW);

	// recalc
	if(!ToRight)
	{
		HintX=sr.Left-HintW-1;
	}

	if(HintW<2)
	{
		return;
	}

	ToRight=(RightSpace>HintW);
	if(!ToRight)
		ToRight=(RightSpace>LeftSpace);

	if(ToRight)
	{
		HintX=sr.Right+2;
		HintW=Min(HintW,RightSpace-3);
	}
	else
	{
		HintX=Max(sr.Left-HintW-1,1);
		HintW=Min(HintW,LeftSpace-3);
	}

	HintY=Min(sr.Top+li.SelectPos-li.TopPos+1,(Rect.Bottom-Rect.Top+1)-HintH-1);

	size_t sz=HintW*HintH;

	PCHAR_INFO vBuf=new CHAR_INFO[sz];

	for(size_t i=0;i<sz;i++)
	{
		vBuf[i].Attributes=BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_INTENSITY;
		vBuf[i].Char.UnicodeChar=L' ';
	}
	t.ToStart();
	for(size_t h=0;!t.IsEnd();)
	{
		string line=t.Next();
		size_t limit=Min(sz,line.GetLength());
		for(size_t i=0;i<limit;i++)
		{
			if(i+h<sz)
			{
				vBuf[i+h].Char.UnicodeChar=line.At(i);
			}
		}
		h+=(line.GetLength()/HintW+((line.GetLength()%HintW)?1:0))*HintW;
	}

	FarDialogItem DialogItems[]=
	{
		{DI_USERCONTROL,0,0,HintW-1,HintH-1,1,(INT_PTR)vBuf,0,1,L""},
	};

	HANDLE hDlg=Info.DialogInit(YacGuid, nullptr, HintX,HintY,HintX+HintW-1,HintY+HintH-1,NULL,DialogItems,ARRAYSIZE(DialogItems),0,FDLG_SMALLDIALOG,HintProc,0);
	int res=Info.DialogRun(hDlg);
	Info.DialogFree(hDlg);
	delete[] vBuf;
}

INT_PTR WINAPI CmpResultsDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if(Opt.Hints)
	{
		NeedHint=(GetTickCount()-TickCount)>200;
	}

	switch(Msg)
	{
	case DN_INITDIALOG:
	{
		if(Opt.Hints)
		{
			TickCount=GetTickCount();
		}
		for(int i=0;i<GlobalContainer->Count();i++)
		{
			FarListItemData data={i,sizeof(DWORD),reinterpret_cast<PVOID>(static_cast<INT_PTR>(i))};
			Info.SendDlgMessage(hDlg,DM_LISTSETDATA,C_DLG_LIST,&data);
		}
		NeedMove=false;
		PrevCount=0;
		*DlgModStr=(const wchar_t*)Param2;
		startlen=DlgModStr->GetLength();
		Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,&CmpDlgPos);
		SMALL_RECT Rect;
		Info.AdvControl(YacGuid, ACTL_GETFARRECT, 0, &Rect);
		VerticalOffset=(Rect.Bottom-Rect.Top+1)-CmpDlgPos.Bottom;
		FarListInfo fli;
		Info.SendDlgMessage(hDlg,DM_LISTINFO,0,&fli);
		Info.SendDlgMessage(hDlg,DM_SETFILTER,fli.ItemsNumber,DlgModStr);
	}
	break;

	case DN_CLOSE:
		{
			ResultDlgExitNum=static_cast<DWORD>(Info.SendDlgMessage(hDlg, DM_LISTGETDATA, 0, (void*)Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, 0)));
		}
		break;

	case DN_ENTERIDLE:
		if(Opt.Hints)
		{
			if(NeedHint)
			{
				CreateHint(hDlg);
				TickCount=GetTickCount();
			}
			if(PostMsg.Key && PostMsg.Msg==DN_KEY)
			{
/*
struct MacroSendMacroText
{
	size_t StructSize;
	FARKEYMACROFLAGS Flags;
	INPUT_RECORD AKey;
	const wchar_t *SequenceText;
};

*/
				FARMACROSTATE State = static_cast<FARMACROSTATE>(Info.MacroControl(nullptr, MCTL_GETSTATE, MSSC_POST, nullptr));
				if(State != MACROSTATE_EXECUTING && State != MACROSTATE_EXECUTING_COMMON)
				{
					//BUGBUGBUG
					/*KeySequence ks={0,1,(DWORD*)&PostMsg.Param2};
					Info.AdvControl(Info.ModuleNumber,ACTL_POSTKEYSEQUENCE,&ks);
					PostMsg.Key=false;*/
				}
			}
			else if(PostMsg.Mouse)
			{
				DWORD n;
				INPUT_RECORD ir={MOUSE_EVENT};
				ir.Event.MouseEvent=PostMsg.r;
				WriteConsoleInputW(GetStdHandle(STD_INPUT_HANDLE),&ir,1,&n);
				PostMsg.Mouse=false;
			}
			break;

		}
		break;

	case DN_DRAGGED:
		{
			return FALSE;
		}
		break;

	case DN_CTLCOLORDLGLIST:
		if(!Param1)
		{
			if(!Opt.DlgColor)
			{
				return FALSE;
			}
			FarDialogItemColors* Colors=reinterpret_cast<FarDialogItemColors*>(Param2);
			
			int ColorIndex1[]={COL_MENUBOX, COL_MENUBOX, COL_MENUTITLE, COL_MENUTEXT, COL_MENUHIGHLIGHT, COL_MENUBOX, COL_MENUSELECTEDTEXT, COL_MENUSELECTEDHIGHLIGHT, COL_MENUSCROLLBAR, COL_MENUDISABLEDTEXT, COL_MENUARROWS, COL_MENUARROWSSELECTED, COL_MENUARROWSDISABLED};
			int ColorIndex2[]={COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINESELECTED, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINE, COL_COMMANDLINESELECTED, COL_COMMANDLINE};
			PINT ColorIndex=(Opt.DlgColor==1? ColorIndex1 : ColorIndex2);
			int Count=ARRAYSIZE(ColorIndex1);
			for(int i=0;i<Count;i++)
			{
				Info.AdvControl(YacGuid,ACTL_GETCOLOR,ColorIndex[i], &Colors->Colors[i]);
			}
			return TRUE;
		}
		break;

	case DM_SETFILTER:
		{
			string TitleStr,BottomStr;
			TitleStr.Format(L"[%d/%d]",Param1,GlobalContainer->Count());
			BottomStr.Format(L"[%s]",reinterpret_cast<string*>(Param2)->CPtr());
			FarListTitles flt={sizeof(flt), 0,TitleStr,0,BottomStr};
			Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,&flt);
		}
		break;

	case DN_KEY:
		{
			switch((int)Param2)
			{
			case KEY_TAB:
			case KEY_CTRLSPACE:
				{
					DWORD Keys[]={KEY_DOWN};
					Info.SendDlgMessage(hDlg,DN_KEY,ARRAYSIZE(Keys),reinterpret_cast<LONG_PTR>(Keys));
					return TRUE;
				}
				break;

			case KEY_SHIFTTAB:
			case KEY_CTRLSHIFTSPACE:
				{
					DWORD Keys[]={KEY_UP};
					Info.SendDlgMessage(hDlg,DN_KEY,ARRAYSIZE(Keys),reinterpret_cast<LONG_PTR>(Keys));
					return TRUE;
				}
				break;
			}
			if(((int)Param2<0x20||(((int)Param2>0xFFFF)&&!IsCharAlpha(reinterpret_cast<wchar_t>(Param2))))&&Param2!=KEY_BS)
				break;
			size_t l=DlgModStr->GetLength();

			bool IsErasing=false;

			if(Param2==KEY_BS)
			{
erase: // здесь и далее - страшное шаманство
				if(l==startlen)
					return FALSE;
				l--;
				DlgModStr->SetLength(l);
				IsErasing=true;
			}
			else // таки символ
			{
				wchar_t c[]={(wchar_t)(Param2),0};
				*DlgModStr+=c;
				l++;
			}
			if(Opt.SmartFilter)
			{
				int selcount=GlobalContainer->Select(*DlgModStr);
				if(selcount)
				{
					string out,maxmatch;
					GetMaxMatch(*DlgModStr,GlobalContainer->SelItems,selcount,out,maxmatch);
					if(!IsErasing)
					{
						*DlgModStr=out;
						Unquote(*DlgModStr);
						l=DlgModStr->GetLength();
					}
				}
				GlobalContainer->FreeSelect();
			}
			if(int count=GlobalContainer->Select(*DlgModStr))
			{
				if(count!=PrevCount)
				{
					PrevCount=count;
					if(ResultListItem)free(ResultListItem);
					ResultListItem=(FarListItem*)malloc(sizeof(FarListItem)*count);

					if(Opt.ShowType)
					{
						for(int i=0;i<count;i++)
						{
							ResultListItem[i].Flags=0;
							LPCWSTR type=(!(GlobalContainer->SelItems[i]->Type&0x80000000/*!=T_NMS*/))?
								(!(GlobalContainer->SelItems[i]->Type&0x10000000))?
								CmplRules[CMPNUM(GlobalContainer->SelItems[i]->Type)].name:L"***":L"---";
							ResultListItem[i].Text=ResultListInitItem[GlobalContainer->SelItems[i]->Number].Text;
						}
					}
					else
					{
						for(int i=0;i<count;i++)
						{
							ResultListItem[i].Flags=0;
							ResultListItem[count].Text=GlobalContainer->SelItems[i]->Name;
						}
					}
					FarList ListItems={count,ResultListItem};
					Info.SendDlgMessage(hDlg,DM_LISTSET,0,&ListItems);
					for(int i=0;i<count;i++)
					{
						FarListItemData data={i,sizeof(DWORD),GlobalContainer->SelItems[i]->Number};
						Info.SendDlgMessage(hDlg,DM_LISTSETDATA,C_DLG_LIST,&data);
					}
				}
				else
				{
					if(Opt.SmartFilter && l!=startlen)
					{
						goto erase;
					}
				}
				GlobalContainer->FreeSelect();
				Info.SendDlgMessage(hDlg,DM_SETFILTER,count,DlgModStr);
			}
			else
			{
				// а нэту таких
				l--;
				DlgModStr->SetLength(l);
				PlayBeep(Opt.Beep);
				return FALSE;
			}
			Info.SendDlgMessage(hDlg,DN_RESIZECONSOLE,0,0); //redraw dialog
		}
		break;
	case DN_RESIZECONSOLE:
		{
			FarListInfo fli = {sizeof(fli)};
			Info.SendDlgMessage(hDlg,DM_LISTINFO,0,&fli);
			int DlgX=GlobalContainer->MaxFarLen();
			if(HIWORD(Opt.MaxFilterSize) && (DWORD)DlgX>HIWORD(Opt.MaxFilterSize))
				DlgX=HIWORD(Opt.MaxFilterSize);
			DlgX+=Opt.ShowType?11:5;
			int DlgY=fli.ItemsNumber+2;
			if(LOWORD(Opt.MaxFilterSize) && (DWORD)DlgY>LOWORD(Opt.MaxFilterSize))
				DlgY=LOWORD(Opt.MaxFilterSize);
			SMALL_RECT Rect;
			Info.AdvControl(YacGuid, ACTL_GETFARRECT, 0, &Rect);
			COORD CPos;
			Info.AdvControl(YacGuid, ACTL_GETCURSORPOS, 0, &CPos);
			switch(ThisState->Workspace)
			{
			case W_SHELL:
			{
				if(DlgX>(Rect.Right-Rect.Left+1)-4)
				{
					DlgX=(Rect.Right-Rect.Left+1)-4;
				}
				if(DlgY>(Rect.Bottom-Rect.Top+1)-3)
				{
					DlgY=(Rect.Bottom-Rect.Top+1)-3;
				}
				NewCmpDlgPos.X=CPos.X-Rect.Left;
				NewCmpDlgPos.Y=(Rect.Bottom-Rect.Top+1)-VerticalOffset-DlgY+1;;
			}
			break;
			case W_DIALOG:
			case W_EDITOR:
			{
				DlgX=Min(DlgX,(Rect.Right-Rect.Left+1)-4);
				DlgY=Min(DlgY,(Rect.Bottom-Rect.Top+1)-4);

				NewCmpDlgPos.X=CPos.X-Rect.Top;

				int CurLine=CPos.Y;
				if(CurLine<(Rect.Bottom-Rect.Top+1)/2) //меню вниз
				{
					NewCmpDlgPos.Y=CurLine+1;
					DlgY=Min(DlgY,(Rect.Bottom-Rect.Top+1)-CurLine-2);
				}
				else //меню вверх
				{
					DlgY=Min(DlgY,CurLine-1);
					NewCmpDlgPos.Y=CurLine-DlgY;
				}
			}
			break;
			}
			COORD NewSize={DlgX,DlgY};
			SMALL_RECT NewListSize={0,0,DlgX-1,DlgY-1};
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,&NewSize);
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,&NewListSize);
			Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,&NewCmpDlgPos);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
			NeedMove=true;
			return TRUE;
		}
		break;

	case DN_DRAWDIALOGDONE:
		if(NeedMove)
		{
			NeedMove=false;
			Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,&NewCmpDlgPos);
		}
		break;
	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


bool ShowDlgMatches(string &result,string& maxmatch,bool StartFromEnd,bool &NoQuote)
{
	CmpMode=true;
	int count=GlobalContainer->Count();
	ResultListInitItem=(FarListItem*)malloc(count*sizeof(FarListItem));
	for(int i=0;i<count;i++)
	{
		if(Opt.ShowType)
		{
			LPCWSTR type=(!(GlobalContainer->Item[i]->Type&0x80000000/*!=T_NMS*/))?
				(!(GlobalContainer->Item[i]->Type&0x10000000))?
					CmplRules[CMPNUM(GlobalContainer->Item[i]->Type)].name:L"***":L"---";

			string Text;
			Text.Format(L"[%.3s]\x2502%s",type,GlobalContainer->Item[i]->Name.CPtr());
			wchar_t* lpwszText=new wchar_t[Text.GetLength()+1];
			lstrcpyW(lpwszText,Text);
			ResultListInitItem[i].Text=lpwszText;
		}
		else
			ResultListInitItem[i].Text=GlobalContainer->Item[i]->Name;
		ResultListInitItem[i].Flags=0;
	}
	ResultListInitItem[StartFromEnd?count-1:0].Flags=LIF_SELECTED;
	FarList ListItems;
	ListItems.Items=ResultListInitItem;
	ListItems.ItemsNumber=count;

	int X1=0,X2=0,Y1=0,Y2=0;
	
	int DlgX=GlobalContainer->MaxFarLen();
	if(HIWORD(Opt.MaxFilterSize) && (DWORD)DlgX>HIWORD(Opt.MaxFilterSize))
		DlgX=HIWORD(Opt.MaxFilterSize);
	DlgX+=Opt.ShowType?10:4;
	int DlgY=ListItems.ItemsNumber+1;
	if(LOWORD(Opt.MaxFilterSize) && (DWORD)DlgY>LOWORD(Opt.MaxFilterSize))
		DlgY=LOWORD(Opt.MaxFilterSize);

	SMALL_RECT Rect;
	Info.AdvControl(YacGuid, ACTL_GETFARRECT, 0, &Rect);
	COORD CPos;
	Info.AdvControl(YacGuid, ACTL_GETCURSORPOS, 0, &CPos);
	switch(ThisState->Workspace)
	{
	case W_SHELL:
		{
			DlgX=Min(DlgX,(Rect.Right-Rect.Left+1)-4);
			DlgY=Min(DlgY,(Rect.Bottom-Rect.Top+1)-4);
			X1=CPos.X;
			Y1=CPos.Y-DlgY-1;
		}
		break;
	case W_DIALOG:
	case W_EDITOR:
		{
			if(DlgX>(Rect.Right-Rect.Left+1)-4)DlgX=(Rect.Right-Rect.Left+1)-4;
			X1=CPos.X;
			int CurLine=CPos.Y;
			if(CurLine<(Rect.Bottom-Rect.Top+1)/2) //меню вниз
			{
				Y1=CurLine+1;
				DlgY=Min(DlgY,(Rect.Bottom-Rect.Top+1)-CurLine-3);
			}
			else //меню вверх
			{
				DlgY=Min(DlgY,CurLine-2);
				Y1=CurLine-DlgY-1;
			}
		}
		break;
	}
	X2=X1+DlgX;
	Y2=Y1+DlgY;
	FarDialogItem DialogItems[]=
	{
		{DI_LISTBOX,0,0,DlgX,DlgY,(INT_PTR)&ListItems,nullptr,nullptr,DIF_LISTWRAPMODE|DIF_DEFAULTBUTTON|DIF_FOCUS,L""},
	};

	bool ret=false;
	DlgModStr=new string;
	Info.AdvControl(YacGuid,ACTL_COMMIT,0,0);
	HANDLE hDlg=Info.DialogInit(YacGuid,nullptr,X1,Y1,X2,Y2,L"CmpResultsDlg",DialogItems,ARRAYSIZE(DialogItems),0,0,CmpResultsDlgProc,(void*)maxmatch.CPtr());
	int res=Info.DialogRun(hDlg);
	if(res==C_DLG_LIST)
	{
		string Append=GlobalContainer->Item[ResultDlgExitNum]->Name;
		if(Append.Equal(maxmatch,maxmatch.GetLength()))
			result.SetLength(result.GetLength()-maxmatch.GetLength());
		result+=Append;
		NoQuote=(GlobalContainer->Item[ResultDlgExitNum]->Type==CMP(T_HST));
		ret=true;
	}
	//ListItems
	Info.DialogFree(hDlg);
	for(size_t i=0;i<ListItems.ItemsNumber;i++)
	{
		delete[] ListItems.Items[i].Text;
	}
	delete DlgModStr;
	free(ResultListInitItem);
	if(ResultListItem)
	{
		free(ResultListItem);
		ResultListItem=NULL;
	}
	CmpMode=false;
	return ret;
}
