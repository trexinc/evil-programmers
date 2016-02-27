void Rebuild()
{
	Clean();
	SStack->flush();
	ResumeData->Clean();
	ParseCommandLine(ResumeData->prefix,ResumeData->rootcmd,ResumeData->pattern,ResumeData->postfix);
	
	if(ResumeData->prefix.Empty())
	{
		size_t Pos=0;
		if(ResumeData->pattern.Pos(Pos,':'))
		{
			if(EnumPrefixes(ResumeData->pattern,true))
			{
				ResumeData->prefix=ResumeData->pattern;
				ResumeData->prefix.SetLength(Pos+1);
				ResumeData->rootcmd=ResumeData->prefix;
				ResumeData->pattern.LShift(Pos+1);
			}
		}
	}
	else
	{
		size_t Pos=0;
		if(ResumeData->prefix.Pos(Pos,':'))
		{
			string tmp=ResumeData->prefix.SubString(0,Pos+1);
			if(EnumPrefixes(tmp,true))
			{
				tmp=ResumeData->prefix.SubString(Pos+1);

				//ResumeData->prefix.SetLength(Pos+1);
				//ResumeData->rootcmd=ResumeData->prefix;
				ResumeData->rootcmd=ResumeData->prefix.SubString(0,Pos+1);
				//ResumeData->prefix=tmp+ResumeData->prefix;
				RemoveExternalSpaces(tmp);
				if(!tmp.Empty())
				{
					PushMultiString(tmp);
				}
				//SStack->push(tmp);
			}
		}
	}

	if(GlobalContainer)
	{
		delete GlobalContainer;
	}
	GlobalContainer=new Container;
	IsDirCmd=CheckIsDirCmd(ResumeData->rootcmd);

	GetLocal();

	EnumAliases();
	EnumSysAliases();
	EnumPrefixes(ResumeData->pattern);
	EnumLogicalDrives();
	EnumRegistry();
	EnumProcs();
	EnumServices();
	EnumComSpec();
	EnumStreams();
	EnumPipes();
	EnumUncItems();
	EnumVolumes();
	EnumServers();
	EnumShares();
	EnumEnvVars(ResumeData->pattern);
	EnumFilenames();
	EnumPath();
	EnumUsers();
	EnumPanelItems();
	EnumDialogHistory();
	EnumCmdLineHistory();
	EnumDialogComboItems();
	//EnumKeys(ResumeData->pattern);

	GlobalContainer->Sort();
}

void Complete(bool Reverse=false)
{
	if(IsStateChanged())
	{
		Rebuild();

		string maxmatch;
		int match=GetMaxMatch(ResumeData->pattern,GlobalContainer->Item,GlobalContainer->Count(),ResumeData->result,maxmatch);
		switch(match)
		{
		case 0: //нифига не нашли
			{
				PlayBeep(Opt.Beep);
				ResetState();
			}
			break;
		case 1: //нашли, причем много
		case 2: //нашли то, что надо
			{
				string result=ResumeData->result;
				QuoteLeft(result);
				if(match==2)
				{
					QuoteRight(result);
					if(Opt.AddSpace&&(CheckIsDir(result)!=1))
					{
						result+=L" ";
					}
				}
				string final=ResumeData->prefix+result;
				ThisState->pos=final.GetLength();
				if(Opt.SaveTail)
				{
					final+=ResumeData->postfix;
				}
				ThisState->line=final;
				Commit(final,ThisState->pos);
				if(match==1)
				{
					// для far /[e|v]
					DWORD ShowResults=Opt.ShowResults;
					if(ShowResults && !Info.PanelControl(PANEL_ACTIVE,FCTL_CHECKPANELSEXIST,0,NULL))
					{
						ShowResults=1;
					}
					switch(ShowResults)
					{
					case 0: //не показываем
						break;
					case 1: //диалог
						{
							bool NoQuote=false;
							if(ShowDlgMatches(ResumeData->result,maxmatch,Reverse,NoQuote))
							{
								result=ResumeData->result;
								if(!NoQuote)
								{
									QuoteLeft(result);
									QuoteRight(result);
								}
								final=ResumeData->prefix+result;
								if(Opt.AddSpace&&(CheckIsDir(ResumeData->result)!=1))
								{
									final+=L" ";
								}
								ThisState->pos=final.GetLength();
								if(Opt.SaveTail)
								{
									final+=ResumeData->postfix;
								}
								Commit(final,ThisState->pos);
							}
							Clean();
						}
						break;
					case 2: //stdout
						PrintItems(*GlobalContainer);
						break;
					}
				}
				else
				{
					ResetState();
				}
			}
			break;
		}
	}
	else
	{
		Reverse?ResumeData->ItemIndex--:ResumeData->ItemIndex++;

		if(ResumeData->ItemIndex>=GlobalContainer->Count())
		{
			ResumeData->ItemIndex=0;
		}
		else if(ResumeData->ItemIndex<0)
		{
			ResumeData->ItemIndex=GlobalContainer->Count()-1;
		}

		string result=ResumeData->CleanPart+GlobalContainer->Item[ResumeData->ItemIndex]->Name;
		ThisState->selstart=ResumeData->prefix.GetLength()+ResumeData->result.GetLength();
		ThisState->selend=ResumeData->prefix.GetLength()+ResumeData->CleanPart.GetLength();
		if(QuoteLeft(result))
		{
			ThisState->selstart++;
			ThisState->selend++;
		}
		ThisState->selend+=GlobalContainer->Item[ResumeData->ItemIndex]->Name.GetLength();
		if(QuoteRight(result))
		{
			ThisState->selend++;
		}
		string final=ResumeData->prefix+result;
		size_t NewCursorPos=final.GetLength();
		if(Opt.SaveTail)
		{
			final+=ResumeData->postfix;
		}
		
		ThisState->pos=NewCursorPos;
		ThisState->line=final;

		Commit(final,NewCursorPos,static_cast<int>(ThisState->selstart),static_cast<int>(ThisState->selend));
	}
}