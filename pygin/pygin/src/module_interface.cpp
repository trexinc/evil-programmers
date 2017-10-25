/*
module_interface.cpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "module.hpp"

#include "error_handling.hpp"

HANDLE WINAPI AnalyseW(const AnalyseInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->AnalyseW(Info);
	},
	[]
	{
		return HANDLE{};
	});
}

void WINAPI CloseAnalyseW(const CloseAnalyseInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->CloseAnalyseW(Info);
	},
	[]
	{
	});
}

void WINAPI ClosePanelW(const ClosePanelInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ClosePanelW(Info);
	},
	[]
	{
	});
}

intptr_t WINAPI CompareW(const CompareInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->CompareW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ConfigureW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI DeleteFilesW(const DeleteFilesInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->DeleteFilesW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

void WINAPI ExitFARW(const ExitInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ExitFARW(Info);
	},
	[]
	{
	});
}

void WINAPI FreeFindDataW(const FreeFindDataInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->FreeFindDataW(Info);
	},
	[]
	{
	});
}

intptr_t WINAPI GetFilesW(GetFilesInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetFilesW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI GetFindDataW(GetFindDataInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetFindDataW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

void WINAPI GetGlobalInfoW(GlobalInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetGlobalInfoW(Info);
	},
	[]
	{
	});
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetOpenPanelInfoW(Info);
	},
	[]
	{
	});
}

void WINAPI GetPluginInfoW(PluginInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetPluginInfoW(Info);
	},
	[]
	{
	});
}

intptr_t WINAPI MakeDirectoryW(MakeDirectoryInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->MakeDirectoryW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

HANDLE WINAPI OpenW(const OpenInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->OpenW(Info);
	},
	[]
	{
		return HANDLE{};
	});
}

intptr_t WINAPI ProcessDialogEventW(const ProcessDialogEventInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessDialogEventW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessEditorEventW(const ProcessEditorEventInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessEditorEventW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessEditorInputW(const ProcessEditorInputInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessEditorInputW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessPanelEventW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessHostFileW(const ProcessHostFileInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessHostFileW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessPanelInputW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessConsoleInputW(ProcessConsoleInputInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessConsoleInputW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessSynchroEventW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI ProcessViewerEventW(const ProcessViewerEventInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->ProcessViewerEventW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI PutFilesW(const PutFilesInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->PutFilesW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->SetDirectoryW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI SetFindListW(const SetFindListInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->SetFindListW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

void WINAPI SetStartupInfoW(const PluginStartupInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->SetStartupInfoW(Info);
	},
	[]
	{
	});
}

intptr_t WINAPI GetContentFieldsW(const GetContentFieldsInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetContentFieldsW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

intptr_t WINAPI GetContentDataW(GetContentDataInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->GetContentDataW(Info);
	},
	[]
	{
		return intptr_t{};
	});
}

void WINAPI FreeContentDataW(const GetContentDataInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		return static_cast<module*>(Info->Instance)->FreeContentDataW(Info);
	},
	[]
	{
	});
}
