#include "headers.hpp"
#pragma hdrstop

#include "guid.hpp"
#include "Console.hpp"

console::console()
{
	PsInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0, nullptr);
}

console::~console()
{
	PsInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0, nullptr);
	PsInfo.AdvControl(&MainGuid, ACTL_REDRAWALL, 0, nullptr);
}
