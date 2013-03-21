#include "ntapi.hpp"
#include "winapi.hpp"

void DrawWaitMessage(int& TickCount)
{
	if(TickCount && GetTickCount()-TickCount>500)
	{
		TickCount=0;
		LPCTSTR Msgs[]={L"",GetMsg(MPleaseWait)};
		Info.Message(YacGuid, nullptr, 0, 0, Msgs, 2, 0);
		Info.Text(0,0,0,NULL);
	}
}

void GetVolumeLabelAndFS(LPCWSTR path,string &str)
{
	WCHAR VolName[MAX_PATH+1];
	WCHAR FSName[MAX_PATH+1];
	if(GetVolumeInformationW(path,VolName,ARRAYSIZE(VolName),NULL,NULL,NULL,FSName,ARRAYSIZE(FSName)))
	{
		str+=GetMsg(MLabel);
		str+=VolName;
		str+=L"\n";
		str+=GetMsg(MFS);
		str+=FSName;
	}
}

bool EnumKeys(LPCWSTR From)
{
	if(!CheckCompletion(T_KEY))
		return false;
	string strKeys;
	GetRegKey(From,RegStr.keys,strKeys,L"");
	if(!strKeys.Empty())
	{
		Tokenizer t(strKeys,L" ");
		while(!t.IsEnd())
		{
			string Key= t.Next();
			if(ResumeData->pattern.Empty() || Key.Equal(ResumeData->pattern,ResumeData->pattern.GetLength()))
			//{
			//	ReplaceStrings(Key,L"\\x20",L" ",-1,false);
				GlobalContainer->Add(Key,CMP(T_KEY));
			//}
		}
	}
	string SubKey;
	for(int i=0;;i++)
	{
		SubKey=L"";
		if(!EnumRegKey(From,i,SubKey,false))
			break;
		if(!ResumeData->pattern.Empty()&&!SubKey.Equal(ResumeData->pattern,ResumeData->pattern.GetLength()))
			continue;
		string Hint;
		if(Opt.Hints)
		{
			string sk=From;
			sk+=L"\\"+SubKey;
			GetRegKey(sk,L"hint",Hint,L"");
		}
		GlobalContainer->Add(SubKey,CMP(T_KEY),Hint);
	}
	return true;
}

void GetLocal()
{
	string KeyName;
	KeyName.Format(L"%s\\%s\\%s",PluginKey,ContextKey,ResumeData->rootcmd.CPtr());
	if(!ResumeData->rootcmd.Empty())
	{
		// найдЄм нужный ключ
		if(!IsRegKeyExist(KeyName)) //точного совпадени€ нет
		{
			string RootCmdName=ResumeData->rootcmd;
			if(IsNamePathExt(RootCmdName)) //им€ с расширением из pathext, выкинем его и попробуем ещЄ раз
			{
				string EntireName=GetName(RootCmdName);
				KeyName.Format(L"%s\\%s\\%s",PluginKey,ContextKey,EntireName.CPtr());
			}
			else //им€ без расширени€, подставл€ем по очереди pathext и пробуем.
			{
				string PathExt;
				apiExpandEnvironmentStrings(L"%PATHEXT%",PathExt);
				Tokenizer t(PathExt,L";");
				while(!t.IsEnd())
				{
					string CurKey;
					CurKey.Format(L"%s\\%s\\%s%s",PluginKey,ContextKey,RootCmdName.CPtr(),t.Next().CPtr());
					//if(GetRegKey(CurKey,RegStr.Completion,LocalCompletion,Opt.Completion)) //что-то нашли
					if(IsRegKeyExist(CurKey))
					{
						KeyName=CurKey;
						break;
					}
				}
			}
		}
	}
	// прочитаем из него настройки дополнени€...
	//GetRegKey(KeyName,RegStr.Completion,LocalCompletion,Opt.Completion);
	switch(ThisState->Workspace)
	{
	case W_SHELL:
		GetRegKey(KeyName,RegStr.CmdParamCompletion,LocalCompletion,Opt.Cmd.ParamCompletion);
		break;
	case W_EDITOR:
		GetRegKey(KeyName,RegStr.EditParamCompletion,LocalCompletion,Opt.Edit.ParamCompletion);
		break;
	case W_DIALOG:
		GetRegKey(KeyName,RegStr.DlgParamCompletion,LocalCompletion,Opt.Dlg.ParamCompletion);
		break;
	}

	// а вот теперь попробуем их переопределить, использу€ стек.
	string NewKeyName=KeyName;
	for(;;)
	{
		string add=SStack->pop();
		if(add.Empty())
			break;
		NewKeyName+=L"\\"+add;
		if(IsRegKeyExist(NewKeyName))
			KeyName=NewKeyName;
	}

	// и ещЄ разок, с наследованием при неудаче
	//GetRegKey(KeyName,RegStr.Completion,LocalCompletion,LocalCompletion);
	switch(ThisState->Workspace)
	{
	case W_SHELL:
		GetRegKey(KeyName,RegStr.CmdParamCompletion,LocalCompletion,LocalCompletion);
		break;
	case W_EDITOR:
		GetRegKey(KeyName,RegStr.EditParamCompletion,LocalCompletion,LocalCompletion);
		break;
	case W_DIALOG:
		GetRegKey(KeyName,RegStr.DlgParamCompletion,LocalCompletion,LocalCompletion);
		break;
	}

	GetRegKey(KeyName,RegStr.filter,*MaskFilter,L"");

	//а вот теперь можно и ключи
	EnumKeys(KeyName);
}

void GetAliases(LPCWSTR pattern,LPCWSTR ExeName)
{
	string Str=pattern;
	int bufsize=GetConsoleAliasesLengthW((wchar_t*)ExeName);
	if(bufsize)
	{
		wchar_t* buffer=new WCHAR[bufsize/sizeof(WCHAR)];
		memset(buffer,0,bufsize);
		if(GetConsoleAliasesW(buffer,bufsize,(wchar_t*)ExeName))
		{
			for(wchar_t* ptr=buffer;ptr-buffer<(int)(bufsize/sizeof(WCHAR));ptr++)
			{
				wchar_t* alias=ptr,*data=ptr;
				while(*ptr&&*ptr!=L'=')
					ptr++;
				if(*ptr)
				{
					data=ptr;
					while(*ptr)ptr++;
					*data=0;
					data++;
				}
				if(Str.Equal(alias,Str.GetLength()))
				{
					string Hint;
					if(Opt.Hints)
					{
						Hint=GetMsg(MValue);
						Hint+=data;
					}
					string Item=alias;
					string root;
					if(TokenVirtualRoot(ResumeData->pattern,root))
						Item=Item.SubString(root.GetLength());
					GlobalContainer->Add(Item,CMP(T_ALS),Hint);
				}
			}
		}
		delete[] buffer;
	}
}


bool EnumAliases()
{
	if(!CheckCompletion(T_ALS))
		return false; 
	WCHAR badchars[]={L'<',L'>'};
	size_t pos=0;
	for(int i=0;i<ARRAYSIZE(badchars);i++)
		if(ResumeData->pattern.Pos(pos,badchars[i]))
			return false;
	GetAliases(ResumeData->pattern,GetFileName(*FarName));
	string comspec;
	apiExpandEnvironmentStrings(L"%COMSPEC%",comspec);
	GetAliases(ResumeData->pattern,GetFileName(comspec));
	return true;
}

int EnumComSpec()
{
	// oops, broken
	/*
	if(!CheckCompletion(T_CMD))
		return false;
	string Cmds;

	string strRegRoot=GetRegRootString();
	strRegRoot.SetLength(strRegRoot.GetLength()-8); //8 - "\plugins"
	SetRegRootString(strRegRoot);

	GetRegKey(L"System\\Executor",L"ExcludeCmds",Cmds,L"");
	//far internals
	Cmds+=L";cls;cd;chdir;chcp;set;if";
	SetRegRootString(Info.RootKey);
	Tokenizer t(Cmds,L";");
	while(!t.IsEnd())
	{
		string cmd=t.Next();
		if(cmd.Equal(ResumeData->pattern,ResumeData->pattern.GetLength()))
			GlobalContainer->Add(cmd,CMP(T_CMD));
	}*/
	return true;
}

bool EnumSysAliases()
{
	// oops, broken
	/*
	if(!CheckCompletion(T_SAL))
		return false;
	string strValueName;
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	SetRegRootString(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");
	bool done=false;
job: // goto, несомненно, зло, но в данном случае необходимое :)
	for(int i=0;;i++)
	{
		strValueName=L"";
		if(!EnumRegKey(L"",i,strValueName))
			break;
		if(!ResumeData->pattern.Empty()&&!strValueName.Equal(ResumeData->pattern,ResumeData->pattern.GetLength()))
			continue;
		string strValue;
		if(Opt.Hints)
		{
			string strData;
			GetRegKey(strValueName,L"",strData,L"");
			strValue=GetMsg(MValue)+strData;
		}
		GlobalContainer->Add(strValueName,CMP(T_SAL),strValue);
	}
	if(done)
		goto end;
	SetRegRootKey(HKEY_CURRENT_USER);
	done=true;
	goto job;
end:
	SetRegRootString(Info.RootKey);
	*/
	return true;
}

bool EnumEnvVars(LPCWSTR Str,bool nodirmode=false,LPCWSTR curdir=L"")
{
	bool NeedPercent=!(ResumeData->prefix==L"set ");
	if(!CheckCompletion(T_ENV)||(*Str&&*Str!=L'%'&&NeedPercent))
		return false;
	string str=*Str?&Str[NeedPercent]:L"";
	size_t CharCount=str.GetLength();
	if(CharCount)
	{
		if(IsSlash(str.At(CharCount-1))||str.At(CharCount-1)==L'*')
			return false;
		if(str.At(CharCount-1)==L'%')str.SetLength(CharCount-1);
	}
	string var;
	void* lpvEnv=GetEnvironmentStringsW();
	if(!lpvEnv)return false;
	for(LPCWSTR Ptr=(const wchar_t*)lpvEnv;*Ptr;Ptr++)
	{
		var=Ptr;
		if((var.At(0)==L'=')&&(var.At(2)==L':')&&(var.At(3)==L'=')/*&&(var.At(4)==0)*/)
		{
			while(*Ptr)Ptr++;
			continue;
		}
		if(var.Equal(str,str.GetLength()))
		{
			LPCWSTR p=var,Start=p;
			string right;
			if(*p==L'=')p++;
			while(*p && *p!=L'=')p++;
			if(*p)
			{
				var.SetLength(p-Start);
				p++;
				right=p;
			}
			// fix for relative directory path
			string DirWithPrefix=L"\\\\?\\"+right;
			BYTE IsDir=CheckIsDir(DirWithPrefix);
			string CurExpFile=curdir;
			CurExpFile+=right;
			if(
				(!nodirmode&&IsDirCmd&&(IsDir!=1))||
				(nodirmode&&(IsDir))||
				(nodirmode&&(CheckIsDir(CurExpFile)==0)&&ResumeData->prefix.Empty())||
				(nodirmode&&wcsstr(right,L":\\")) //hack :)
			)
			{
				while(*Ptr)Ptr++;
				continue;
			}
			string FormVar=L"";
			if(NeedPercent) FormVar+=L"%";
			FormVar+=var;
			if(NeedPercent) FormVar+=L"%";
			string ExpStr,Hint;
			if(NeedPercent||Opt.Hints)
				apiExpandEnvironmentStrings(FormVar,ExpStr);
			if(NeedPercent)
			{
				apiExpandEnvironmentStrings(FormVar,ExpStr);
				if(((CheckIsDir(ExpStr)==1)||(CheckIsDir(CurExpFile)==1))&&
					(Opt.AddEndSlash&CMP(T_ENV))&&(!IsSlash(ExpStr.At(ExpStr.GetLength()-1))))
					AddEndSlash(FormVar);
			}
			if(Opt.Hints)
			{
				Hint=GetMsg(MValue);
				Hint+=ExpStr;
			}
			GlobalContainer->Add(FormVar,CMP(T_ENV),Hint);
		}
		while(*Ptr)
			Ptr++;
	}
	FreeEnvironmentStringsW((wchar_t*)lpvEnv);
	return true;
}

bool EnumLogicalDrives()
{
	if(!CheckCompletion(T_DRV))
		return false;
	if(ResumeData->pattern.Empty()||!ResumeData->pattern.At(1)||(ResumeData->pattern.At(0)==L'\\'&&ResumeData->pattern.At(1)==L'\\'&&ResumeData->pattern.At(3)==L'\\'&&((ResumeData->pattern.At(4)&&!ResumeData->pattern.At(5))||!ResumeData->pattern.At(4))&&(ResumeData->pattern.At(2)==L'.'||ResumeData->pattern.At(2)==L'?')))
	{
		DWORD drives=GetLogicalDrives();
		WCHAR D[]=L"A:\\",d[]=L"a:\\";
		if(!(Opt.AddEndSlash&CMP(T_DRV)))
			D[2]=0; //уберЄм слеш
		for(UINT i=1;i<0x3FFFFFF;i<<=1)
		{
			if(((i&drives)==i)&&(ResumeData->pattern.Empty()||(!ResumeData->pattern.At(1)&&(ResumeData->pattern.At(0)==*d||ResumeData->pattern.At(0)==*D))||(ResumeData->pattern.At(1)&&(!ResumeData->pattern.At(4)||ResumeData->pattern.At(4)==*d||ResumeData->pattern.At(4)==*D))))
			{
				string Hint;
				if(Opt.Hints)
					GetVolumeLabelAndFS(D,Hint);
				GlobalContainer->Add(D,CMP(T_DRV),Hint);
			}
			d[0]++;
			D[0]++;
		}
	}
	return true;
}

bool IsPartialSharePath(LPCWSTR Path)
{
	bool ret=false;
	if(Path && Path[0] == L'\\' && Path[1] == L'\\')
	{
		if((Path[2]==L'.'||Path[2]==L'?')&&Path[3]==L'\\' && !strcmpin(Path+4,L"UNC",3) && IsSlash(Path[7]))
		{
			LPCWSTR ShareName=wcschr(Path+8,L'\\');
			if(ShareName)
				ShareName++;
			if(ShareName && !wcschr(ShareName,L'\\') && !wcschr(ShareName,L'/'))
				ret=true;
		}
		else if(Path[2]!=L'.' && Path[2]!=L'?')
		{
			LPCWSTR ShareName=wcschr(Path+2,L'\\');
			if(ShareName)
				ShareName++;
			if(ShareName && !wcschr(ShareName,L'\\') && !wcschr(ShareName,L'/'))
				ret=true;
		}
	}
	return ret;
}

bool EnumFilenamesEx(LPCWSTR str,bool NoDir,DWORD Type)
{
	if(IsPartialSharePath(str))
	{
		return false;
	}
	size_t charcount=wcslen(str);
	bool result=false;
	WIN32_FIND_DATAW file;
	HANDLE hFile=0;
	string path=str,ParentDirectory=str;
	if(apiGetFileAttributes(ParentDirectory)==INVALID_FILE_ATTRIBUTES)
		apiExpandEnvironmentStrings(ParentDirectory,path);
	if(lstrcmpiW(ParentDirectory,path)&&(ParentDirectory[charcount-1]==L'%'))
	{
		AddEndSlash(ParentDirectory);
		AddEndSlash(path);
		charcount++;
	}
	size_t l=ParentDirectory.GetLength();
	if(!TokenVirtualRoot(ParentDirectory,ParentDirectory))
		ParentDirectory=L"";
	string filename=path.SubString(path.GetLength()-(l-ParentDirectory.GetLength()));
	path+=L"*";
	//current fname

net_retry:
	if(hFile && hFile!=INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	hFile=apiFindFirstFileEx(path,FindExInfoStandard,&file,FindExSearchNameMatch,0,0);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		if(GetLastError()==ERROR_NOT_READY)
		{
			StdErrMsg();
			return false;
		}
		//for network shares content
		if(GetLastError()==ERROR_ACCESS_DENIED)
		{
			if(path.At(0)==L'\\' && path.At(1)==L'\\')
			{
				string ServerName=path,ShareName=L"";
				int si;
				for(si=2;ServerName.At(si);si++)
					if(IsSlash(ServerName.At(si)))
					{
						ShareName=ServerName.SubString(si+1);
						break;
					}
				if(ServerName.At(si)==0)ServerName=L"";
				else ServerName.SetLength(si);
				if(!ShareName.Empty())
				{
					int i=0;
					while(ShareName.At(i) && !IsSlash(ShareName.At(i)))i++;
					if(!ShareName.At(i) || ShareName.At(i+1)==L'*')
					{
						ShareName.SetLength(i);
						ServerName+=L"\\";
						ServerName+=ShareName;
						int ConResult=Connect(ServerName);
						switch(ConResult)
						{
						case 0:
							return false;
						case 1:
							goto net_retry;
						}
					}
				}
			}
//   else
//ShowErrMsg:
//   {
			StdErrMsg();
//   }
		}
		//return false;
		result=false;
	}
	else
	{
		//find anything
		COORD CPos;
		Info.AdvControl(YacGuid, ACTL_GETCURSORPOS, 0, &CPos);
		SMALL_RECT SR={CPos.X,CPos.Y,CPos.X,CPos.Y};
		COORD dwBufferSize={1,1},dwBufferCoord={0,0};
		CHAR_INFO Buffer[1];
		ReadConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE),Buffer,dwBufferSize,dwBufferCoord,&SR);
		int TickCount=GetTickCount();
		WCHAR PrevSymbol;
		PrevSymbol=Buffer[0].Char.UnicodeChar;
		string KeyName;
		do
		{
			if(
						//ignore "." and ".."
						//((file.cFileName[0]==L'.') && !file.cFileName[1])||
						//((file.cFileName[0]==L'.') && (file.cFileName[1]==L'.') && !file.cFileName[2])||
						// no filename completion for directory commands
						(IsDirCmd&&!(file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))||
						//for %PATH%\filename.%PATHEXT% completion
						(NoDir&&(file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
						)continue;
			if(((ThisState->Workspace==W_SHELL)||(Type==CMP(T_PTH)))&&Opt.UsePathExt&&(ResumeData->prefix.Empty()||NoDir)&&!(file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				LPCWSTR Ext=file.cFileName;
				for(;*Ext;Ext++);
				for(;*Ext!=L'.'&&*(Ext-1);Ext--);
				if(Ext==file.cFileName)Ext=0;
				if(!CheckPathExt(Ext))
					continue;
			}
			string AddName=file.cFileName;
			if((file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)&&(Opt.AddEndSlash&CMP(T_FIL)))AddName+=L"\\";
			DrawWaitMessage(TickCount);
			if(**MaskFilter && !(file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if(!FSF.ProcessName(*MaskFilter,file.cFileName,ARRAYSIZE(file.cFileName),PN_CMPNAMELIST)&&!(file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
					continue;
			}
			if(filename.Empty()||AddName.Equal(filename,filename.GetLength()))
			{
				string Hint=GetMsg(MPath);
				Hint+=ParentDirectory.Empty()?L".\\":ParentDirectory;
				Hint+=L"\n";
				string sz;
				ULARGE_INTEGER SZ;
				SZ.HighPart=file.nFileSizeHigh;
				SZ.LowPart=file.nFileSizeLow;
				sz.Format(L"%I64d ",SZ.QuadPart);
				Hint+=GetMsg(MSize)+sz+GetMsg(MBytes);
				GlobalContainer->Add(AddName,Type,Hint);
				result=true;
			}
		}
		while(FindNextFileW(hFile,&file));
		Buffer->Char.UnicodeChar=PrevSymbol;
		WriteConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE),Buffer,dwBufferSize,dwBufferCoord,&SR);
		FindClose(hFile);
	}
	string var=path.SubString(ParentDirectory.GetLength());
	if(var.At(0)==L'%')
	{
		size_t last=var.GetLength()-1;
		if(var.At(last)==L'*')var.SetLength(last);
		return EnumEnvVars(var,true,ParentDirectory);
	}
	return result;
}

bool EnumRegistry()
{
	if(!CheckCompletion(T_REG))
		return false;
	string key=ResumeData->pattern;
	LPCWSTR ptr=ResumeData->pattern,start=ptr;
	while(*ptr&&*ptr!=L'\\')ptr++;
	int RootKey=0;
	if(!*ptr) //дописываем rootkeys
	{
		for(int i=0;i<ARRAYSIZE(ROOTS);i++)
			if(key.Equal(ROOTS[i].Name,key.GetLength()))
			{
				if(!(Opt.AddEndSlash&CMP(T_REG)))
				{
					WCHAR tmp[21];
					lstrcpyW(tmp,ROOTS[i].Name);
					tmp[wcslen(tmp)-1]=0;
					GlobalContainer->Add(tmp,CMP(T_REG));
				}
				else GlobalContainer->Add(ROOTS[i].Name,CMP(T_REG));
			}
		return true;
	}
	else //проверка
	{
		bool NoReg=true;
		for(RootKey=0;RootKey<10;RootKey++)
			if(key.Equal(ROOTS[RootKey].Name,wcslen(ROOTS[RootKey].Name)))
			{
				NoReg=false;
				break;
			}
		if(NoReg)return false;
	}
	while(*ptr)ptr++;
	ptr--;
	while(*ptr&&*ptr!=L'\\')ptr--;
	
	string token=&ptr[1];
	key.SetLength(ptr-start);
	HKEY hKey;
	string SubKey=key.SubString(wcslen(ROOTS[RootKey].Name));
	long res=RegOpenKeyExW(ROOTS[RootKey].key,SubKey,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE|KEY_WOW64_64KEY,&hKey);
	if(res!=ERROR_SUCCESS)
	{
		StdErrMsg(res);
		return false;
	}
	DWORD NameSizeInit=0x3FFF;
	wchar_t* Name=new WCHAR[NameSizeInit];
	DWORD NameSize=NameSizeInit;
	int TickCount=GetTickCount();
	for(int i=0;;i++)
	{
		DrawWaitMessage(TickCount);
		NameSize=NameSizeInit*sizeof(WCHAR);
		if(RegEnumKeyExW(hKey,i,Name,&NameSize,0,0,0,0)!=ERROR_SUCCESS)break;
		if(token.Equal(Name,token.GetLength()))
		{
			if(Opt.AddEndSlash&CMP(T_REG))
				lstrcatW(Name,L"\\");
			GlobalContainer->Add(Name,CMP(T_REG));
		}
	}
	for(int i=0;;i++)
	{
		NameSize=NameSizeInit*sizeof(WCHAR);
		if(RegEnumValueW(hKey,i,Name,&NameSize,0,0,0,0)!=ERROR_SUCCESS)break;
		if(token.Equal(Name,token.GetLength()))
			GlobalContainer->Add(Name,CMP(T_REG));
	}
	RegCloseKey(hKey);
	return true;
}

bool EnumProcs()
{
	if(!CheckCompletion(T_PRC))
		return false;
	DWORD aProcesses[2048],cbNeeded,cProcesses;
	if(!EnumProcesses(aProcesses,sizeof(aProcesses),&cbNeeded))
		return false;
	cProcesses = cbNeeded/sizeof(DWORD);
	WCHAR szProcessName[2048]; //BUGBUG, хз как делать
	for(UINT i=0;i<cProcesses;i++)
	{
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,aProcesses[i]);
		if(hProcess)
		{
			HMODULE hMod;
			if(EnumProcessModules(hProcess,&hMod,sizeof(hMod),&cbNeeded))
			{
				//GetModuleBaseNameW(hProcess,hMod,szProcessName,ARRAYSIZE(szProcessName));
				GetModuleFileNameExW(hProcess,NULL,szProcessName,ARRAYSIZE(szProcessName));
				LPCWSTR ProcBaseName=GetFileName(szProcessName);
				if(ResumeData->pattern.Empty()||ResumeData->pattern.Equal(ProcBaseName,ResumeData->pattern.GetLength()))
				{
					string ExInfo;
					if(Opt.Hints)
					{
						ExInfo=GetMsg(MPath);
						ExInfo+=szProcessName;
						ExInfo+=L"\n";
						/*// находим адрес глобальной переменной
						LONG_PTR pfn=reinterpret_cast<LONG_PTR>(GetProcAddress(GetModuleHandleW(L"kernel32"),"GetCommandLineW"));
						LONG_PTR global=*(PLONG_PTR)(pfn+1);
						// читаем значение переменной в чужом процессе
						LONG_PTR addr;
						ReadProcessMemory(hProcess,(LPCVOID)global,&addr,sizeof(LONG_PTR),NULL);
						// извлекаем командную строку
						WCHAR cmdline[256];
						ReadProcessMemory(hProcess,(LPCVOID)addr,cmdline,sizeof(cmdline),NULL);
						*/
						DWORD sz=0;
						GetProcessCmdLine(hProcess,NULL,0,sz);
						if(sz)
						{
							sz+=sizeof(WCHAR);
							wchar_t* cmdline=(wchar_t*)malloc(sz);
							GetProcessCmdLine(hProcess,cmdline,sz,sz);
							cmdline[sz/2]=0;
							ExInfo+=GetMsg(MCmdline);
							ExInfo+=cmdline;
							free(cmdline);
						}
					}
					GlobalContainer->Add(ProcBaseName,CMP(T_PRC),ExInfo);
				}
			}
		}
	}
	return true;
}

bool EnumServices()
{
	if(!CheckCompletion(T_SVC))
		return false;
	SC_HANDLE schSCManager=OpenSCManagerW(0,0,SC_MANAGER_ENUMERATE_SERVICE);
	if(!schSCManager)
		return false;
	DWORD pcbBytesNeeded,ServicesReturned,ResumeHandle=0;
	if(EnumServicesStatusExW(schSCManager,SC_ENUM_PROCESS_INFO,SERVICE_DRIVER|SERVICE_WIN32,SERVICE_STATE_ALL,NULL,0,&pcbBytesNeeded,&ServicesReturned,&ResumeHandle,0))
	{
		LPENUM_SERVICE_STATUS_PROCESSW Services=(LPENUM_SERVICE_STATUS_PROCESSW)malloc(pcbBytesNeeded);
		if(EnumServicesStatusExW(schSCManager,SC_ENUM_PROCESS_INFO,SERVICE_DRIVER|SERVICE_WIN32,SERVICE_STATE_ALL,(LPBYTE)Services,pcbBytesNeeded,&pcbBytesNeeded,&ServicesReturned,&ResumeHandle,0))
		{
			for(DWORD i=0;i<ServicesReturned;i++)
			{
				if(ResumeData->pattern.Empty()||ResumeData->pattern.Equal(Services[i].lpServiceName,ResumeData->pattern.GetLength()))
				{
					string Hint;
					if(Opt.Hints)
					{
						Hint=GetMsg(MSvcName);
						Hint+=Services[i].lpDisplayName;
					}
					GlobalContainer->Add(Services[i].lpServiceName,CMP(T_SVC),Hint);
				}
			}
		}
		CloseServiceHandle(schSCManager);
		free(Services);
	}
	return true;
}


bool EnumPath()
{
	if(!CheckCompletion(T_PTH))
		return false;
	for(size_t i=0;i<ResumeData->pattern.GetLength();i++)
		if(IsSlash(ResumeData->pattern.At(i)))
			return false;
	string AllPath;
	apiExpandEnvironmentStrings(L"%PATH%",AllPath);
	size_t slashpos;
	if(FarName->RPos(slashpos,L'\\'))
		AllPath+=L";"+FarName->SubString(0,slashpos);

	Tokenizer t(AllPath,L";");

	/*if(Opt.Hints)
	{
		if(PathDirs)
			delete PathDirs;
		PathDirs=new PATHDIRS(t.Count());
	}
	UINT c=0;*/
	SilentEnum=true;
	while(!t.IsEnd())
	{
		string path=t.Next();
		/*if(Opt.Hints)
		{
			PathDirs->Dirs[c]=path;
			c++;
		}*/
		AddEndSlash(path);
		path+=ResumeData->pattern;
		EnumFilenamesEx(path,true,CMP(T_PTH));
	}
	SilentEnum=false;
	return true;
}

bool EnumFilenames()
{
	if(!CheckCompletion(T_FIL))
		return false;
	// пайпы пока не трогаем...
	WCHAR prefix[]=L"\\\\.\\pipe\\";
	if(ResumeData->pattern.Equal(prefix,ARRAYSIZE(prefix)-1))
		return false;
	prefix[2]=L'?';
	if(ResumeData->pattern.Equal(prefix,ARRAYSIZE(prefix)-1))
		return false;
	return EnumFilenamesEx(ResumeData->pattern,false,CMP(T_FIL));
}

bool EnumStreams()
{
	if(!CheckCompletion(T_ADS))
		return false;
	size_t scpos=0;
	if(!ResumeData->pattern.RPos(scpos,L':'))
		return false;
	if(IsSlash(ResumeData->pattern.At(scpos-1)))
		return false;

	string file=ResumeData->pattern;
	file.SetLength(scpos);
	string stream=ResumeData->pattern.SubString(scpos+1);

	typedef NTSTATUS (__stdcall *PNtQueryInformationFile)(IN HANDLE FileHandle,OUT PIO_STATUS_BLOCK IoStatusBlock,OUT PVOID FileInformation,IN ULONG Length,IN int FileInformationClass);
	PNtQueryInformationFile NtQueryInformationFile;

	typedef struct _FILE_STREAM_INFORMATION
	{
		ULONG NextEntryOffset;
		ULONG StreamNameLength;
		LARGE_INTEGER StreamSize;
		LARGE_INTEGER StreamAllocationSize;
		WCHAR StreamName[1];
	}
	FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

	HMODULE hNtDll=GetModuleHandleW(L"ntdll");
	if(hNtDll)
		NtQueryInformationFile=(PNtQueryInformationFile)GetProcAddress(hNtDll,"NtQueryInformationFile");
	if(!NtQueryInformationFile)
		return false;
	LPBYTE InfoBlock=new BYTE[64<<10];
	ZeroMemory(InfoBlock,64<<10);
	PFILE_STREAM_INFORMATION pStreamInfo=(PFILE_STREAM_INFORMATION)InfoBlock,Ptr=pStreamInfo;
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile = apiCreateFile(file,0,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,0);
	if(hFile==INVALID_HANDLE_VALUE)
		return false;

#define FileStreamInformation 22

	int res=NtQueryInformationFile(hFile,&ioStatus,InfoBlock,64<<10,FileStreamInformation);
	CloseHandle(hFile);
	if(!res)
	{
		for(;;)
		{
			
			if(*Ptr->StreamName&&Ptr->StreamName[1]!=L':') //не добавл€ем безым€нный стрим
			{
				string str(&Ptr->StreamName[1],Ptr->StreamNameLength/2-1);
				size_t eol=str.GetLength();
				if(str.At(eol-1)==L'A'&&str.At(eol-2)==L'T'&&str.At(eol-3)==L'A'&&str.At(eol-4)==L'D'&&str.At(eol-5)==L'$'&&str.At(eol-6)==L':')
					str.SetLength(eol-6);
				if(stream.Empty()||stream.Equal(str,stream.GetLength()))
				{
					string Hint;
					if(Opt.Hints)
					{
						Hint.Format(L"%I64d ",Ptr->StreamSize.QuadPart);
						Hint=GetMsg(MSize)+Hint+GetMsg(MBytes);
					}
					GlobalContainer->Add(str,CMP(T_ADS),Hint);
				}
			}
			if(!Ptr->NextEntryOffset)
				break;
			Ptr=(PFILE_STREAM_INFORMATION)((char*)Ptr+Ptr->NextEntryOffset);
		}
	}
	delete[] InfoBlock;
	return true;
}

bool EnumPipes()
{
	if(!CheckCompletion(T_PIP))
		return false;
	WCHAR prefix[]=L"\\\\.\\pipe\\*";
	if(!ResumeData->pattern.Equal(prefix,ARRAYSIZE(prefix)-2))
	{
		prefix[2]=L'?';
		if(!ResumeData->pattern.Equal(prefix,ARRAYSIZE(prefix)-2))
			return false;
	}
	WIN32_FIND_DATAW pipe;
	HANDLE hPipe=apiFindFirstFileEx(prefix,FindExInfoStandard,&pipe,FindExSearchNameMatch,0,0);
	if(hPipe==INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if(*pipe.cFileName)
		{
			string pn=ResumeData->pattern.SubString(9);
			if(!pn.At(0)||pn.Equal(pipe.cFileName,pn.GetLength()))
				GlobalContainer->Add(pipe.cFileName,CMP(T_PIP));
		}
	}
	while(FindNextFileW(hPipe,&pipe));
	return true;
}

bool ProcessPrefixStr(LPCWSTR CommandPrefix,LPCWSTR Str,LPCWSTR Name,bool CheckOnly=false)
{
	Tokenizer t(CommandPrefix,L":");
	bool ret=false;
	while(!t.IsEnd())
	{
		string prefix=t.Next();
		if(!prefix.Empty() && prefix.Equal(Str,(CheckOnly?prefix.GetLength():wcslen(Str))))
		{
			if(CheckOnly)
			{
				return (wcslen(Str)>=(int)prefix.GetLength()) && (Str[prefix.GetLength()]==L':');
			}
			prefix+=L":";
			string Hint;
			if(Opt.Hints)
			{
				Hint=GetMsg(MPlugin);
				Hint+=Name;
			}
			GlobalContainer->Add(prefix,CMP(T_PRF),Hint);
			ret=true;
		}
	}
	return ret;
}

bool GetPrefixFromModule(__in HMODULE hModule, __out string &StrPrefix)
{
	typedef void (WINAPI *GETPLUGININFO)(PluginInfo *Info);
	GETPLUGININFO pGetPluginInfo=(GETPLUGININFO)GetProcAddress(hModule,"GetPluginInfoW");
	if(pGetPluginInfo)
	{
		PluginInfo pi={0};
		pGetPluginInfo(&pi);
		if(pi.CommandPrefix)
		{
			StrPrefix=pi.CommandPrefix;
			return true;
		}
	}
	return false;
}

bool EnumPrefixes(LPCWSTR Str,bool CheckOnly=false)
{
	// oops, broken
#if 0
	if(!CheckCompletion(T_PRF)&&!CheckOnly)
		return false;
	/*if(!Opt.Prefix->Empty())
	{
		string strYAC=FSF.PointToName(Info.ModuleName);
		if(ProcessPrefixStr(*Opt.Prefix,Str,strYAC,CheckOnly)) // про себ€, любимого, не забудем :)
			return true;
	}*/
	//сначала из кеша - оттуда можно поиметь больше информации.
	//учтЄм им€ юзера
	string strValueData;
	string strRegRoot=GetRegRootString();
	SetRegRootString(strRegRoot+L"Cache");
	string PluginEntry;
	for(int i=0;;i++)
	{
		strValueData=L"";
		PluginEntry.Format(L"Plugin%d",i);
		HKEY key=OpenRegKey(PluginEntry);
		if(!key)
			break;
		CloseRegKey(key);
		if(GetRegKey(PluginEntry,L"CommandPrefix",strValueData,L"") && !strValueData.Empty())
		{
			string strModule;
			if(GetRegKey(PluginEntry,L"Name",strModule,L"") && !strModule.Empty())
				if(apiGetFileAttributes(strModule)!=INVALID_FILE_ATTRIBUTES)
				{
					if(ProcessPrefixStr(strValueData,Str,GetFileName(strModule),CheckOnly))
					{
						SetRegRootString(Info.RootKey);
						return true;
					}
				}
			}
	}
	SetRegRootString(Info.RootKey);

	DWORD cbNeeded=0;
	HMODULE hModuleInit;
	EnumProcessModules(GetCurrentProcess(),&hModuleInit,sizeof(hModuleInit),&cbNeeded);
	if(cbNeeded)
	{
		HMODULE* lphModule=new HMODULE[cbNeeded/sizeof(HMODULE)];
		EnumProcessModules(GetCurrentProcess(),lphModule,cbNeeded,&cbNeeded);
		bool ret=false;
		HMODULE h_=NULL;
		for(DWORD i=0;i<cbNeeded/sizeof(HMODULE);i++)
		{
			string strName;
			apiGetModuleFileName(lphModule[i],strName);
			string prefixes;
			if(GetPrefixFromModule(lphModule[i],prefixes))
			{
				if(ProcessPrefixStr(prefixes,Str,GetFileName(strName),CheckOnly))
				{
					ret=true;
				}
			}
		}
		delete[] lphModule;
		if(h_)
		{
			string prefixes;
			if(GetPrefixFromModule(h_,prefixes))
			{
				string strName;
				apiGetModuleFileName(h_,strName);
				if(ProcessPrefixStr(prefixes,Str,GetFileName(strName),CheckOnly))
				{
					ret=true;
				}
			}
		}
		return ret;
	}
#endif
	return false;
}

bool EnumUncItems()
{
	if(ResumeData->pattern.At(0)==L'\\'&&ResumeData->pattern.At(1)==L'\\'&&(ResumeData->pattern.At(2)==L'?'||ResumeData->pattern.At(2)==L'.') && IsSlash(ResumeData->pattern.At(3)))
	{
		string str=ResumeData->pattern.SubString(4);
		if(str.Equal(L"UNC\\"))
			return false;
		if((ResumeData->pattern.Empty()||str.Equal(L"UNC\\",str.GetLength())&&((LocalCompletion&CMP(T_SRV))||(LocalCompletion&CMP(T_SHR)))))
			GlobalContainer->Add(L"UNC\\",/*T_NMS*/0x80000000);
		if(str.Equal(L"pipe\\"))
			return false;
		if((ResumeData->pattern.Empty()||str.Equal(L"pipe\\",str.GetLength())&&LocalCompletion&CMP(T_PIP)&&(!ResumeData->prefix.Empty()||ThisState->Workspace!=W_SHELL)))
			GlobalContainer->Add(L"pipe\\",/*T_NMS*/0x80000000);
	}
	if(ResumeData->pattern.GetLength()<4&&ResumeData->pattern.At(0)==L'\\'&&ResumeData->pattern.At(1)==L'\\')
	{
		if(ResumeData->pattern.GetLength()<3||ResumeData->pattern.At(2)==L'.')GlobalContainer->Add(L".\\",0x80000000/*T_NMS*/);
		if(ResumeData->pattern.GetLength()<3||ResumeData->pattern.At(2)==L'?')GlobalContainer->Add(L"?\\",0x80000000/*T_NMS*/);
	}
	return true;
}

bool EnumVolumes()
{
	if(!CheckCompletion(T_VOL))
		return false;
	if(!(ResumeData->pattern.At(0)==L'\\'&&ResumeData->pattern.At(1)==L'\\'&&ResumeData->pattern.At(3)==L'\\'&&(ResumeData->pattern.At(2)==L'.'||ResumeData->pattern.At(2)==L'?')))
		return false;
	int slashcount=0;
	for(size_t i=0;i<ResumeData->pattern.GetLength();i++)
		if(IsSlash(ResumeData->pattern.At(i)))
			slashcount++;
	if(slashcount>3)
		return false;
	WCHAR n[50];
	HANDLE hVol=FindFirstVolumeW(n,ARRAYSIZE(n));
	int res=1;
	string vol=ResumeData->pattern.SubString(4);
	while(res)
	{
		if(vol.Equal(&n[4],vol.GetLength()))
		{
			string Hint;
			if(Opt.Hints)
				GetVolumeLabelAndFS(n,Hint);
			if(!(Opt.AddEndSlash&CMP(T_VOL)))
				n[48]=0; //уберЄм слеш
			GlobalContainer->Add(&n[4],CMP(T_VOL),Hint);
		}
		res=FindNextVolumeW(hVol,n,ARRAYSIZE(n));
	}
	FindVolumeClose(hVol);
	return true;
}

bool EnumServers()
{
	if(!CheckCompletion(T_SRV))
		return false;
	string Str=ResumeData->pattern;
	if(Str.Equal(L"\\\\?\\UNC\\",8)||Str.Equal(L"\\\\.\\UNC\\",8))
	{
		Str.LShift(6);
		wchar_t* ptr=Str.GetBuffer();
		*ptr=L'\\';
		Str.ReleaseBuffer();
	}
	if(Str.At(0)!=L'\\'||Str.At(1)!=L'\\')
		return false;
	if(Str.At(2)==L'?'||Str.At(2)==L'.')
		return false;
	for(size_t i=2;i<Str.GetLength();i++)
		if(IsSlash(Str.At(i)))
			return false;
	LPSERVER_INFO_100 Servers;
	ZeroMemory(&Servers,sizeof(Servers));
	DWORD entriesread=0,totalentries=0,resume_handle=0;
	NET_API_STATUS res=0;
	ThParams p={0,0,&res,reinterpret_cast<LONG_PTR>(&Servers),&entriesread,&totalentries,&resume_handle,0};
	DoNetEnum(&p);
	if(res==NERR_Success) 
		for(DWORD i=0;i<totalentries;i++)
		{
			string tmp=Str.SubString(2);
			if(!strcmpin(tmp,Servers[i].sv100_name,tmp.GetLength()))
			{
				string server=Servers[i].sv100_name;
				if(Opt.AddEndSlash&CMP(T_SRV))
					server+=L"\\";
				GlobalContainer->Add(server,CMP(T_SRV));
			}
		}
	NetApiBufferFree(Servers);
	return true;
}

bool EnumShares()
{
	if(!CheckCompletion(T_SHR))
		return false;
//--UNC hack-----------------------------------------------
	string Str=ResumeData->pattern;
	if(Str.Equal(L"\\\\?\\UNC\\",8) || Str.Equal(L"\\\\.\\UNC\\",8))
	{
		Str.LShift(6);
		wchar_t* ptr=Str.GetBuffer();
		*ptr=L'\\';
		Str.ReleaseBuffer();
	}
	if(Str.At(0)!=L'\\'||Str.At(1)!=L'\\')return false;
	if(Str.At(2)==L'?'||Str.At(2)==L'.')return false;
	string ServerName=Str,ShareName=L"";

	UINT si;
	for(si=2;si<ServerName.GetLength();si++)
		if(IsSlash(ServerName.At(si)))
		{
			ShareName=ServerName.SubString(si+1);
			break;
		}
	if(!ServerName.At(si))ServerName=L"";
	else ServerName.SetLength(si);

	//check sharename
	for(UINT i=0;i<ShareName.GetLength();i++)
		if(IsSlash(ShareName.At(i)))
			return false; //directory in path
	if(!ServerName.Empty())
	{
		LPSHARE_INFO_1 Shares;
		ZeroMemory(&Shares,sizeof(Shares));
retry:
		DWORD entriesread=0,totalentries=0,resume_handle=0;
		NET_API_STATUS res=0;
		ThParams p={1,ServerName,&res,reinterpret_cast<LONG_PTR>(&Shares),&entriesread,&totalentries,&resume_handle,0};
		DoNetEnum(&p);
		if(res==NERR_Success)
		{
			for(DWORD i=0;i<totalentries;i++)
				if(ShareName.Equal(Shares[i].shi1_netname,ShareName.GetLength()))
				{
					if((IsDirCmd||ResumeData->prefix.Empty())&&!((Shares[i].shi1_type==STYPE_DISKTREE)||((Shares[i].shi1_type&STYPE_SPECIAL)&&!(Shares[i].shi1_type&STYPE_IPC))))
						continue;
					string share=Shares[i].shi1_netname;
					if(Opt.AddEndSlash&CMP(T_SHR))
						share+=L"\\";
					string Hint;
					if(Opt.Hints)
					{
						Hint=GetMsg(MType);
						LPCTSTR Type;
						switch(Shares[i].shi1_type)
						{
							case STYPE_DISKTREE:Type=GetMsg(MSTYPE_DISKTREE);
							case STYPE_IPC:Type=GetMsg(MSTYPE_IPC);
							case STYPE_SPECIAL:Type=GetMsg(MSTYPE_SPECIAL);
						}
						Hint+=Type;
						Hint+=L"\n";
						Hint+=GetMsg(MDescription);
						Hint+=Shares[i].shi1_remark;
					}
					GlobalContainer->Add(share,CMP(T_SHR),Hint);
				}
			NetApiBufferFree(Shares);
		}
		else if(res==ERROR_ACCESS_DENIED)
		{
			string Ipc=ServerName+L"\\IPC$";
			int ConResult=Connect(Ipc);
			switch(ConResult)
			{
			case 0:
				return false;
			case 1:
				goto retry;
			}
			StdErrMsg(res);
		}
		else
		{
			StdErrMsg(res);
		}
	}
	return true;
}

bool EnumUsers()
{
	if(!CheckCompletion(T_USR))
		return false;

	// ToDo
	/*SID rgpSids[1024];
	LPDATAOBJECT lpDo
	LookupSids(ARRAYSIZE(rgpSids),rgpSids,&lpDo);*/

	LPUSER_INFO_10 Users;
	DWORD entriesread=0,totalentries=0,resume_handle=0;
	NET_API_STATUS res=NetUserEnum(NULL,10,0,(LPBYTE*)&Users,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resume_handle);
	if(res==NERR_Success) 
		for(DWORD i=0;i<totalentries;i++)
			if(ResumeData->pattern.Equal(Users[i].usri10_name,ResumeData->pattern.GetLength()))
			{
				string Hint;
				if(Opt.Hints)
				{
					if(*Users[i].usri10_comment)
					{
						Hint+=L"\n";
						Hint+=GetMsg(MUsrComment);
						Hint+=Users[i].usri10_comment;
					}
					if(*Users[i].usri10_full_name)
					{
						Hint+=L"\n";
						Hint+=GetMsg(MUsrFullName);
						Hint+=Users[i].usri10_full_name;
					}
				}
				GlobalContainer->Add(Users[i].usri10_name,CMP(T_USR),Hint);
			}
	NetApiBufferFree(Users);
	return true;
}

bool EnumPanelItems()
{
	if(!CheckCompletion(T_PNL))
	{
		return false;
	}
	PanelInfo pi;
	Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi);
	if(!(pi.Flags&PFLAGS_PLUGIN))
	{
		return false;
	}

	for(size_t i=0;i<pi.ItemsNumber;i++)
	{
		LPCWSTR Item;
		FarGetPluginPanelItem fgpi;
		fgpi.Size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, i, nullptr);
		fgpi.Item = reinterpret_cast<PluginPanelItem *>(malloc(fgpi.Size));
		Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, i, &fgpi);
		Item = fgpi.Item->FileName;
		if(ResumeData->pattern.Equal(Item,ResumeData->pattern.GetLength()) && lstrcmpW(Item,(L"..")))
			GlobalContainer->Add(Item,CMP(T_PNL)/*,Hint*/);
		free(fgpi.Item);
	}
	return true;
}
/*
	else if(ThisState->Workspace==W_SHELL)
	{
		strRegPath=L"\\SavedHistory";
	}
*/

bool EnumDialogHistory()
{
	//oops, broken
#if 0
	if(!CheckCompletion(T_HST) || ThisState->Workspace!=W_DIALOG)
		return false;
	string strRegRoot=GetRegRootString();
	strRegRoot.SetLength(strRegRoot.GetLength()-8); //8 - "\plugins"
	
	SetRegRootString(strRegRoot+L"\\SavedDialogHistory\\"+*DlgItemHistory);
	DWORD Size=GetRegKey(L"",L"Lines",NULL,0,0);
	if(!Size)
	{
		return false;
	}
	LPBYTE buffer=static_cast<LPBYTE>(malloc(Size));
	GetRegKey(L"",L"Lines",buffer,0,Size);
	SetRegRootString(Info.RootKey);
	for(wchar_t* ptr=reinterpret_cast<wchar_t*>(buffer);ptr-(wchar_t*)buffer<(int)(Size/sizeof(WCHAR));ptr++)
	{
		string Item=ptr;
		while(*ptr)
			ptr++;
		if(ResumeData->pattern.Equal(Item,ResumeData->pattern.GetLength()))
		{
			/*string Hint;
			if(Opt.Hints)
			{
			}
			*/
			// это пиздец, но работает неожиданно красиво
			string root;
			if(TokenVirtualRoot(ResumeData->pattern,root))
				Item=Item.SubString(root.GetLength());
			GlobalContainer->Add(Item,CMP(T_HST)/*,Hint*/);
		}
	}
	ex_free(buffer);
#endif
	return true;
}

bool EnumCmdLineHistory()
{
	//oops, broken
#if 0
	if(!CheckCompletion(T_HST) || ThisState->Workspace!=W_SHELL)
	{
		return false;
	}
	string strRegRoot=GetRegRootString();
	strRegRoot.SetLength(strRegRoot.GetLength()-8); //8 - "\plugins"
	SetRegRootString(strRegRoot+L"\\SavedHistory\\");
	DWORD Size=GetRegKey(L"",L"Lines",NULL,0,0);
	if(!Size)
	{
		SetRegRootString(Info.RootKey);
		return false;
	}
	LPBYTE buffer=static_cast<LPBYTE>(malloc(Size));
	GetRegKey(L"",L"Lines",buffer,0,Size);
	SetRegRootString(Info.RootKey);
	for(wchar_t* ptr=reinterpret_cast<wchar_t*>(buffer);ptr-(wchar_t*)buffer<(int)(Size/sizeof(wchar_t));ptr++)
	{
		string Item=ptr;
		while(*ptr)
			ptr++;
		if(ResumeData->pattern.Equal(Item,ResumeData->pattern.GetLength()))
		{
			/*
			string Hint;
			if(Opt.Hints)
			{
			}
			*/
			GlobalContainer->Add(Item,CMP(T_HST)/*,Hint*/);
		}
	}
	ex_free(buffer);
#endif
	return true;
}

bool EnumDialogComboItems()
{
	if(!CheckCompletion(T_HST) || ThisState->Workspace!=W_DIALOG)
		return false;

	
	ID=(int)Info.SendDlgMessage(ThisState->hDlg,DM_GETFOCUS,0,0);
	size_t Size=Info.SendDlgMessage(ThisState->hDlg,DM_GETDLGITEM,ID,NULL);
	FarDialogItem* di=reinterpret_cast<FarDialogItem*>(malloc(Size));
	Info.SendDlgMessage(ThisState->hDlg,DM_GETDLGITEM,ID,di);
	if(di->Type==DI_COMBOBOX)
	{
		for(size_t i=0;i<di->ListItems->ItemsNumber;i++)
		{
			string Item=di->ListItems->Items->Text;
			if(ResumeData->pattern.Equal(Item,ResumeData->pattern.GetLength()))
			{
				/*
				string Hint;
				if(Opt.Hints)
				{
				}
				*/
				GlobalContainer->Add(Item,CMP(T_HST)/*,Hint*/);
			}
		}
	}
	free(di);
	return true;
}
