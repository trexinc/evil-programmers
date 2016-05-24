#include "headers.hpp"
#include "guid.hpp"
#include "Console.hpp"

Console::Console()
{
	PsInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0, nullptr);
}

Console::~Console()
{
	PsInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0, nullptr);
	PsInfo.AdvControl(&MainGuid,ACTL_REDRAWALL, 0, nullptr);
}
