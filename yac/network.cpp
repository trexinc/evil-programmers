DWORD WINAPI NetAddThreadProc(void* lpParameter)
{
	ThParams* p=(ThParams*)lpParameter;
	switch(p->Type)
	{
	case 0:
		*p->res=NetServerEnum(NULL,100,(LPBYTE*)p->Data,MAX_PREFERRED_LENGTH,p->entriesread,p->totalentries,SV_TYPE_ALL,NULL,p->resume_handle);
		break;
	case 1:
		*p->res=NetShareEnum((wchar_t*)p->ServerName,1,(LPBYTE*)p->Data,MAX_PREFERRED_LENGTH,p->entriesread,p->totalentries,p->resume_handle);
		break;
	//case 2:
		//*p->res=NetUserEnum(NULL,10,0,(LPBYTE*)p->Data,MAX_PREFERRED_LENGTH,p->entriesread,p->totalentries,p->resume_handle);
		//break;
	}
	Info.SendDlgMessage(*p->hDlg,DM_CLOSE,-3,0);
	return *p->res;
}

INT_PTR WINAPI NetScanDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch(Msg)
	{
	case DN_INITDIALOG:
		*((HANDLE*)Param2)=hDlg;
		break;
	case DN_CLOSE:
		switch(Param1)
		{
		case 0:
			return FALSE;
		case -3:
			return TRUE;
		default:
			LPCTSTR MsgItems[]={GetMsg(MAbortCaption),GetMsg(MAbortText),L"\x1"};
			return !Info.Message(YacGuid, nullptr, FMSG_WARNING|FMSG_MB_YESNO,0,MsgItems,ARRAYSIZE(MsgItems),0);
		}
	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void DoNetEnum(ThParams* p)
{
	HANDLE hNetDlg;
	p->hDlg=&hNetDlg;
	HANDLE hThread=CreateThread(0,0,NetAddThreadProc,(void*)p,0,0);
	if(hThread)
	{
		int n1=lstrlen(GetMsg(MPleaseWait)),
				n2=lstrlen(GetMsg(MNetScan)),
				MaxLen=Max(n1,n2);
		
		FarDialogItem DialogItems[]=
		{
			{DI_DOUBLEBOX,3,1,MaxLen+6,3,0,0,0,0,GetMsg(MPleaseWait)},
			{DI_TEXT,-1,2,0,2,0,0,0,0,GetMsg(MNetScan)},
		};
		HANDLE _hDlg = Info.DialogInit(YacGuid, nullptr, -1,-1,MaxLen+10,5,L"NetScanDlg",DialogItems,ARRAYSIZE(DialogItems),0,0,NetScanDlgProc,&hNetDlg);
		Info.DialogRun(_hDlg);
		Info.DialogFree(_hDlg);
		DWORD ret;
		GetExitCodeThread(hThread,&ret);
		if(ret==STILL_ACTIVE)
		{
			TerminateThread(hThread,1);
			WaitForSingleObject(hThread,10000);
		}
		CloseHandle(hThread);
	}
}

int SavePassword(LPCWSTR ServerName,LPCWSTR UserName,LPCWSTR Password)
{
	CREDENTIALW c;
	ZeroMemory(&c,sizeof(CREDENTIALW));
	c.Type=CRED_TYPE_DOMAIN_PASSWORD;
	c.TargetName=(wchar_t*)ServerName;
	c.CredentialBlobSize=static_cast<DWORD>(wcslen(Password)*sizeof(wchar_t));
	c.CredentialBlob=(LPBYTE)Password;
	c.Persist=CRED_PERSIST_LOCAL_MACHINE;
	wchar_t UN[512];
	lstrcpyW(UN,ServerName);
	AddEndSlash(UN);
	lstrcatW(UN,UserName);
	c.UserName=UN;
	if(_CredWriteW)
		_CredWriteW(&c,0);
	return 1;
}

int Connect(LPCWSTR path)
{
	int Err=GetLastError();
	HMODULE hAdvApi=GetModuleHandleW(L"AdvApi32");
	if(hAdvApi)
		_CredWriteW=(PCredWriteW)GetProcAddress(hAdvApi,"CredWriteW");
	SetLastError(Err);
	enum
	{
		CONDLG_BORDER,
		CONDLG_USERNAMELABEL,
		CONDLG_USERNAMEEDIT,
		CONDLG_USERPASSLABEL,
		CONDLG_USERPASSEDIT,
		CONDLG_SAVEPASSBOX,
		CONDLG_SEPARATOR,
		CONDLG_OK,
		CONDLG_CANCEL
	};

	string ServerName=path;
	int i=2;
	while(ServerName.At(i) && !IsSlash(ServerName.At(i)))
		i++;
	ServerName.SetLength(i);
	LPCTSTR FarServerName;
	
	LPTSTR DlgOk=new TCHAR[lstrlen(GetMsg(MOK))+5];
	lstrcpy(DlgOk,L"{ ");
	lstrcat(DlgOk,GetMsg(MOK));
	lstrcat(DlgOk,L" }");

	FarServerName=ServerName;
	FarDialogItem DialogItems[]=
	{
		{DI_DOUBLEBOX,  3, 1, 44, 7, 0, nullptr,        nullptr, DIF_NONE,                       FarServerName},
		{DI_TEXT,       5, 2,  5, 2, 0, nullptr,        nullptr, DIF_FOCUS,                       GetMsg(MUserName)},
		{DI_EDIT,      19, 2, 42, 2, 1, L"NetworkUser", nullptr, DIF_HISTORY|DIF_USELASTHISTORY, L""},
		{DI_TEXT,       5, 3,  5, 3, 0, nullptr,        nullptr, DIF_NONE,                       GetMsg(MUserPass)},
		{DI_PSWEDIT,   19, 3, 42, 3, 0, nullptr,        nullptr, DIF_NONE,                       L""},
		{DI_CHECKBOX,  19, 4, 19, 4, 0, nullptr,        nullptr, _CredWriteW?0:DIF_DISABLE,      GetMsg(MSavePass)},
		{DI_TEXT,       3, 5,  3, 5,  0,nullptr,        nullptr, DIF_SEPARATOR|DIF_BOXCOLOR,     L""},
		{DI_BUTTON,     0, 6,  0, 6,  0,nullptr,        nullptr, DIF_CENTERGROUP|DIF_NOBRACKETS|DIF_DEFAULTBUTTON, DlgOk},
		{DI_BUTTON,     0, 6,  0, 6,  0,nullptr,        nullptr, DIF_CENTERGROUP,                GetMsg(MCancel)},
	};
	HANDLE hDlg=Info.DialogInit(YacGuid, nullptr, -1,-1,48,9,L"LogonDlg",DialogItems,ARRAYSIZE(DialogItems),0,0,0,0);
	int result=Info.DialogRun(hDlg);
	delete[] DlgOk;
	if(result==-1||result==CONDLG_CANCEL)
	{
		Info.DialogFree(hDlg);
		return -1;
	}
	string UserName,UserPass;
	wchar_t* lpwszUserName=UserName.GetBuffer(Info.SendDlgMessage(hDlg,DM_GETTEXTLENGTH,CONDLG_USERNAMEEDIT,0)+1);
	wchar_t* lpwszUserPass=UserPass.GetBuffer(Info.SendDlgMessage(hDlg,DM_GETTEXTLENGTH,CONDLG_USERPASSEDIT,0)+1);
	Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,CONDLG_USERNAMEEDIT,lpwszUserName);
	Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,CONDLG_USERPASSEDIT,lpwszUserPass);
	UserName.ReleaseBuffer();
	UserPass.ReleaseBuffer();
	if(UserName.Empty())
		UserName=L"guest";
	if(Info.SendDlgMessage(hDlg,DM_GETCHECK,CONDLG_SAVEPASSBOX,0)==BSTATE_CHECKED)
		// /*if(*/SavePassword(&ServerName[2],UserName,UserPass)/*)return 1*/;
		//if(SavePassword(&ServerName[2],UserName,UserPass))
	{
		string srvname=ServerName.SubString(2);
		/*if(*/SavePassword(srvname,UserName,UserPass)/*)
			return 1*/;
	}

	USE_INFO_2 ui2;
	ui2.ui2_local=NULL;
	ui2.ui2_remote=(wchar_t*)path;
	ui2.ui2_password=(wchar_t*)UserPass.CPtr();
	ui2.ui2_status=0;
	ui2.ui2_asg_type=USE_WILDCARD;
	ui2.ui2_refcount=0;
	ui2.ui2_usecount=0;
	ui2.ui2_username=(wchar_t*)UserName.CPtr();
	ui2.ui2_domainname=0;
	DWORD ParmError;
	NET_API_STATUS res;
	if((res=NetUseAdd(NULL,2,(LPBYTE)&ui2,&ParmError))==NERR_Success)
		return 1;
	else
	{
		StdErrMsg(res);
	}
	Info.DialogFree(hDlg);
	return 0;
}
