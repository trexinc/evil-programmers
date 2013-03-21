void PrintItems(Container& Items)
{
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
	cci.bVisible=FALSE;

	Info.PanelControl(PANEL_ACTIVE,FCTL_GETUSERSCREEN,0,NULL);
	int c=Items.Count();
	DWORD n;
	SMALL_RECT Rect;
	Info.AdvControl(YacGuid, ACTL_GETFARRECT, 0, &Rect);
	int colcount=Opt.Multicolumn?((Rect.Right-Rect.Left+1)+1)/(Items.MaxFarLen()+(Opt.ShowType?8:2)):1;
	if(!colcount)
		colcount++;
	int lines=0,passedlines=0,i=0;
	LPWORD lpAttribute=0;
	if(Opt.ShowType)
		lpAttribute=new WORD[(Rect.Right-Rect.Left+1)];
	PCHAR_INFO lpBuffer=new CHAR_INFO[(Rect.Right-Rect.Left+1)];
	COORD dwStatCoord1={0,(Rect.Bottom-Rect.Top+1)-1},
	      dwBufferSize={(Rect.Right-Rect.Left+1),1},
	      dwBufferCoord={0,0};
	COORD dwStatCoord_newFar=dwStatCoord1;
	dwStatCoord_newFar.X+=Rect.Left;
	dwStatCoord_newFar.Y+=Rect.Top;
	while(i<c) // while num of items < all
	{
		string str;
		DWORD cCharsWritten;
		if(Opt.ShowType)
		{
			ReadConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE),lpAttribute,(Rect.Right-Rect.Left+1),dwStatCoord_newFar,&cCharsWritten);
		}
		for(int j=0;j<colcount;j++)
		{
			if(Opt.Pausable&&(lines>(Rect.Bottom-Rect.Top+1)-4))
			{
				string StatMsg;
				StatMsg.Format(L"--more-- [%d/%d - %d%%]",i,c,i*100/c);
				LPWORD lpStatAttribute=new WORD[StatMsg.GetLength()];
				ReadConsoleOutputAttribute(GetStdHandle(STD_ERROR_HANDLE),lpStatAttribute,static_cast<DWORD>(wcslen(StatMsg)),dwStatCoord_newFar,&cCharsWritten);
				for(int a=0;StatMsg.At(a);a++)
					lpStatAttribute[a]=((lpStatAttribute[a]%0x10)<<4)+(lpStatAttribute[a]>>4);
				WriteConsoleOutputAttribute(GetStdHandle(STD_ERROR_HANDLE),lpStatAttribute,static_cast<DWORD>(wcslen(StatMsg)),dwStatCoord_newFar,&cCharsWritten);
				WriteConsoleOutputCharacterW(GetStdHandle(STD_ERROR_HANDLE),StatMsg,static_cast<DWORD>(wcslen(StatMsg)),dwStatCoord_newFar,&cCharsWritten);
				DWORD NumberOfEventsRead;
				INPUT_RECORD ir[1];
				ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE),ir,1,&NumberOfEventsRead);
				SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
				for(;;)
				{
					ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE),ir,1,&NumberOfEventsRead);
					bool end=false;
					if((ir[0].EventType==KEY_EVENT)&&ir[0].Event.KeyEvent.bKeyDown)
						switch(ir[0].Event.KeyEvent.wVirtualKeyCode)
						{
						case VK_SPACE:
							for(int a=0;StatMsg.At(a);a++)
								lpStatAttribute[a]=((lpStatAttribute[a]%0x10)<<4)+(lpStatAttribute[a]>>4);
							WriteConsoleOutputAttribute(GetStdHandle(STD_ERROR_HANDLE),lpStatAttribute,static_cast<DWORD>(wcslen(StatMsg)),dwStatCoord_newFar,&cCharsWritten);
							lines=0;
							end=true;
							break;
						case VK_RETURN:
							for(int a=0;StatMsg.At(a);a++)
								lpStatAttribute[a]=((lpStatAttribute[a]%0x10)<<4)+(lpStatAttribute[a]>>4);
							WriteConsoleOutputAttribute(GetStdHandle(STD_ERROR_HANDLE),lpStatAttribute,static_cast<DWORD>(wcslen(StatMsg)),dwStatCoord_newFar,&cCharsWritten);
							lines=(Rect.Bottom-Rect.Top+1)-4;
							end=true;
							break;
						case 'C':
							if((ir[0].Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED)||
							(ir[0].Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED))
							{
								for(int a=0;StatMsg.At(a);a++)
									lpStatAttribute[a]=((lpStatAttribute[a]%0x10)<<4)+(lpStatAttribute[a]>>4);
								WriteConsoleOutputAttribute(GetStdHandle(STD_ERROR_HANDLE),lpStatAttribute,static_cast<DWORD>(wcslen(StatMsg)),dwStatCoord_newFar,&cCharsWritten);
								end=true;
								i=c;
							}
							break;
						}
						if(end)
							break;
				}
				delete[] lpStatAttribute;
			}
			if(i+j>=c)break;
			wchar_t type[]=L"[---] ";
			int w=Items.MaxFarLen()+2+(Opt.ShowType?6:0);
			if(Opt.ShowType)
			{
				for(int _=0;_<5;_++)
					lpAttribute[j*w+_]++;
				if(!(Items.Item[i+j]->Type&0x10000000))
				{
					int Num=CMPNUM(Items.Item[i+j]->Type);
					if(Num!=-1)
					{
						type[1]=CmplRules[Num].name[0];
						type[2]=CmplRules[Num].name[1];
						type[3]=CmplRules[Num].name[2];
					}
				}
				else
				{
					type[1]=type[2]=type[3]=L'*';
				}

				str+=type;
			}
			string fdata=Items.Item[i+j]->Name;
			if((colcount>1)&& // не равняем пробелами, если только один столбец
						(j!=colcount-1)) //последний тоже не равняем
			{
				while(fdata.GetLength()<Items.MaxFarLen())fdata+=L" ";
				fdata+=L"  ";
			}
			str+=fdata;
		}
		SMALL_RECT ReadRegion={0,(Rect.Bottom-Rect.Top+1)-1,(Rect.Right-Rect.Left+1),(Rect.Bottom-Rect.Top+1)-1};
		ReadConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE),lpBuffer,dwBufferSize,dwBufferCoord,&ReadRegion);
		for(int k=0;k<(Rect.Right-Rect.Left+1);k++)lpBuffer[k].Char.UnicodeChar=L' ';
		WriteConsoleOutputW(GetStdHandle(STD_OUTPUT_HANDLE),lpBuffer,dwBufferSize,dwBufferCoord,&ReadRegion);
		if(i<c)
		{
			Info.AdvControl(YacGuid, ACTL_SETCURSORPOS, 0, &dwStatCoord1);
			size_t l=str.GetLength();
			WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),str,static_cast<DWORD>(l),&n,0);
			if(Opt.ShowType)
			{
				dwStatCoord_newFar.Y-=static_cast<short>(l/(Rect.Right-Rect.Left+1));
				WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE),lpAttribute,(Rect.Right-Rect.Left+1),dwStatCoord_newFar,&cCharsWritten);
				dwStatCoord_newFar.Y+=static_cast<short>(l/(Rect.Right-Rect.Left+1));
			}
			WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),L"\n",1,&n,0);
		}
		i+=colcount;
		lines++;
		passedlines++;
	}
	if(Opt.ShowType)delete[] lpAttribute;
	delete[] lpBuffer;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),L"\n",1,&n,0);
	cci.bVisible=TRUE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
	Info.PanelControl(PANEL_ACTIVE,FCTL_SETUSERSCREEN,0,NULL);
	Info.AdvControl(YacGuid,ACTL_REDRAWALL,0,0);
}
