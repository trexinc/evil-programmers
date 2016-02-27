#include "headers.hpp"
#include "guid.hpp"
#include "Console.hpp"

Console::Console()
{
	Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0, nullptr);
}

Console::~Console()
{
	Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0, nullptr);
	Info.AdvControl(&MainGuid,ACTL_REDRAWALL, 0, nullptr);
}
