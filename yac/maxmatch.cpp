int GetMaxMatch(LPCWSTR Str,Container::ITEM** Items,const int ItemCount,string &out,string &outmaxmatch)
{
	string maxmatch,in=Str;
	ResumeData->CleanPart=Str;
	out=L"";
	if(!ItemCount)
		return 0;

	if(in.Empty()||(IsSlash(in.At(0))&&in.GetLength()==1))
	{
		out=Str;
		return 1;
	}

	if(ItemCount==1)
	{
		string UnquotedName=Items[0]->Name;
		Unquote(UnquotedName);
		if(in.Equal(UnquotedName,in.GetLength()))
		{
			out=Items[0]->Name;
			return 2; //perfect
		}
	}

	LPCWSTR Start=in,Ptr=in;
	out=in;
	while(*Ptr)
		Ptr++;
	if((Ptr[-1]==L':'&& !IsSlash(Ptr[-2]))||(IsSlash(Ptr[-1])&&((CheckIsDir(Start)==1)||((Start[0]==L'\\')&&(Start[1]==L'\\')))))
	{
		if(ItemCount>1)
			return 1;
		else
		{
			out+=Items[0]->Name;
			return 2;
		}
	}
	do Ptr--;
	while(!IsSlash(Ptr[-1])&&(Ptr[-1]!=L':')&&(!(Ptr[-1]==L':'&&!IsSlash(Ptr[-2])&&(Ptr-2>Start)))&&(Ptr>Start));

	out.SetLength(Ptr-Start);
	
	ResumeData->CleanPart=out;
	
	bool perfectmatch=true;

	maxmatch=Items[0]->Name;
	Unquote(maxmatch);
	for(int i=1;i<ItemCount;i++)
	{
		for(int count=0;maxmatch.At(count) && Items[i]->Name.At(count);count++)
		{
			string UnquotedName=Items[i]->Name;
			Unquote(UnquotedName);
			if(CharLowerW((wchar_t*)maxmatch.At(count))!=CharLowerW((wchar_t*)UnquotedName.At(count)))
			{
				perfectmatch=false;
				maxmatch.SetLength(count);
				break;
			}
		}
	}
	if(maxmatch.Empty())
		return 0;
	if(ItemCount>1 && maxmatch.Equal(Items[0]->Name,maxmatch.GetLength()))
	{
		out+=maxmatch;
		outmaxmatch=maxmatch;
		return 1;
	}
	out+=maxmatch;
	outmaxmatch=maxmatch;
	return (int)perfectmatch+1;
}
