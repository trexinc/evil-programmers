/*
far_api.cpp

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

#include "far_api.hpp"

#include "py.boolean.hpp"
#include "py.import.hpp"
#include "py.integer.hpp"
#include "py.list.hpp"
#include "py.string.hpp"
#include "py.tuple.hpp"
#include "py.uuid.hpp"

#include "py.common.hpp"

#include "error_handling.hpp"
#include "helpers.hpp"

using namespace py::literals;

namespace
{
	auto FileTimeToUI64(const FILETIME& ft)
	{
		return ULARGE_INTEGER{ ft.dwLowDateTime, ft.dwHighDateTime }.QuadPart;
	}

	template<typename T, typename converter = decltype(py::cast<T>)>
	auto list_to_vector(const py::list& List, const converter& Converter = py::cast<T>)
	{
		const auto Size = List.size();
		std::vector<T> Result;
		Result.reserve(Size);
		for (size_t i = 0; i != Size; ++i)
		{
			Result.emplace_back(Converter(List[i]));
		}
		return Result;
	}

	FarKey PyFarKeyToFarKey(const py::object& PyFarKey)
	{
		return { py::cast<WORD>(PyFarKey["VirtualKeyCode"]), py::cast<DWORD>(PyFarKey["ControlKeyState"]) };
	}

	auto get_args(const char* Name, PyObject* RawArgs, size_t Count)
	{
		const auto Args = py::cast<py::tuple>(py::object::from_borrowed(RawArgs));
		if (Args.size() != Count)
			throw MAKE_PYGIN_EXCEPTION(Name + ": wrong number of arguments (expected: "s + std::to_string(Count) + ", actual: "s + std::to_string(Args.size()) + ")"s);
		return Args;
	}

	template <typename type>
	struct get_arity: get_arity<decltype(&type::Do)>
	{
	};

	template <typename type, typename... args>
	struct get_arity<type(*)(args...)>: std::integral_constant<size_t, sizeof...(args)>
	{
	};

	template<typename type, size_t... Indexes>
	auto apply_py_tuple(const py::tuple& Args, std::index_sequence<Indexes...>)
	{
		return type::Do(Args[Indexes]...);
	}

	template<typename type>
	PyObject* Method(PyObject* Self, PyObject* RawArgs)
	{
		try
		{
			const auto Args = get_args(type::HrName, RawArgs, get_arity<type>::value);
			return apply_py_tuple<type>(Args, std::make_index_sequence<get_arity<type>::value>{}).release();
		}
		catch(const py::exception& e)
		{
			py::err::raise(far_api::get().exception(), e.what());
		}
		catch (const pygin_exception& e)
		{
			py::err::raise(far_api::get().exception(), e.what());
		}
		catch (const std::exception& e)
		{
			py::err::raise(far_api::get().exception(), e.what());
		}
		return nullptr;
	}

	template<typename T>
	py::method_definition Define()
	{
		return { T::DecoratedName, Method<T>, T::Doc };
	}

	const auto& fsf()
	{
		return far_api::get().fsf();
	}

	const auto& psi()
	{
		return far_api::get().psi();
	}
}

namespace far_api_implementation
{
#define METHOD(name, doc) \
	static constexpr auto HrName = #name; \
	static constexpr auto DecoratedName = "__" ## #name; \
	static constexpr auto Doc = doc;

	struct GetMsg
	{
		METHOD(GetMsg, "Get localised message by Id")

		static py::object Do(const py::object& PyPluginId, const py::object& PyMsgId)
		{
			const auto PluginId = py::cast<UUID>(PyPluginId);
			const auto MsgId = py::cast<intptr_t>(PyMsgId);

			return py::string(psi().GetMsg(&PluginId, MsgId));
		}
	};

	struct Message
	{
		METHOD(Message, "Show message")

		static py::object Do(const py::object& PyPluginId, const py::object& PyId, const py::object& PyFlags, const py::object& PyHelpTopic, const py::object& PyTitle, const py::object& PyItems, const py::object& PyButtons)
		{
			const auto PluginId = py::cast<UUID>(PyPluginId);
			const auto Id = py::cast<UUID>(PyId);
			const auto Flags = py::cast<FARMESSAGEFLAGS>(PyFlags);
			const auto HelpTopic = py::cast<std::wstring>(PyHelpTopic);
			const auto Title = py::cast<std::wstring>(PyTitle);
			const auto Items = list_to_vector<std::wstring>(py::cast<py::list>(PyItems));
			const auto Buttons = list_to_vector<std::wstring>(py::cast<py::list>(PyButtons));

			std::vector<const wchar_t*> AllItems;
			AllItems.reserve(1 + Items.size() + Buttons.size());
			AllItems.emplace_back(Title.data());
			std::transform(Items.cbegin(), Items.cend(), std::back_inserter(AllItems), [](const auto& i) { return i.data(); });
			std::transform(Buttons.cbegin(), Buttons.cend(), std::back_inserter(AllItems), [](const auto& i) { return i.data(); });

			const auto Result = psi().Message(&PluginId, &Id, Flags & ~FMSG_ALLINONE, HelpTopic.data(), AllItems.data(), AllItems.size(), Buttons.size());
			if (Result != -1)
				return py::integer(Result);

			return py::object::none();
		}
	};

	struct InputBox
	{
		METHOD(InputBox, "Input box")
		
		static py::object Do(const py::object& PyPluginId, const py::object& PyId, const py::object& PyTitle, const py::object& PySubTitle, const py::object& PyHistoryName, const py::object& PySrcText, const py::object& PyDestSize, const py::object& PyHelpTopic, const py::object& PyFlags)
		{
			const auto PluginId = py::cast<UUID>(PyPluginId);
			const auto Id = py::cast<UUID>(PyId);
			const auto Title = py::cast<std::wstring>(PyTitle);
			const auto SubTitle = py::cast<std::wstring>(PySubTitle);
			const auto HistoryName = py::cast<std::wstring>(PyHistoryName);
			const auto SrcText = py::cast<std::wstring>(PySrcText);
			const auto DestSize = py::cast<size_t>(PyDestSize);
			const auto HelpTopic = py::cast<std::wstring>(PyHelpTopic);
			const auto Flags = py::cast<INPUTBOXFLAGS>(PyFlags);
			std::vector<wchar_t> Buffer(DestSize);

			const auto Result = psi().InputBox(&PluginId, &Id, Title.data(), SubTitle.data(), HistoryName.data(), SrcText.data(), Buffer.data(), Buffer.size(), HelpTopic.data(), Flags);
			if (Result)
				return py::string(Buffer.data());

			return py::object::none();
		}
	};

	struct Menu
	{
		METHOD(Menu, "Menu")

		static py::object Do(const py::object& PyPluginId, const py::object& PyId, const py::object& PyX, const py::object& PyY, const py::object& PyMaxHeight, const py::object& PyFlags, const py::object& PyTitle, const py::object& PyBottom, const py::object& PyHelpTopic, const py::object& PyBreakKeys, const py::object& PyBreakCode, const py::object& PyItems)
		{
			const auto PluginId = py::cast<UUID>(PyPluginId);
			const auto Id = py::cast<UUID>(PyId);
			const auto X = py::cast<intptr_t>(PyX);
			const auto Y = py::cast<intptr_t>(PyY);
			const auto MaxHeight = py::cast<intptr_t>(PyMaxHeight);
			const auto Flags = py::cast<FARMENUFLAGS>(PyFlags);
			const auto Title = py::cast<std::wstring>(PyTitle);
			const auto Bottom = py::cast<std::wstring>(PyBottom);
			const auto HelpTopic = py::cast<std::wstring>(PyHelpTopic);

			std::vector<FarKey> OptionalBreakKeys;
			FarKey* BreakKeys = nullptr;

			if (PyBreakKeys)
			{
				OptionalBreakKeys = list_to_vector<FarKey>(py::cast<py::list>(PyBreakKeys), PyFarKeyToFarKey);
				BreakKeys = OptionalBreakKeys.data();
			}

			const auto Items = py::cast<py::list>(PyItems);
			const auto ItemsSize = Items.size();
			std::vector<FarMenuItem> MenuItems;
			MenuItems.reserve(ItemsSize);
			std::vector<std::wstring> MenuStrings;
			MenuStrings.reserve(ItemsSize);

			for(size_t i = 0; i != ItemsSize; ++i)
			{
				auto Item = Items[i];
				FarMenuItem MenuItem{};
				MenuStrings.emplace_back(py::cast<std::wstring>(Item["Text"]));
				MenuItem.Text = MenuStrings.back().data();
				MenuItem.Flags = py::cast<MENUITEMFLAGS>(Item["Flags"]);
				if (const py::object AccelKey = Item["AccelKey"])
				{
					MenuItem.AccelKey = PyFarKeyToFarKey(AccelKey);
				}
				MenuItem.UserData = py::cast<intptr_t>(Item["UserData"]);
				MenuItems.emplace_back(MenuItem);
			}

			intptr_t BreakCode = 0;
			const auto Result = psi().Menu(&PluginId, &Id, X, Y, MaxHeight, Flags, Title.data(), Bottom.data(), HelpTopic.data(), BreakKeys, &BreakCode, MenuItems.data(), MenuItems.size());
			if (PyBreakCode)
			{
				auto BreakCodeContainer = py::cast<py::list>(PyBreakCode);
				BreakCodeContainer[0] = BreakCode;
			}

			if (Result != -1)
				return py::integer(Result);

			return py::object::none();
		}
	};

	struct ShowHelp
	{
		METHOD(ShowHelp, "Show help")

		static py::object Do(const py::object& PyGuid, const py::object& PyHelpTopic, const py::object& PyFlags)
		{

			const auto HelpTopic = py::cast<std::wstring>(PyHelpTopic);
			const auto Flags = py::cast<FARHELPFLAGS>(PyFlags);

			std::wstring ModuleStr;
			GUID ModuleGuid;
			const void* ModulePtr;

			if (Flags & FHELP_GUID)
			{
				ModuleGuid = py::cast<UUID>(PyGuid);
				ModulePtr = &ModuleGuid;
			}
			else
			{
				ModuleStr = py::cast<std::wstring>(PyGuid);
				ModulePtr = &ModuleStr;
			}

			return py::boolean(psi().ShowHelp(static_cast<const wchar_t*>(ModulePtr), HelpTopic.data(), Flags) != FALSE);
		}
	};

	struct AdvControl
	{
		METHOD(AdvControl, "Advanced Control Commands")

		static py::object Do(const py::object& PyPluginId, const py::object& PyCommand, const py::object& PyParam1, const py::object& PyParam2)
		{
			const auto PluginId = py::cast<UUID>(PyPluginId);
			const auto Command = py::cast<ADVANCED_CONTROL_COMMANDS>(PyCommand);
			const auto Param1 = py::cast<intptr_t>(PyParam1);

			const auto& DefaultCall = [&]
			{
				return psi().AdvControl(&PluginId, Command, Param1, nullptr);
			};

			switch (Command)
			{
			case ACTL_SETCURRENTWINDOW:
			case ACTL_COMMIT:
			case ACTL_REDRAWALL:
			case ACTL_QUIT:
			case ACTL_PROGRESSNOTIFY:
				return py::boolean(DefaultCall() != 0);

			case ACTL_GETWINDOWCOUNT:
			case ACTL_GETFARHWND:
				return py::integer(DefaultCall());

			case ACTL_GETFARMANAGERVERSION:
				{
					VersionInfo Info;
					if (!psi().AdvControl(&PluginId, Command, Param1, &Info))
						return py::object::none();

					return far_api::type("VersionInfo")(
						Info.Major,
						Info.Minor,
						Info.Revision,
						Info.Build,
						far_api::type("VersionStage")(Info.Stage)
						);
				}

			case ACTL_WAITKEY:
				// BUGBUG
				return py::object::none();

			case ACTL_GETCOLOR:
				// BUGBUG
				return py::object::none();

			case ACTL_GETARRAYCOLOR:
				// BUGBUG
				return py::object::none();

			case ACTL_GETWINDOWINFO:
				// BUGBUG
				return py::object::none();

			case ACTL_SETARRAYCOLOR:
				// BUGBUG
				return py::object::none();

			case ACTL_SYNCHRO:
				// BUGBUG
				return py::object::none();

			case ACTL_SETPROGRESSSTATE:
				// BUGBUG
				return py::object::none();

			case ACTL_SETPROGRESSVALUE:
				// BUGBUG
				return py::object::none();

			case ACTL_GETFARRECT:
				// BUGBUG
				return py::object::none();

			case ACTL_GETCURSORPOS:
				// BUGBUG
				return py::object::none();

			case ACTL_SETCURSORPOS:
				// BUGBUG
				return py::object::none();

			case ACTL_GETWINDOWTYPE:
				{
					WindowType Type;
					if (!psi().AdvControl(&PluginId, Command, 0, &Type))
						return py::object::none();

					auto WindowTypeInstance = far_api::type("WindowType")();
					WindowTypeInstance["Type"] = far_api::type("WindowInfoType")(Type.Type);
					return WindowTypeInstance;
				}

			default:
				return py::object::none();
			}
		}
	};

	struct PanelControl
	{
		METHOD(PanelControl, "Panel Control Commands")

		static py::object Do(const py::object& PyPanel, const py::object& PyCommand, const py::object& PyParam1, const py::object& PyParam2)
		{
			const auto Panel = py::cast<HANDLE>(PyPanel);
			const auto Command = py::cast<FILE_CONTROL_COMMANDS>(PyCommand);
			const auto Param1 = py::cast<intptr_t>(PyParam1);

			const auto& DefaultCall = [&]
			{
				return psi().PanelControl(Panel, Command, Param1, nullptr);
			};

			const auto& ReceiveString = [&]() -> py::object
			{
				auto Size = psi().PanelControl(Panel, Command, 0, nullptr);
				if (!Size)
					return py::object::none();

				std::vector<wchar_t> Buffer(Size);
				for (;;)
				{
					const auto FillSize = psi().PanelControl(Panel, Command, Size, Buffer.data());
					if (FillSize == Size)
						return py::string(Buffer.data(), FillSize);

					Buffer.resize(Size = FillSize);
				}
			};

			const auto& PassString = [&]
			{
				std::wstring Str;
				if (PyParam2)
					Str = py::cast<std::wstring>(PyParam2);
				return py::boolean(psi().PanelControl(Panel, Command, Param1, const_cast<wchar_t*>(Str.data())) != 0);
			};

			switch (Command)
			{
			case FCTL_CLOSEPANEL:
			case FCTL_SETCMDLINE:
			case FCTL_INSERTCMDLINE:
				return PassString();

			case FCTL_GETCMDLINE:
			case FCTL_GETCOLUMNTYPES:
			case FCTL_GETCOLUMNWIDTHS:
			case FCTL_GETPANELFORMAT:
			case FCTL_GETPANELHOSTFILE:
			case FCTL_GETPANELPREFIX:
				return ReceiveString();

			case FCTL_UPDATEPANEL:
			case FCTL_SETVIEWMODE:
			case FCTL_SETUSERSCREEN:
			case FCTL_SETCMDLINEPOS:
			case FCTL_SETSORTMODE:
			case FCTL_SETSORTORDER:
			case FCTL_CHECKPANELSEXIST:
			case FCTL_SETNUMERICSORT:
			case FCTL_GETUSERSCREEN:
			case FCTL_ISACTIVEPANEL:
			case FCTL_BEGINSELECTION:
			case FCTL_ENDSELECTION:
			case FCTL_CLEARSELECTION:
			case FCTL_SETDIRECTORIESFIRST:
			case FCTL_SETCASESENSITIVESORT:
			case FCTL_SETACTIVEPANEL:
				return py::boolean(DefaultCall() != 0);

			case FCTL_GETPANELINFO:
				{
					PanelInfo Info;
					if (!psi().PanelControl(Panel, Command, Param1, &Info))
						return py::object::none();

					auto PanelInfoInstance = far_api::type("PanelInfo")();
					PanelInfoInstance["PluginHandle"] = py::integer(Info.PluginHandle);
					PanelInfoInstance["OwnerGuid"] = Info.OwnerGuid;
					PanelInfoInstance["Flags"] = far_api::type("PanelInfoFlags")(Info.Flags);
					PanelInfoInstance["ItemsNumber"] = Info.ItemsNumber;
					PanelInfoInstance["SelectedItemsNumber"] = Info.SelectedItemsNumber;
					PanelInfoInstance["PanelRect"] = far_api::type("Rect")(Info.PanelRect.left, Info.PanelRect.top, Info.PanelRect.right, Info.PanelRect.bottom);
					PanelInfoInstance["CurrentItem"] = Info.CurrentItem;
					PanelInfoInstance["TopPanelItem"] = Info.TopPanelItem;
					PanelInfoInstance["ViewMode"] = Info.ViewMode;
					PanelInfoInstance["PanelType"] = far_api::type("PanelInfoType")(Info.PanelType);
					PanelInfoInstance["SortMode"] = Info.SortMode < SM_COUNT? far_api::type("SortModes")(Info.SortMode) : py::integer(Info.SortMode);

					return PanelInfoInstance;
				}

			case FCTL_REDRAWPANEL:
				// BUGBUG
				return py::object::none();

			case FCTL_SETSELECTION:
				// BUGBUG
				return py::object::none();

			case FCTL_SETPANELDIRECTORY:
				// BUGBUG
				return py::object::none();

			case FCTL_GETCMDLINEPOS:
				{
					int Pos;
					if (!psi().PanelControl(Panel, Command, Param1, &Pos))
						return py::object::none();

					return py::integer(Pos);
				}

			case FCTL_SETCMDLINESELECTION:
				// BUGBUG
				return py::object::none();

			case FCTL_GETCMDLINESELECTION:
				// BUGBUG
				return py::object::none();

			case FCTL_GETPANELITEM:
			case FCTL_GETSELECTEDPANELITEM:
			case FCTL_GETCURRENTPANELITEM:
				{
					const size_t Size = psi().PanelControl(Panel, Command, Param1, nullptr);
					if (!Size)
						return py::object::none();

					std::vector<char> Buffer(Size);
					const auto PPI = reinterpret_cast<PluginPanelItem*>(Buffer.data());
					FarGetPluginPanelItem FGPPI { sizeof(FGPPI), Size, PPI };
					if (!psi().PanelControl(Panel, Command, Param1, &FGPPI))
						return py::object::none();

					auto PluginPanelItemInstance = far_api::type("PluginPanelItem")();
					const auto FileTimeType = far_api::type("FileTime");
					PluginPanelItemInstance["CreationTime"] = FileTimeType(FileTimeToUI64(PPI->CreationTime));
					PluginPanelItemInstance["LastAccessTime"] = FileTimeType(FileTimeToUI64(PPI->LastAccessTime));
					PluginPanelItemInstance["LastWriteTime"] = FileTimeType(FileTimeToUI64(PPI->LastWriteTime));
					PluginPanelItemInstance["ChangeTime"] = FileTimeType(FileTimeToUI64(PPI->ChangeTime));
					PluginPanelItemInstance["FileSize"] = PPI->FileSize;
					PluginPanelItemInstance["AllocationSize"] = PPI->AllocationSize;
					PluginPanelItemInstance["FileName"] = PPI->FileName;
					PluginPanelItemInstance["AlternateFileName"] = PPI->AlternateFileName;
					PluginPanelItemInstance["Description"] = PPI->Description;
					PluginPanelItemInstance["Owner"] = PPI->Owner;
					PluginPanelItemInstance["CustomColumnData"] = helpers::list::from_array(PPI->CustomColumnData, PPI->CustomColumnNumber, [](auto i) { return py::from(i); });
					PluginPanelItemInstance["Flags"] = far_api::type("PluginPanelItemFlags")(PPI->Flags);
					//PluginPanelItemInstance["UserData"] = PPI->UserData;
					PluginPanelItemInstance["FileAttributes"] = PPI->FileAttributes;
					PluginPanelItemInstance["NumberOfLinks"] = PPI->NumberOfLinks;
					PluginPanelItemInstance["CRC32"] = PPI->CRC32;
					//PluginPanelItemInstance["Reserved"] = PPI->Reserved;

					return PluginPanelItemInstance;
				}

			case FCTL_GETPANELDIRECTORY:
				// BUGBUG
				return py::object::none();


			default:
				return py::object::none();
			}
		}
	};

#undef METHOD

	static const py::method_definition Methods[] =
	{
		Define<GetMsg>(),
		Define<Message>(),
		Define<InputBox>(),
		Define<Menu>(),
		Define<ShowHelp>(),
		Define<AdvControl>(),
		Define<PanelControl>(),
	};
}

far_api::far_api(const PluginStartupInfo* Psi):
	m_PyMethods(far_api_implementation::Methods, std::size(far_api_implementation::Methods)),
	m_Module(py::import::import("pygin.far"_py)),
	m_Exception("pygin.far.error"),
	m_Psi(*Psi),
	m_Fsf(*Psi->FSF)
{
	m_Psi.FSF = &m_Fsf;

	// We use single instance of PSI and FSF for all plugins.
	// This is perfectly fine for FSF as it is completely static.
	// PSI, however, has some dynamic fields.
	// It is better to reset all of them to avoid any misusing.
	m_Psi.ModuleName = nullptr;
	m_Psi.Private = nullptr;
	m_Psi.Instance = nullptr;

	m_Module.add_object("error", m_Exception);
	m_Module.add_functions(m_PyMethods.get());
}

const PluginStartupInfo& far_api::psi() const
{
	return m_Psi;
}
const FarStandardFunctions& far_api::fsf() const
{
	return m_Fsf;
}

const py::object& far_api::exception() const
{
	return m_Exception;
}

const py::object& far_api::get_module() const
{
	return m_Module;
}

static std::unique_ptr<far_api> s_FarApi;

void far_api::initialise(const PluginStartupInfo* Psi)
{
	if (!s_FarApi)
		s_FarApi = std::make_unique<far_api>(Psi);
}

const far_api& far_api::get()
{
	return *s_FarApi;
}

const py::object& far_api::module()
{
	return s_FarApi->get_module();
}

py::type far_api::type(const std::string& TypeName)
{
	return py::type(s_FarApi->m_Module[TypeName]);
}

void far_api::uninitialise()
{
	s_FarApi.reset();
}
