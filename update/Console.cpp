#include "headers.hpp"

#include "Console.hpp"

Console::Console()
{
	Info.Control(
		INVALID_HANDLE_VALUE,
		FCTL_GETUSERSCREEN,
		0
#ifdef UNICODE		
		,NULL
#endif		
	);
}

Console::~Console()
{
	Info.Control(
		INVALID_HANDLE_VALUE,
		FCTL_SETUSERSCREEN,
		0
#ifdef UNICODE		
		,NULL
#endif
	);		
	Info.AdvControl(Info.ModuleNumber,ACTL_REDRAWALL,0);
}
