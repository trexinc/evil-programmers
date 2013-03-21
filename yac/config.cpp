#define CMPDLGX 52
#define CMPDLGY 23

#define CFGDLGX 50
#define CFGDLGY 14

#define DM_GETCMP DM_USER+1
#define DM_SETCMP DM_USER+2
#define DM_SHOWDUMP DM_USER+3

bool MoveDown=true;
int CmdType=0,Area=0;

void ReadConfig()
{
	Opt.Cmd.CmdCompletion=GetRegKey(PluginKey,RegStr.CmdCmdCompletion,
		//0xffffffff
		CMP(T_FIL)|CMP(T_PTH)|CMP(T_SRV)|CMP(T_SHR)|CMP(T_ENV)|CMP(T_DRV)|CMP(T_ADS)|CMP(T_VOL)|CMP(T_CMD)|CMP(T_ALS)|CMP(T_SAL)|CMP(T_PRF)|CMP(T_KEY)|CMP(T_HST)
	);
	Opt.Cmd.ParamCompletion=GetRegKey(PluginKey,RegStr.CmdParamCompletion,
		//0xffffffff
		CMP(T_FIL)|CMP(T_PTH)|CMP(T_SRV)|CMP(T_SHR)|CMP(T_ENV)|CMP(T_DRV)|CMP(T_ADS)|CMP(T_REG)|CMP(T_PIP)|CMP(T_VOL)|CMP(T_PRC)|CMP(T_SVC)|CMP(T_CMD)|CMP(T_KEY)|CMP(T_USR)|CMP(T_PNL)
	);
	Opt.Edit.CmdCompletion=GetRegKey(PluginKey,RegStr.EditCmdCompletion,
		//0xffffffff
		CMP(T_FIL)|CMP(T_PTH)|CMP(T_SRV)|CMP(T_SHR)|CMP(T_ENV)|CMP(T_DRV)|CMP(T_ADS)|CMP(T_VOL)|CMP(T_CMD)|CMP(T_ALS)|CMP(T_SAL)|CMP(T_KEY)|CMP(T_USR)|CMP(T_PNL)
	);
	Opt.Edit.ParamCompletion=GetRegKey(PluginKey,RegStr.EditParamCompletion,
		//0xffffffff
		CMP(T_FIL)|CMP(T_PTH)|CMP(T_SRV)|CMP(T_SHR)|CMP(T_ENV)|CMP(T_DRV)|CMP(T_ADS)|CMP(T_REG)|CMP(T_PIP)|CMP(T_VOL)|CMP(T_PRC)|CMP(T_SVC)|CMP(T_CMD)|CMP(T_KEY)|CMP(T_USR)|CMP(T_PNL)
	);
	Opt.Dlg.CmdCompletion=GetRegKey(PluginKey,RegStr.DlgCmdCompletion,
		//0xffffffff
		CMP(T_FIL)|CMP(T_PTH)|CMP(T_SRV)|CMP(T_SHR)|CMP(T_ENV)|CMP(T_DRV)|CMP(T_ADS)|CMP(T_REG)|CMP(T_PIP)|CMP(T_VOL)|CMP(T_PRC)|CMP(T_SVC)|CMP(T_CMD)|CMP(T_ALS)|CMP(T_SAL)|CMP(T_PRF)|CMP(T_KEY)|CMP(T_USR)|CMP(T_PNL)|CMP(T_HST)
	);
	Opt.Dlg.ParamCompletion=GetRegKey(PluginKey,RegStr.DlgParamCompletion,
		//0xffffffff
		CMP(T_FIL)|CMP(T_PTH)|CMP(T_SRV)|CMP(T_SHR)|CMP(T_ENV)|CMP(T_DRV)|CMP(T_ADS)|CMP(T_REG)|CMP(T_PIP)|CMP(T_VOL)|CMP(T_PRC)|CMP(T_SVC)|CMP(T_CMD)|CMP(T_ALS)|CMP(T_SAL)|CMP(T_PRF)|CMP(T_KEY)|CMP(T_USR)|CMP(T_PNL)|CMP(T_HST)
	);

	//Opt.Completion=GetRegKey(PluginKey,RegStr.Completion,0xffffffff);
	Opt.ShowType=GetRegKey(PluginKey,RegStr.ShowType,1);
	Opt.Multicolumn=GetRegKey(PluginKey,RegStr.Multicolumn,1);
	Opt.Pausable=GetRegKey(PluginKey,RegStr.Pausable,1);
	Opt.MaxFilterSize=GetRegKey(PluginKey,RegStr.MaxFilterSize,0);
	Opt.SmartFilter=GetRegKey(PluginKey,RegStr.SmartFilter,1);
	Opt.Hints=GetRegKey(PluginKey,RegStr.Hints,1);
	Opt.ShowResults=GetRegKey(PluginKey,RegStr.ShowResults,1);
	Opt.Beep=GetRegKey(PluginKey,RegStr.Beep,0x01000080);
	Opt.AddEndSlash=GetRegKey(PluginKey,RegStr.AddEndSlash,0xffffffff);
	Opt.AddSpace=GetRegKey(PluginKey,RegStr.AddSpace,1);
	Opt.SaveTail=GetRegKey(PluginKey,RegStr.SaveTail,1);
	Opt.Preload=GetRegKey(PluginKey,RegStr.Preload,1);
	Opt.UsePathExt=GetRegKey(PluginKey,RegStr.UsePathExt,1);
	Opt.DlgColor=(BYTE)GetRegKey(PluginKey,RegStr.DlgColor,1);
	GetRegKey(PluginKey,RegStr.DirCmds,*Opt.DirCmds,L"cd;chdir;md;mkdir;rd;rmdir;dir;ls");
	GetRegKey(PluginKey,RegStr.Prefix,*Opt.Prefix,L"YAC");
}

void WriteConfig()
{
	//SetRegKey(PluginKey,RegStr.Completion,Opt.Completion);
	
	SetRegKey(PluginKey,RegStr.CmdCmdCompletion,Opt.Cmd.CmdCompletion);
	SetRegKey(PluginKey,RegStr.CmdParamCompletion,Opt.Cmd.ParamCompletion);
	SetRegKey(PluginKey,RegStr.EditCmdCompletion,Opt.Edit.CmdCompletion);
	SetRegKey(PluginKey,RegStr.EditParamCompletion,Opt.Edit.ParamCompletion);
	SetRegKey(PluginKey,RegStr.DlgCmdCompletion,Opt.Dlg.CmdCompletion);
	SetRegKey(PluginKey,RegStr.DlgParamCompletion,Opt.Dlg.ParamCompletion);

	SetRegKey(PluginKey,RegStr.ShowType,Opt.ShowType);
	SetRegKey(PluginKey,RegStr.Multicolumn,Opt.Multicolumn);
	SetRegKey(PluginKey,RegStr.Pausable,Opt.Pausable);
	SetRegKey(PluginKey,RegStr.MaxFilterSize,Opt.MaxFilterSize);
	SetRegKey(PluginKey,RegStr.SmartFilter,Opt.SmartFilter);
	SetRegKey(PluginKey,RegStr.Hints,Opt.Hints);
	SetRegKey(PluginKey,RegStr.ShowResults,Opt.ShowResults);
	SetRegKey(PluginKey,RegStr.Beep,Opt.Beep);
	SetRegKey(PluginKey,RegStr.AddEndSlash,Opt.AddEndSlash);
	SetRegKey(PluginKey,RegStr.AddSpace,Opt.AddSpace);
	SetRegKey(PluginKey,RegStr.SaveTail,Opt.SaveTail);
	SetRegKey(PluginKey,RegStr.Preload,Opt.Preload);
	SetRegKey(PluginKey,RegStr.UsePathExt,Opt.UsePathExt);
	SetRegKey(PluginKey,RegStr.DlgColor,Opt.DlgColor);
	SetRegKey(PluginKey,RegStr.DirCmds,*Opt.DirCmds);
	SetRegKey(PluginKey,RegStr.Prefix,*Opt.Prefix);
}


#if 1
enum
{
	CMPCFGDLG_BORDER,
	CMPCFGDLG_LABELWHAT,
	CMPCFGDLG_RADIOCMD,
	CMPCFGDLG_RADIOPARAM,
	CMPCFGDLG_SEPARATOR1,
	CMPCFGDLG_LABELWHERE,
	CMPCFGDLG_RADIOCMDLINE,
	CMPCFGDLG_RADIOEDITOR,
	CMPCFGDLG_RADIODIALOG,
	CMPCFGDLG_SEPARATOR2,
	CMPCFGDLG_OPTLIST,
	CMPCFGDLG_SEPARATOR3,
	CMPCFGDLG_OK,
	CMPCFGDLG_CANCEL,
	CMPCFGDLG_STATUS,
};

int NewCompletion;

INT_PTR WINAPI OptCompletionDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch(Msg)
	{
	case DN_INITDIALOG:
		{
			Info.SendDlgMessage(hDlg,DM_SETMOUSEEVENTNOTIFY,TRUE,0);
		}
		break;

	case DN_RESIZECONSOLE:
		{
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			int ItemsCount=ARRAYSIZE(CmplRules);
			SMALL_RECT Rect;
			Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,&Rect);
			COORD DlgCoord={Rect.Right-Rect.Left+1,ItemsCount+10};
			PCOORD c=reinterpret_cast<PCOORD>(Param2);
			DlgCoord.Y=Min(DlgCoord.Y,static_cast<short>(c->Y-2));
			Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,1,&DlgCoord);
			COORD center={-1,-1};
			Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,1,&center);
			SMALL_RECT ItemPos={3,1,CMPDLGX-4,DlgCoord.Y-2};
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,CMPCFGDLG_BORDER,&ItemPos);
			ItemPos.Left=4;ItemPos.Top=6;ItemPos.Right=CMPDLGX-5;ItemPos.Bottom=DlgCoord.Y-5;
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,CMPCFGDLG_OPTLIST,&ItemPos);
			ItemPos.Left=-1;ItemPos.Top=DlgCoord.Y-4;ItemPos.Right=-1;ItemPos.Bottom=DlgCoord.Y-4;
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,CMPCFGDLG_SEPARATOR3,&ItemPos);
			ItemPos.Left=0;ItemPos.Top=DlgCoord.Y-3;ItemPos.Right=0;ItemPos.Bottom=DlgCoord.Y-3;
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,CMPCFGDLG_OK,&ItemPos);
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,CMPCFGDLG_CANCEL,&ItemPos);
			ItemPos.Left=0;ItemPos.Top=DlgCoord.Y-1;ItemPos.Right=0;ItemPos.Bottom=DlgCoord.Y-1;
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,CMPCFGDLG_STATUS,&ItemPos);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
		}
		break;

	case DN_CLOSE:
		switch(Param1)
		{
		case CMPCFGDLG_OK:
			NewCompletion=0;
			for(UINT i=0;i<ARRAYSIZE(CmplRules);i++)
			{
				FarListGetItem item;
				item.ItemIndex=i;
				Info.SendDlgMessage(hDlg, DM_LISTGETITEM, CMPCFGDLG_OPTLIST, &item);
				NewCompletion|=CmplRules[i].Value*(LIF_CHECKED==(item.Item.Flags&LIF_CHECKED));
			}
			return TRUE;

		case CMPCFGDLG_OPTLIST:
			{
				//hack :-)
				Msg=DN_KEY;
				Param2=KEY_SPACE;
				MoveDown=false;
			}
		}

	case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* p = static_cast<const INPUT_RECORD*>(Param2);
			bool Key = p->EventType == KEY_EVENT;
			if(Key)
			{
				if(p->Event.KeyEvent.wVirtualKeyCode == 'H' && IsCtrl(p->Event.KeyEvent.dwControlKeyState))
				{
					Info.SendDlgMessage(hDlg,DM_SHOWITEM,CMPCFGDLG_STATUS,(void*)!Info.SendDlgMessage(hDlg,DM_SHOWITEM,CMPCFGDLG_STATUS,(void*)-1));
					Info.SendDlgMessage(hDlg,DM_SHOWDUMP,0,(void*)Info.SendDlgMessage(hDlg,DM_GETCMP,0,0));
					break;
				}

				switch(Param1)
				{
				case CMPCFGDLG_OPTLIST:
					{
						//if(Param2==KEY_CTRLENTER)
							//Info.SendDlgMessage(hDlg,DM_CLOSE,CMPCFGDLG_OK,0);
						FarListGetItem item={(int)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,CMPCFGDLG_OPTLIST,0)};
						Info.SendDlgMessage(hDlg,DM_LISTGETITEM,CMPCFGDLG_OPTLIST,&item);
						switch((int)Param2)
						{
						case KEY_ADD:
							item.Item.Flags=LIF_CHECKED;
							break;
						case KEY_SUBTRACT:
							item.Item.Flags=0;
							break;
						case KEY_SPACE:
						case KEY_INS:
							item.Item.Flags=item.Item.Flags&LIF_CHECKED?0:LIF_CHECKED;
							if(item.ItemIndex!=ARRAYSIZE(CmplRules)-1 && MoveDown)
							{
								DWORD Keys[]={KEY_DOWN};
								KeySequence ks={KSFLAGS_DISABLEOUTPUT,ARRAYSIZE(Keys),Keys};
								Info.AdvControl(Info.ModuleNumber,ACTL_POSTKEYSEQUENCE,&ks);
							}
							if(!MoveDown)
							{
								MoveDown=true;
							}
							break;
						}
						Info.SendDlgMessage(hDlg,DM_LISTUPDATE,CMPCFGDLG_OPTLIST,&item);

						// а какой из типов дополнения сейчас активен?
						int _CmdType=0;
						if(Info.SendDlgMessage(hDlg,DM_GETCHECK,CMPCFGDLG_RADIOPARAM,0)==BSTATE_CHECKED)
						{
							_CmdType=1;
						}
						int _Area=0;
						if(Info.SendDlgMessage(hDlg,DM_GETCHECK,CMPCFGDLG_RADIOEDITOR,0)==BSTATE_CHECKED)
						{
							_Area=1;
						}
						else
						{
							if(Info.SendDlgMessage(hDlg,DM_GETCHECK,CMPCFGDLG_RADIODIALOG,0)==BSTATE_CHECKED)
							{
								_Area=2;
							}
						}

						DWORD CurrentCompletion=static_cast<DWORD>(Info.SendDlgMessage(hDlg,DM_GETCMP,0,0));
						Info.SendDlgMessage(hDlg,DM_SHOWDUMP,0,(void*)CurrentCompletion);
						switch(_Area)
						{
						case 0: // cmdline
							if(!_CmdType)
								TmpOpt.Cmd.CmdCompletion=CurrentCompletion;
							else
								TmpOpt.Cmd.ParamCompletion=CurrentCompletion;
							break;
						case 1: // editor
							if(!_CmdType)
								TmpOpt.Edit.CmdCompletion=CurrentCompletion;
							else
								TmpOpt.Edit.ParamCompletion=CurrentCompletion;
							break;
						case 2: // dialog
							if(!_CmdType)
								TmpOpt.Dlg.CmdCompletion=CurrentCompletion;
							else
								TmpOpt.Dlg.ParamCompletion=CurrentCompletion;
							break;
						}
					}
					break;
				}
			}
			else if(p->EventType == MOUSE_EVENT && p->Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED && p->Event.MouseEvent.dwEventFlags&MOUSE_MOVED)
			{
				return FALSE;
			}
		}
		break;

	case DN_LISTHOTKEY:
		{
			FarListGetItem item={reinterpret_cast<int>(Param2)};
			Info.SendDlgMessage(hDlg,DM_LISTGETITEM,CMPCFGDLG_OPTLIST,&item);
			item.Item.Flags=item.Item.Flags&LIF_CHECKED?0:LIF_CHECKED;
			Info.SendDlgMessage(hDlg,DM_LISTUPDATE,CMPCFGDLG_OPTLIST,&item);
			return FALSE;
		}
	case DN_BTNCLICK:
		{
			switch(Param1)
			{
			case CMPCFGDLG_RADIOCMD:
			case CMPCFGDLG_RADIOPARAM:
			case CMPCFGDLG_RADIOCMDLINE:
			case CMPCFGDLG_RADIOEDITOR:
			case CMPCFGDLG_RADIODIALOG:
				{
					if(Param2)
					{
						switch(Param1)
						{
						case CMPCFGDLG_RADIOCMD:
							if((int)Param2==1)
								CmdType=0;
							break;
						case CMPCFGDLG_RADIOPARAM:
							if(int)(Param2==1)
								CmdType=1;
							break;
						case CMPCFGDLG_RADIOCMDLINE:
							if((int)Param2==1)
								Area=0;
							break;
						case CMPCFGDLG_RADIOEDITOR:
							if((int)Param2==1)
								Area=1;
							break;
						case CMPCFGDLG_RADIODIALOG:
							if((int)Param2==1)
								Area=2;
							break;
						}
						DWORD CurrentCompletion=0;
						switch(Area)
						{
						case 0: // cmdline
							CurrentCompletion=!CmdType?TmpOpt.Cmd.CmdCompletion:TmpOpt.Cmd.ParamCompletion;
							break;
						case 1: // editor
							CurrentCompletion=!CmdType?TmpOpt.Edit.CmdCompletion:TmpOpt.Edit.ParamCompletion;
							break;
						case 2: // dialog
							CurrentCompletion=!CmdType?TmpOpt.Dlg.CmdCompletion:TmpOpt.Dlg.ParamCompletion;
							break;
						}
						Info.SendDlgMessage(hDlg,DM_SETCMP,0,(void*)CurrentCompletion);
						Info.SendDlgMessage(hDlg,DM_SHOWDUMP,0,(void*)CurrentCompletion);
					}
				}
			}
		}
		break;
	case DM_SHOWDUMP:
		{
			string Hint;
			LPCWSTR CmpName=L"";
			switch(Area)
			{
			case 0:
				CmpName=CmdType?RegStr.CmdParamCompletion:RegStr.CmdCmdCompletion;
				break;
			case 1:
				CmpName=CmdType?RegStr.EditParamCompletion:RegStr.EditCmdCompletion;
				break;
			case 2:
				CmpName=CmdType?RegStr.DlgParamCompletion:RegStr.DlgCmdCompletion;
				break;
			}
			Hint.Format(L"%s=0x%08x",CmpName,reinterpret_cast<DWORD>(Param2));
			LPCTSTR status;
			status=Hint;
			Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,CMPCFGDLG_STATUS,&status);
		}
		break;
	case DM_GETCMP:
		{
			FarListInfo Li;
			Info.SendDlgMessage(hDlg,DM_LISTINFO,CMPCFGDLG_OPTLIST,&Li);
			DWORD CurrentCompletion=0;
			for(size_t i=0;i<Li.ItemsNumber;i++)
			{
				FarListGetItem GetItem={i};
				Info.SendDlgMessage(hDlg,DM_LISTGETITEM,CMPCFGDLG_OPTLIST,&GetItem);
				CurrentCompletion|=(GetItem.Item.Flags&LIF_CHECKED)?CMP(i):0;
			}
			return CurrentCompletion;
		}
	case DM_SETCMP:
		{
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			FarListInfo Li;
			Info.SendDlgMessage(hDlg,DM_LISTINFO,CMPCFGDLG_OPTLIST,&Li);
			for(size_t i=0;i<Li.ItemsNumber;i++)
			{
				FarListGetItem GetItem={i};
				Info.SendDlgMessage(hDlg,DM_LISTGETITEM,CMPCFGDLG_OPTLIST,&GetItem);
				GetItem.Item.Flags=CMP(i)&(reinterpret_cast<DWORD>(Param2))?LIF_CHECKED:0;
				Info.SendDlgMessage(hDlg,DM_LISTUPDATE,CMPCFGDLG_OPTLIST,&GetItem);
			}
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
		}
		break;
	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int DlgOptCompletion()
{
	TmpOpt.Cmd=Opt.Cmd;
	TmpOpt.Edit=Opt.Edit;
	TmpOpt.Dlg=Opt.Dlg;

	const int count=ARRAYSIZE(CmplRules);
	SMALL_RECT Rect;
	Info.AdvControl(YacGuid, ACTL_GETFARRECT, 0, &Rect);
	int Y=(count>(Rect.Bottom-Rect.Top+1)-10)?(Rect.Bottom-Rect.Top+1)-2:count+10;

	FarListItem items[count];
	for(int i=0;i<count;i++)
	{
		items[i].Text=GetMsg(MComplFilenames+i);
		items[i].Flags=(Opt.Cmd.CmdCompletion&CMP(i))?LIF_CHECKED:0;
	}
	FarList list={count,items};

	FarDialogItem DialogItems[]=
	{
		/*00*/{DI_DOUBLEBOX,  /**/3, /**/1,  /**/CMPDLGX-4,/**/Y-2,/**/0,             /**/nullptr, nullptr, 0,                                                                       /**/GetMsg(MCompl)},
		/*01*/{DI_TEXT,       /**/4, /**/2,  /**/4,        /**/2,  /**/0,             /**/nullptr, nullptr, DIF_FOCUS,                                                                       /**/GetMsg(MTypeLabel)},
		/*02*/{DI_RADIOBUTTON,/**/11,/**/2,  /**/11,       /**/2,  /**/1,             /**/nullptr, nullptr, DIF_GROUP,                                                               /**/GetMsg(MTypeCmd)},
		/*03*/{DI_RADIOBUTTON,/**/25,/**/2,  /**/25,       /**/2,  /**/0,             /**/nullptr, nullptr, 0,                                                                       /**/GetMsg(MTypeParam)},
		/*04*/{DI_TEXT,       /**/-1,/**/3,  /**/-1,       /**/3,  /**/0,             /**/nullptr, nullptr, DIF_BOXCOLOR|DIF_SEPARATOR,                                              /**/L""},
		/*05*/{DI_TEXT,       /**/4, /**/4,  /**/4,        /**/4,  /**/0,             /**/nullptr, nullptr, 0,                                                                       /**/GetMsg(MAreaLabel)},
		/*06*/{DI_RADIOBUTTON,/**/11,/**/4,  /**/11,       /**/4,  /**/1,             /**/nullptr, nullptr, DIF_GROUP,                                                               /**/GetMsg(MAreaCmd)},
		/*07*/{DI_RADIOBUTTON,/**/25,/**/4,  /**/25,       /**/4,  /**/0,             /**/nullptr, nullptr, 0,                                                                       /**/GetMsg(MAreaEditor)},
		/*08*/{DI_RADIOBUTTON,/**/38,/**/4,  /**/38,       /**/4,  /**/0,             /**/nullptr, nullptr, 0,                                                                       /**/GetMsg(MAreaDlg)},
		/*09*/{DI_TEXT,       /**/-1,/**/5,  /**/-1,       /**/5,  /**/0,             /**/nullptr, nullptr, DIF_BOXCOLOR|DIF_SEPARATOR,                                              /**/L""},
		/*10*/{DI_LISTBOX,    /**/4, /**/6,  /**/CMPDLGX-5,/**/Y-5,/**/(INT_PTR)&list,/**/nullptr, nullptr, DIF_LISTWRAPMODE|DIF_LISTNOBOX|DIF_LISTNOAMPERSAND|DIF_LISTAUTOHIGHLIGHT,/**/L""},
		/*11*/{DI_TEXT,       /**/-1,/**/Y-4,/**/-1,       /**/Y-4,/**/0,             /**/nullptr, nullptr, DIF_BOXCOLOR|DIF_SEPARATOR,                                              /**/L""},
		/*12*/{DI_BUTTON,     /**/0, /**/Y-3,/**/0,        /**/Y-3,/**/0,             /**/nullptr, nullptr, DIF_CENTERGROUP|DIF_DEFAULTBUTTON,                                       /**/GetMsg(MOK)},
		/*13*/{DI_BUTTON,     /**/0, /**/Y-3,/**/0,        /**/Y-3,/**/0,             /**/nullptr, nullptr, DIF_CENTERGROUP,                                                         /**/GetMsg(MCancel)},
		/*14*/{DI_TEXT,       /**/0, /**/Y-1,/**/0,        /**/Y-1,/**/0,             /**/nullptr, nullptr, DIF_HIDDEN,                                                              /**/L""},
	};
	HANDLE hDlg=Info.DialogInit(YacGuid, nullptr, -1,-1,CMPDLGX,Y,L"OptCompletionDlg",DialogItems,ARRAYSIZE(DialogItems),0,0,OptCompletionDlgProc,0);
	int Ret=Info.DialogRun(hDlg);
	Info.DialogFree(hDlg);
	if(Ret==CMPCFGDLG_OK)
	{
		Opt.Cmd=TmpOpt.Cmd;
		Opt.Edit=TmpOpt.Edit;
		Opt.Dlg=TmpOpt.Dlg;
	}
	return Ret==CMPCFGDLG_OK;
}
#endif
enum
{
	DLG_BORDER,
	/*DLG_AREALABEL,
	DLG_AREA,
	DLG_CMDTYPELABEL,
	DLG_CMDTYPE,
	DLG_COMPLETIONLABEL,
	DLG_COMPLETION,
	DLG_SEPARATOR0,*/
	DLG_SHOW,
	DLG_SHOWRESULTS,
	DLG_SHOWTYPE,
	DLG_ADDSPACE,
	DLG_SAVETAIL,
	DLG_DIRCMDSTEXT,
	DLG_DIRCMDSDATA,
	DLG_SEPARATOR1,
	DLG_BEEPCHECK,
	DLG_BEEPCOMBO,
	DLG_BEEPHZTXT,
	DLG_BEEPHZ,
	DLG_BEEPMSTXT,
	DLG_BEEPMS,
	DLG_BEEPTEST,
	DLG_SEPARATOR2,
	//DLG_PREFIXTEXT,
	//DLG_PREFIXDATA,
	//DLG_SEPARATOR3,
	DLG_OK,
	DLG_CANCEL,
};

void EditDlg()
{
	enum EDITEMS
	{
		ED_BORDER,
		ED_COLORTEXT,
		ED_COLORCOMBO,
		ED_SMARTFILTER,
		ED_HINTS,
		ED_SIZETEXT,
		ED_SIZEX,
		ED_SIZEXEDIT,
		ED_SIZEY,
		ED_SIZEYEDIT,
		ED_SEPATATOR,
		ED_OK,
		ED_CANCEL,
	};

	int DX=40,DY=10;

	TCHAR MaxWidth[4],MaxHeight[4];
	FSF.itoa(HIWORD(TmpOpt.MaxFilterSize),MaxWidth,10);
	FSF.itoa(LOWORD(TmpOpt.MaxFilterSize),MaxHeight,10);

	FarDialogItem DialogItems[]=
	{
		{DI_DOUBLEBOX,/**/3,    /**/1,   /**/DX-4,/**/DY-2,/**/0,                 nullptr, nullptr, /**/0,                                                                          /**/GetMsg(MList)},
		{DI_TEXT,     /**/5,    /**/2,   /**/5,   /**/2,   /**/0,                 nullptr, nullptr, /**/0,                                                                          /**/GetMsg(MColorScheme)},
		{DI_COMBOBOX, /**/DX-19,/**/2,   /**/DX-6,/**/2,   /**/0,                 nullptr, nullptr, /**/DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,/**/L""},
		{DI_CHECKBOX, /**/5,    /**/3,   /**/5,   /**/3,   /**/TmpOpt.SmartFilter,nullptr, nullptr, /**/0,                                                                          /**/GetMsg(MSmartFilter)},
		{DI_CHECKBOX, /**/5,    /**/4,   /**/5,   /**/4,   /**/TmpOpt.Hints,      nullptr, nullptr, /**/0,                                                                          /**/GetMsg(MHints)},
		{DI_TEXT,     /**/5,    /**/5,   /**/5,   /**/5,   /**/0,                 nullptr, nullptr, /**/0,                                                                          /**/GetMsg(MMaxFilterSize)},
		{DI_TEXT,     /**/18,   /**/5,   /**/20,  /**/5,   /**/0,                 nullptr, nullptr, /**/0,                                                                          /**/L"&X:"},
		{DI_FIXEDIT,  /**/21,   /**/5,   /**/23,  /**/5,   /**/0,                 nullptr, nullptr, /**/0,                                                                          /**/MaxWidth},
		{DI_TEXT,     /**/29,   /**/5,   /**/31,  /**/5,   /**/0,                 nullptr, nullptr, /**/0,                                                                          /**/L"&Y:"},
		{DI_FIXEDIT,  /**/32,   /**/5,   /**/34,  /**/5,   /**/0,                 nullptr, nullptr, /**/0,                                                                          /**/MaxWidth},
		{DI_TEXT,     /**/-1,   /**/DY-4,/**/-1,  /**/DY-4,/**/0,                 nullptr, nullptr, /**/DIF_BOXCOLOR|DIF_SEPARATOR,                                                 /**/L""},
		{DI_BUTTON,   /**/0,    /**/DY-3,/**/0,   /**/DY-3,/**/0,                 nullptr, nullptr, /**/DIF_CENTERGROUP|DIF_DEFAULTBUTTON,                                          /**/GetMsg(MOK)},
		{DI_BUTTON,   /**/0,    /**/DY-3,/**/0,   /**/DY-3,/**/0,                 nullptr, nullptr, /**/DIF_CENTERGROUP,                                                            /**/GetMsg(MCancel)},
	};

	FarListItem ColorComboItems[3]={0};
	ColorComboItems[0].Text=GetMsg(MDlgScheme);
	ColorComboItems[1].Text=GetMsg(MMenuScheme);
	ColorComboItems[2].Text=GetMsg(MCmdScheme);
	ColorComboItems[TmpOpt.DlgColor].Flags=LIF_SELECTED;
	FarList ColorComboList={ARRAYSIZE(ColorComboItems),ColorComboItems};
	DialogItems[ED_COLORCOMBO].ListItems=&ColorComboList;

	HANDLE hDlg=Info.DialogInit(YacGuid, nullptr,-1,-1,DX,DY,L"EditDlg",DialogItems,ARRAYSIZE(DialogItems),0,0,0,0);
	int ret=Info.DialogRun(hDlg);

	if(ret==ED_OK)
	{
		TmpOpt.DlgColor=(BYTE)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,ED_COLORCOMBO,NULL);
		TmpOpt.SmartFilter=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,ED_SMARTFILTER,NULL);
		TmpOpt.Hints=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,ED_HINTS,NULL);
		Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,ED_SIZEXEDIT,MaxWidth);
		Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,ED_SIZEYEDIT,MaxHeight);
		WORD MAXX=FSF.atoi(MaxWidth),
		     MAXY=FSF.atoi(MaxHeight);

		if(MAXX<0)MAXX=0;
		if(MAXY<0)MAXY=0;

		TmpOpt.MaxFilterSize=MAKELONG(MAXY,MAXX);

		Info.DialogFree(hDlg);
	}
}

void EditStdOut()
{
	enum ESODITEMS
	{
		ESO_BORDER,
		ESO_MULTICOLUMN,
		ESO_PAUSABLE,
		ESO_SEPATATOR,
		ESO_OK,
		ESO_CANCEL,
	};

	int DX=40,DY=8;

	FarDialogItem DialogItems[]=
	{
		{DI_DOUBLEBOX,/**/3, /**/1,   /**/DX-4,/**/DY-2,/**/0,                 /**/nullptr, nullptr, 0,                         /**/GetMsg(MStdout)},
		{DI_CHECKBOX, /**/5, /**/2,   /**/5,   /**/2,   /**/TmpOpt.Multicolumn,/**/nullptr, nullptr, 0,                         /**/GetMsg(MMulticolumn)},
		{DI_CHECKBOX, /**/5, /**/3,   /**/5,   /**/3,   /**/TmpOpt.Pausable,   /**/nullptr, nullptr, 0,                         /**/GetMsg(MPausable)},
		{DI_TEXT,     /**/-1,/**/DY-4,/**/-1,  /**/DY-4,/**/0,                 /**/nullptr, nullptr, DIF_BOXCOLOR|DIF_SEPARATOR,/**/L""},
		{DI_BUTTON,   /**/0, /**/DY-3,/**/0,   /**/DY-3,/**/0,                 /**/nullptr, nullptr, DIF_CENTERGROUP|DIF_DEFAULTBUTTON,           /**/GetMsg(MOK)},
		{DI_BUTTON,   /**/0, /**/DY-3,/**/0,   /**/DY-3,/**/0,                 /**/nullptr, nullptr, DIF_CENTERGROUP,           /**/GetMsg(MCancel)},
	};

	HANDLE hDlg=Info.DialogInit(YacGuid, nullptr,-1,-1,DX,DY,L"EditStdOut",DialogItems,ARRAYSIZE(DialogItems),0,0,0,0);
	int ret=Info.DialogRun(hDlg);

	if(ret==ESO_OK)
	{
		TmpOpt.Multicolumn=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,ESO_MULTICOLUMN,NULL);
		TmpOpt.Pausable=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,ESO_PAUSABLE,NULL);
	}
	Info.DialogFree(hDlg);

}

INT_PTR WINAPI OptDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch(Msg)
	{
	case DN_INITDIALOG:
		{
			//Info.SendDlgMessage(hDlg,DM_SETMOUSEEVENTNOTIFY,TRUE,0);
			Info.SendDlgMessage(hDlg,DM_SETCOMBOBOXEVENT,DLG_SHOWRESULTS,(void*)CBET_KEY);
			Info.SendDlgMessage(hDlg,DM_SETCOMBOBOXEVENT,DLG_BEEPCOMBO,(void*)CBET_KEY);
			FarListTitles t={sizeof(t),0,NULL,0,(LPTSTR)GetMsg(MEditDlgAndStdOut)};
			Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,DLG_SHOWRESULTS,&t);
			t.Bottom=(LPTSTR)GetMsg(MPreviewSound);
			Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,DLG_BEEPCOMBO,&t);
			
			Info.SendDlgMessage(hDlg,DM_SETCMP,0,(void*)Opt.Cmd.CmdCompletion);
		}
		break;

	case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* p = static_cast<const INPUT_RECORD*>(Param2);
			bool Key = p->EventType == KEY_EVENT;
			switch(Param1)
			{
			case DLG_SHOWRESULTS:
				if(Key && p->Event.KeyEvent.wVirtualKeyCode == VK_F4 && Info.SendDlgMessage(hDlg,DM_GETDROPDOWNOPENED,0,0))
				{
					switch(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,DLG_SHOWRESULTS,NULL))
					{
					case 0:
						break;
					case 1:
						EditDlg();
						break;
					case 2:
						EditStdOut();
						break;
					}
				}
				break;
			case DLG_BEEPCOMBO:
				if(Key && p->Event.KeyEvent.wVirtualKeyCode == VK_F3)
					Info.SendDlgMessage(hDlg,DN_BTNCLICK,DLG_BEEPTEST,0);
				break;
			}
		}
		break;

	case DN_EDITCHANGE:
		switch(Param1)
		{
		case DLG_SHOWRESULTS:
			{
				Info.SendDlgMessage(hDlg,DM_ENABLE,DLG_SHOWTYPE,(void*)(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,DLG_SHOWRESULTS,NULL)!=0));
			}
			break;
		case DLG_BEEPCOMBO:
			{
				SMALL_RECT sr;
				Info.SendDlgMessage(hDlg,DM_GETITEMPOSITION,DLG_BEEPCOMBO,&sr);
				INT_PTR pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,DLG_BEEPCOMBO,0);
				sr.Right=(pos==1?CFGDLGX-32:CFGDLGX-15);
				Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,DLG_BEEPCOMBO,&sr);
				Info.SendDlgMessage(hDlg,DM_ENABLE,DLG_BEEPTEST,(void*)pos);
				for(int iID=DLG_BEEPHZTXT;iID<DLG_BEEPTEST;iID++)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,iID,(void*)(pos==1));
					Info.SendDlgMessage(hDlg,DM_SHOWITEM,iID,(void*)(pos==1));
				}
			}
			break;
		}
		break;
	case DN_BTNCLICK:
		switch(Param1)
		{
		case DLG_BEEPTEST:
			{
				TCHAR cHZ[6],cMS[4];
				Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DLG_BEEPHZ,cHZ);
				Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DLG_BEEPMS,cMS);
				int HZ=FSF.atoi(cHZ),
				    MS=FSF.atoi(cMS);
				if(HZ>0x7FFF)
					HZ=0x7FFF;
				if(MS>999)
					MS=999;
				PlayBeep((HZ<<16)+(MS)+(int)(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,DLG_BEEPCOMBO,0))*0x1000);
			}
			break;
		case DLG_BEEPCHECK:
			{
				Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
				for(int iID=DLG_BEEPCOMBO;iID<DLG_BEEPTEST+1;iID++)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,iID,Param2);
				}
				if(!Param2)
				{
					FarListPos lp={0,-1};
					Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,DLG_BEEPCOMBO,&lp);
					SMALL_RECT sr={6,10,13,10};
					Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,DLG_BEEPCOMBO,&sr);
					for(int iID=DLG_BEEPHZTXT;iID<DLG_BEEPTEST;iID++)
						Info.SendDlgMessage(hDlg,DM_SHOWITEM,iID,(void*)1);
				}
				Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
			}
		}
		break;
/*
	case DN_CLOSE:
		{
			if(Param1==DLG_OK)
			{
				//BUGBUGBUG
				//Opt.Completion=0;
				for(UINT i=0;i<ARRAYSIZE(CmplRules);i++)
				{
					FarListGetItem item;
					item.ItemIndex=i;
					Info.SendDlgMessage(hDlg,DM_LISTGETITEM,DLG_COMPLETION,&item);
					//BUGBUGBUG
					//Opt.Completion|=CmplRules[i].Value*(LIF_CHECKED==(item.Item.Flags&LIF_CHECKED));
				}
			}
		}
		break;
*/
	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void OptDlg()
{
	TmpOpt.MaxFilterSize=Opt.MaxFilterSize;
	TmpOpt.DlgColor=Opt.DlgColor;
	TmpOpt.SmartFilter=Opt.SmartFilter;
	TmpOpt.Hints=Opt.Hints;
	TmpOpt.Multicolumn=Opt.Multicolumn;
	TmpOpt.Pausable=Opt.Pausable;

	LPCWSTR DirCmds=*Opt.DirCmds;
	TCHAR HZTXT[6],MSTXT[4];
	FSF.itoa(HIWORD(Opt.Beep)>0x7FFF?0x7FFF:HIWORD(Opt.Beep),HZTXT,10);
	FSF.itoa(LOWORD(Opt.Beep)%0x1000>999?999:LOWORD(Opt.Beep)%0x1000,MSTXT,10);
	int beep=LOWORD(Opt.Beep)/0x1000;

	FarDialogItem DialogItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,CFGDLGX-4,CFGDLGY-2, 0, nullptr, nullptr, DIF_NONE, GetMsg(MOther)},
		{DI_TEXT,      5, 2, 5, 2, 0, nullptr, nullptr, DIF_NONE, GetMsg(MShowResults)},

		{DI_COMBOBOX, CFGDLGX-24, 2,CFGDLGX-6, 2,0,nullptr, nullptr, DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,L""},
		{DI_CHECKBOX,  5, 3, 5, 3,Opt.ShowType,nullptr, nullptr, (Opt.ShowResults?0:DIF_DISABLE),GetMsg(MShowType)},
		{DI_CHECKBOX,  5, 4, 5, 4,Opt.AddSpace,nullptr, nullptr, DIF_NONE,GetMsg(MAddSpace)},
		{DI_CHECKBOX,  5, 5, 5, 5,Opt.SaveTail,nullptr, nullptr, DIF_NONE,GetMsg(MSaveTail)},

		{DI_TEXT,      5,6, 5,6,0,nullptr, nullptr, DIF_NONE, GetMsg(MDirCmds)},
		{DI_EDIT,     CFGDLGX-24,6,CFGDLGX-6,6,0,nullptr, nullptr, DIF_NONE, DirCmds},

		{DI_TEXT,     -1,7,-1,7,0,nullptr, nullptr, DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERTEXT,L""},
		{DI_TEXT,      5,8, 5,8,0,nullptr, nullptr, DIF_NONE, GetMsg(MBeep)},
		{DI_COMBOBOX,  5,9,(beep==1?CFGDLGX-32:CFGDLGX-15),9,0,nullptr, nullptr, DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,L""},

		{DI_TEXT,     CFGDLGX-29,9,CFGDLGX-29,9,0, nullptr, nullptr, (beep==1?0:DIF_DISABLE|DIF_HIDDEN),GetMsg(MHz)},
		{DI_FIXEDIT,  CFGDLGX-25,9,CFGDLGX-23,9,0, nullptr, nullptr, (beep==1?0:DIF_DISABLE|DIF_HIDDEN),HZTXT},
		{DI_TEXT,     CFGDLGX-21,9,CFGDLGX-21,9,0, nullptr, nullptr, (beep==1?0:DIF_DISABLE|DIF_HIDDEN),GetMsg(MMs)},
		{DI_FIXEDIT,  CFGDLGX-17,9,CFGDLGX-15,9,0, nullptr, nullptr, (beep==1?0:DIF_DISABLE|DIF_HIDDEN),MSTXT},
		{DI_BUTTON,   CFGDLGX-12,9,CFGDLGX-4, 9,0, nullptr, nullptr, (beep?0:DIF_DISABLE)|DIF_BTNNOCLOSE,GetMsg(MTest)},

//		{DI_TEXT,-1,14,-1,14,0,0,DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERTEXT,0,L""},
//		{DI_TEXT,5,15,5,15,0,0,0,0,GetMsg(MPrefix)},
//		{DI_EDIT,28,15,39,15,0,0,0,0,Opt.CommandPrefixA},
//		{DI_TEXT,-1,16,-1,16,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L""},

		{DI_TEXT, -1,CFGDLGY-4,-1,CFGDLGY-4,0,nullptr, nullptr, DIF_BOXCOLOR|DIF_SEPARATOR|DIF_CENTERTEXT,L""},
		{DI_BUTTON,0,CFGDLGY-3, 0,CFGDLGY-3,0,nullptr, nullptr, DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(MOK)},
		{DI_BUTTON,0,CFGDLGY-3, 0,CFGDLGY-3,0,nullptr, nullptr, DIF_CENTERGROUP,GetMsg(MCancel)},
	};

	FarListItem cmpitems[ARRAYSIZE(CmplRules)];
	for(int i=0;i<ARRAYSIZE(CmplRules);i++)
	{
		cmpitems[i].Text=GetMsg(MComplFilenames+i);
		//BUGBUGBUG
		//cmpitems[i].Flags=(Opt.Completion&CmplRules[i].Value)?LIF_CHECKED:0;
	}

#if 0
	cmpitems[T_KEY].Flags|=LIF_DISABLE;
#endif

	/*FarList CmpComboList={ARRAYSIZE(CmplRules),cmpitems};
	DialogItems[DLG_COMPLETION].ListItems=&CmpComboList;


	FarListItem AreaComboItems[3];
	memset(AreaComboItems,0,sizeof(AreaComboItems));
#ifndef UNICODE
	lstrcpy(AreaComboItems[0].Text,GetMsg(MAreaCmd));
	lstrcpy(AreaComboItems[1].Text,GetMsg(MAreaEditor));
	lstrcpy(AreaComboItems[2].Text,GetMsg(MAreaDlg));
#else
	AreaComboItems[0].Text=GetMsg(MAreaCmd);
	AreaComboItems[1].Text=GetMsg(MAreaEditor);
	AreaComboItems[2].Text=GetMsg(MAreaDlg);
#endif
	AreaComboItems[0].Flags=LIF_SELECTED;
	FarList AreaComboList={ARRAYSIZE(AreaComboItems),AreaComboItems};
	DialogItems[DLG_AREA].ListItems=&AreaComboList;



	FarListItem TypeComboItems[2];
	memset(TypeComboItems,0,sizeof(TypeComboItems));
#ifndef UNICODE
	lstrcpy(TypeComboItems[0].Text,GetMsg(MTypeCmd));
	lstrcpy(TypeComboItems[1].Text,GetMsg(MTypeParam));
#else
	TypeComboItems[0].Text=GetMsg(MTypeCmd);
	TypeComboItems[1].Text=GetMsg(MTypeParam);
#endif
	TypeComboItems[0].Flags=LIF_SELECTED;
	FarList TypeComboList={ARRAYSIZE(TypeComboItems),TypeComboItems};
	DialogItems[DLG_CMDTYPE].ListItems=&TypeComboList;

*/


	FarListItem ShowComboItems[3]={0};
	ShowComboItems[0].Text=GetMsg(MDisabled);
	ShowComboItems[1].Text=GetMsg(MList);
	ShowComboItems[2].Text=GetMsg(MStdout);
	ShowComboItems[Opt.ShowResults%3].Flags=LIF_SELECTED;
	FarList ShowComboList={ARRAYSIZE(ShowComboItems),ShowComboItems};
	DialogItems[DLG_SHOWRESULTS].ListItems=&ShowComboList;

	FarListItem SoundComboItems[8]={0};

	LPCTSTR Snds[]=
	{
		GetMsg(MDisabled),
		GetMsg(MSpeaker),
		GetMsg(MSimpleBeep),
		GetMsg(MDefaultBeep),

		GetMsg(MSystemHand),
		GetMsg(MSystemQuestion),
		GetMsg(MSystemExclamation),
		GetMsg(MSystemAsterisk),

		//L"",L"",L"",L"",L"",
	};
	/**pStrRegRoot=L"AppEvents\\EventLabels";
	GetRegKey(L".Default",L"",Snds[3],L"Default Beep");
	GetRegKey(L"SystemHand",L"",Snds[4],L"Critical Stop");
	GetRegKey(L"SystemQuestion",L"",Snds[5],L"Question");
	GetRegKey(L"SystemExclamation",L"",Snds[6],L"Exclamation");
	GetRegKey(L"SystemAsterisk",L"",Snds[7],L"Asterisk");
	*pStrRegRoot=Info.RootKey;*/

	for(int i=0;i<ARRAYSIZE(SoundComboItems);i++)
		SoundComboItems[i].Text=Snds[i];
	SoundComboItems[beep].Flags=LIF_SELECTED;
	FarList SoundComboList={ARRAYSIZE(SoundComboItems),SoundComboItems};
	DialogItems[DLG_BEEPCOMBO].ListItems=&SoundComboList;

	HANDLE hDlg=Info.DialogInit(YacGuid, nullptr,-1,-1,CFGDLGX,CFGDLGY,L"Opt",DialogItems,ARRAYSIZE(DialogItems),0,0,OptDlgProc,0);
	int result=Info.DialogRun(hDlg);

	if(result==DLG_OK)
	{
		wchar_t* lpDirCmds=Opt.DirCmds->GetBuffer(Info.SendDlgMessage(hDlg,DM_GETTEXTLENGTH,DLG_DIRCMDSDATA,0)+1);
		Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DLG_DIRCMDSDATA,lpDirCmds);
		Opt.DirCmds->ReleaseBuffer();
		Opt.ShowResults=(DWORD)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,DLG_SHOWRESULTS,0);
		wchar_t wHZ[4],wMS[4];
		Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DLG_BEEPHZ,wHZ);
		Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DLG_BEEPMS,wMS);
		int HZ=FSF.atoi(wHZ),MS=FSF.atoi(wMS);
		Opt.Beep=(DWORD)((HZ>0x7FFF?0x7FFF:HZ)<<16)+(MS>999?999:MS)+((DWORD)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,DLG_BEEPCOMBO,0)*0x1000);
		Opt.ShowType=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,DLG_SHOWTYPE,0);
		Opt.AddSpace=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,DLG_ADDSPACE,0);
		Opt.SaveTail=(BOOL)Info.SendDlgMessage(hDlg,DM_GETCHECK,DLG_SAVETAIL,0);

		Opt.MaxFilterSize=TmpOpt.MaxFilterSize;
		Opt.DlgColor=TmpOpt.DlgColor;
		Opt.SmartFilter=TmpOpt.SmartFilter;
		Opt.Hints=TmpOpt.Hints;
		Opt.Multicolumn=TmpOpt.Multicolumn;
		Opt.Pausable=TmpOpt.Pausable;
	}
	Info.DialogFree(hDlg);
}

int Config(int/* ItemNumber*/)
{
	CfgMode=true;

	FarMenuItem MenuItems[]=
	{
		{MIF_NONE, GetMsg(MCompl)},
		{MIF_NONE, GetMsg(MOther)},
	};

	int MenuCode=0;
	while(MenuCode!=-1)
	{
		for(int i=0;i<ARRAYSIZE(MenuItems);i++)
			MenuItems[i].Flags&=~MIF_SELECTED;
		MenuItems[MenuCode].Flags|=MIF_SELECTED;
		MenuCode=Info.Menu(YacGuid,nullptr,-1,-1,0,FMENU_WRAPMODE,GetMsg(MName),NULL,L"Config",0,0,MenuItems,ARRAYSIZE(MenuItems));
		switch(MenuCode)
		{
		case 0:
			DlgOptCompletion();
			break;
		case 1:
			OptDlg();
			break;
		default:
			break;
		}
	}
	//OptDlg();
	CfgMode=false;
	WriteConfig();
	return FALSE;
}
