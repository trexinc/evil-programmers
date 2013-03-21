void ParseCommandLine(string& prefix,string& rootcmd,string& pattern,string& postfix)
{
	string cmdline=ThisState->line;
	size_t CursorPos=ThisState->pos;

	for(int i=0;cmdline.At(i);i++)
		if((cmdline.At(i)==L'"')&&(cmdline.At(i+1)==L'"')&&(cmdline.At(i+2))&&(cmdline.At(i+2)!=L' '))
			return;

	prefix=L"";
	postfix=L"";
	rootcmd=L"";
	pattern=L"";
	
	postfix=cmdline.SubString(CursorPos);
	cmdline.SetLength(CursorPos);

	size_t I=CursorPos-1;
	if(I&&IsSlash(cmdline.At(I)))
		I--;
	if(I&&cmdline.At(I)==L'"')
	{
		do
			I--;
		while(I&&cmdline.At(I)!=L'"');
	}
	else if(wcschr(cmdline,L'"'))
	{
		if(!IsQuotePair(cmdline))
			while(I&&cmdline.At(I)!=L'"')I--;
		else
		{
			bool q=false;
			while(I)
			{
				if(!q&&IsQuotedSymbol(cmdline.At(I-1)))
					break;
				if(q&&cmdline.At(I)==L'"')
					break;
				if(cmdline.At(I)==L'"')
					q=true;
				I--;
			}
		}
	}
	else
		for(;;)
		{
			if(CmdSeparator(cmdline.At(I)))
			{
				I++;
				break;
			}
			if(!I)
				break;
			I--;
		}
	pattern=cmdline;
	pattern.LShift(I);
	pattern.SetLength(CursorPos-I);
	NeedFirstQuote=(pattern.At(0)==L'"');
	Unquote(pattern);
	if(I)
	{
		size_t n;

		if(cmdline.At(0)==L'"')
		{
			for(n=1;n<cmdline.GetLength()&&cmdline.At(n)!=L'"';n++)
				rootcmd.Append(cmdline.At(n));
			rootcmd.SetLength(n-1);
		}
		else
		{
			for(n=0;n<cmdline.GetLength()&&!IsQuotedSymbol(cmdline.At(n));n++)
				rootcmd.Append(cmdline.At(n));
			rootcmd.SetLength(n);
		}

		prefix=cmdline;
		prefix.SetLength(I);

		// стек "промежуточных" команд:
		// цель - возможность задать разное дополнение для, например, svn commit и svn checkout
		string Keys=prefix;
		Unquote(Keys);
		Keys=Keys.SubString(rootcmd.GetLength());

		RemoveExternalSpaces(Keys);
		if(!Keys.Empty())
			PushMultiString(Keys);

		LPCWSTR p=rootcmd;
		LPCWSTR ptr=rootcmd;
		while(*p)
			p++;
		while(p>ptr&&!IsSlash(*p))
			p--;
		if(p>ptr)
			p++;
		string tmp=p;
		rootcmd=tmp;
	}
}
