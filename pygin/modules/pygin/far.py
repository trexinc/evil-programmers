"""
Far API implementation
"""
"""
far.py
"""
"""
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
"""

import datetime
from dataclasses import dataclass, field
import enum
import inspect
import sys
import uuid
from enum import unique
from typing import *

def __invoke_api():
	Module = sys.modules[__name__]
	Frame = sys._getframe(1)
	CallerName = Frame.f_code.co_name
	Args = inspect.getargvalues(Frame).locals
	try:
		return getattr(Module, "__" + CallerName)(*[Args[Arg] for Arg in inspect.getfullargspec(getattr(Module, CallerName)).args])
	except AttributeError as e:
		raise NameError("'{0}' cannot be called before plugin is loaded".format(CallerName))

def bit(n: int) -> int:
	return 1 << n

class IntEnum(enum.IntEnum):
	@classmethod
	def _missing_(cls, value):
		pseudo_member = int.__new__(cls, value)
		pseudo_member._name_ = "Unknown_{}".format(value)
		pseudo_member._value_ = value
		return pseudo_member

class IntFlag(enum.IntFlag):
	pass

NullUuid = uuid.UUID("{00000000-0000-0000-0000-000000000000}")

@unique
class VersionStage(IntEnum):
	Release                                         = 0
	Alpha                                           = 1
	Beta                                            = 2
	RC                                              = 3

class VersionInfo:
	def __init__(self, Major: int, Minor: int, Revision: int, Build: int, Stage: VersionStage):
		self.Major = Major
		self.Minor = Minor
		self.Revision = Revision
		self.Build = Build
		self.Stage = Stage

	def __str__(self):
		return "{0}.{1}.{2}.{3} {4}".format(self.Major, self.Minor, self.Revision, self.Build, self.Stage.name)

@unique
class PluginFlags(IntFlag):
	Default                                         = 0
	Preload                                         = bit(0)
	DisablePanels                                   = bit(1)
	Editor                                          = bit(2)
	Viewer                                          = bit(3)
	FullCmdLine                                     = bit(4)
	Dialog                                          = bit(5)

class PluginInfo:
	def __init__(self):
		self.Flags = PluginFlags.Default
		# [(string, UUID), ...]
		self.DiskMenuItems = []
		self.PluginMenuItems = []
		self.PluginConfigItems = []
		self.CommandPrefix = ""

class PanelFlags(IntFlag):
	DisableFilter           = bit(0)
	DisableSortGroups       = bit(1)
	DisableHighlighting     = bit(2)
	AddDots                 = bit(3)
	RawSelection            = bit(4)
	RealNames               = bit(5)
	ShowNamesOnly           = bit(6)
	ShowRightAlignNames     = bit(7)
	ShowPreserveCase        = bit(8)
	CompareFatTime          = bit(10)
	ExternalGet             = bit(11)
	ExternaPput             = bit(12)
	ExternalDelete          = bit(13)
	ExternalMkdir           = bit(14)
	UseAttrHighlighting     = bit(15)
	UseCrc32                = bit(16)
	UseFreeSize             = bit(17)
	Shortcut                = bit(18)

class OpenPanelInfo:
	def __init__(self):
		self.Flags = PanelFlags(0)
		self.HostFile = None
		self.CurDir = None
		self.Format = None
		self.PanelTitle = None

@unique
class OpenFrom(IntEnum):
	Unknown                                         = -1
	LeftDiskMenu                                    = 0
	PluginsMenu                                     = 1
	FindList                                        = 2
	Shortcut                                        = 3
	CommandLine                                     = 4
	Editor                                          = 5
	Viewer                                          = 6
	Filepanel                                       = 7
	Dialog                                          = 8
	Analyse                                         = 9
	RightDiskMenu                                   = 10
	FromMacro                                       = 11
	LuaMacro                                        = 100

class OpenCommandLineInfo:
	def __init__(self):
		self.CommandLine = ""

@unique
class FarMacroVarType(IntEnum):
	Unknown                                         = 0
	Integer                                         = 1
	String                                          = 2
	Double                                          = 3
	Boolean                                         = 4
	Binary                                          = 5
	Pointer                                         = 6
	Nil                                             = 7
	Array                                           = 8
	Panel                                           = 9

class FarMacroValue:
	def __init__(self):
		self.Type = FarMacroVarType.Unknown
		self.Value = None

class OpenMacroInfo:
	def __init__(self):
		# [FarMacroValue, ...]
		self.Values = []

@unique
class OperationModes(IntFlag):
	Default                                         = 0
	Silent                                          = bit(0)
	Find                                            = bit(1)
	View                                            = bit(2)
	Edit                                            = bit(3)
	TopLevel                                        = bit(4)
	Descr                                           = bit(5)
	QuickView                                       = bit(6)
	PgDn                                            = bit(7)
	Commands                                        = bit(8)

class AnalyseInfo:
	def __init__(self):
		self.FileName = ""
		self.Buffer = bytes()
		self.OpMode = OperationModes.Default

@unique
class OpenShortcutFlags(IntFlag):
	Default = 0
	Active  = bit(0)

class OpenShortcutInfo:
	def __init__(self):
		self.HostFile = ""
		self.ShortcutData = ""
		self.Flags = OpenShortcutFlags.Default

class OpenDlgPluginData:
	def __init__(self):
		self.Dialog = 0

class OpenInfo:
	def __init__(self):
		self.OpenFrom = OpenFrom.Unknown
		self.Guid = NullUuid
		self.Data = None
		pass

class ConfigureInfo:
	def __init__(self):
		self.Guid = NullUuid
		pass

class ExitInfo:
	def __init__(self):
		pass

def GetMsg(PluginId: uuid, MsgId: int) -> str:
	return __invoke_api()

@unique
class MessageFlags(IntFlag):
	Default                                         = 0
	Warning                                         = bit(0)
	ErrorType                                       = bit(1)
	KeepBackground                                  = bit(2)
	LeftAlign                                       = bit(3)
	ButtonOk                                        = bit(16) * 1
	ButtonOkCancel                                  = bit(16) * 2
	ButtonAbortRertyIgnore                          = bit(16) * 3
	ButtonYesNo                                     = bit(16) * 4
	ButtonYesNoCancel                               = bit(16) * 5
	ButtonRetryCancel                               = bit(16) * 6

def Message(PluginId: uuid, Id: uuid, Flags: MessageFlags, HelpTopic: str, Title: str, Items: list, Buttons: list) -> Union[int, None]:
	return __invoke_api()

@unique
class InputBoxFlags(IntFlag):
	Default                                         = 0
	EnableEmpty                                     = bit(0)
	Password                                        = bit(1)
	ExpandEnv                                       = bit(2)
	NoUseLastHistory                                = bit(3)
	Buttons                                         = bit(4)
	NoAmpersand                                     = bit(5)
	EditPath                                        = bit(6)
	EditPathExec                                    = bit(7)

def InputBox(PluginId: uuid, Id: uuid, Title: str, SubTitle: str, HistoryName: str, SrcText: str, DestSize: int, HelpTopic: str, Flags: InputBoxFlags) -> Union[str, None]:
	return __invoke_api()

@unique
class MenuItemFlags(IntFlag):
	Default                                         = 0
	Selected                                        = bit(16)
	Checked                                         = bit(17)
	Separator                                       = bit(18)
	Disabled                                        = bit(19)
	Grayed                                          = bit(20)
	Hidden                                          = bit(21)

class FarKey:
	def __init__(self, VirtualKeyCode: int, ControlKeyState: int):
		self.VirtualKeyCode = VirtualKeyCode
		self.ControlKeyState = ControlKeyState

class MenuItem:
	def __init__(self, Text: str, Flags: MenuItemFlags = 0, AccelKey: FarKey = None, UserData: int = 0):
		self.Text = Text
		self.Flags = Flags
		self.AccelKey = AccelKey
		self.UserData = UserData

@unique
class MenuFlags(IntFlag):
	Default                                         = 0
	ShowAmpersand                                   = bit(0)
	WrapMode                                        = bit(1)
	Autohighlight                                   = bit(2)
	ReverseAutohighlight                            = bit(3)
	ChangeConsoleTitle                              = bit(4)

def Menu(PluginId: uuid, Id: uuid, X: int, Y: int, MaxHeight: int, Flags: MenuFlags, Title: str, Bottom: str, HelpTopic: str, BreakKeys: list, BreakCode, Items: list) -> Union[int, None]:
	return __invoke_api()

@unique
class HelpFlags(IntFlag):
	# enum part
	SelfHelp                                        = 0
	FarHelp                                         = bit(0)
	CustomFile                                      = bit(1)
	CustomPath                                      = bit(2)
	Guid                                            = bit(3)

	#flags part
	UseContents = bit(30)
	NoShowError = bit(31)

def ShowHelp(ModuleName: str, HelpTopic: str, Flags: HelpFlags) -> bool:
	return __invoke_api()

@unique
class AdvancedControlCommands(IntEnum):
	GetFarManagerVersion                            = 0
	WaitKey                                         = 2
	GetColor                                        = 3
	GetArrayColor                                   = 4
	GetWindowInfo                                   = 6
	GetWindowCount                                  = 7
	SetCurrentWindow                                = 8
	Commit                                          = 9
	GetFarHwnd                                      = 10
	SetArrayColor                                   = 16
	RedrawAll                                       = 19
	Synchro                                         = 20
	SetProgressState                                = 21
	SetProgressValue                                = 22
	Quit                                            = 23
	GetFarRect                                      = 24
	GetCursorPos                                    = 25
	SetCursorPos                                    = 26
	ProgressNotify                                  = 27
	GetWindowType                                   = 28

@unique
class WindowInfoType(IntEnum):
	Unknown                                         = -1
	Desktop                                         = 0
	Panels                                          = 1
	Viewer                                          = 2
	Editor                                          = 3
	Dialog                                          = 4
	Vmenu                                           = 5
	Help                                            = 6
	Combobox                                        = 7

class WindowType:
	def __init__(self):
		self.Type = WindowInfoType.Unknown

def AdvControl(PluginId: uuid, Command: AdvancedControlCommands, Param1: int = 0, Param2 = None):
	return __invoke_api()

@unique
class FileControlCommands(IntEnum):
	ClosePanel                                      = 0,
	GetPanelInfo                                    = 1,
	UpdatePanel                                     = 2,
	RedrawPanel                                     = 3,
	GetCmdLine                                      = 4,
	SetCmdLine                                      = 5,
	SetSelection                                    = 6,
	SetViewMode                                     = 7,
	InsertCmdLine                                   = 8,
	SetUserScreen                                   = 9,
	SetPanelDirectory                               = 10,
	SetCmdLinePos                                   = 11,
	GetCmdLinePos                                   = 12,
	SetSortMode                                     = 13,
	SetSortOrder                                    = 14,
	SetCmdLineSelection                             = 15,
	GetCmdLineSelection                             = 16,
	CheckPanelsExist                                = 17,
	SetNumericSort                                  = 18,
	GetUserScreen                                   = 19,
	IsActivePanel                                   = 20,
	GetPanelItem                                    = 21,
	GetSelectedPanelItem                            = 22,
	GetCurrentPanelItem                             = 23,
	GetPanelDirectory                               = 24,
	GetColumnTypes                                  = 25,
	GetColumnWidths                                 = 26,
	BeginSelection                                  = 27,
	EndSelection                                    = 28,
	ClearSelection                                  = 29,
	SetDirectoriesFirst                             = 30,
	GetPanelFormat                                  = 31,
	GetPanelHostFile                                = 32,
	SetCaseSensitiveSort                            = 33,
	GetPanelPrefix                                  = 34,
	SetActivePanel                                  = 35,

@unique
class Panels(IntEnum):
	Active = -1
	Passive = -2

@unique
class SortModes(IntEnum):
	Default                                         =  0,
	Unsorted                                        =  1,
	Name                                            =  2,
	Extension                                       =  3,
	ModificationTime                                =  4,
	CreationTime                                    =  5,
	AccessTime                                      =  6,
	Size                                            =  7,
	Description                                     =  8,
	Owner                                           =  9,
	CompressedSize                                  = 10,
	NumberOfLinks                                   = 11,
	NumberOfStreams                                 = 12,
	StreamsSize                                     = 13,
	FullName                                        = 14,
	ChangeTime                                      = 15,

@unique
class PanelInfoFlags(IntFlag):
	Default                                         = 0
	ShowHidden                                      = bit(0)
	Highlight                                       = bit(1)
	ReverseSortOrder                                = bit(2)
	UseSortGroups                                   = bit(3)
	SelectedFirst                                   = bit(4)
	RealNames                                       = bit(5)
	NumericSort                                     = bit(6)
	PanelLeft                                       = bit(7)
	DirectoriesFirst                                = bit(8)
	UseCRC32                                        = bit(9)
	CaseSensitiveSort                               = bit(10)
	Plugin                                          = bit(11)
	Visible                                         = bit(12)
	Focus                                           = bit(13)
	AlternativeNames                                = bit(14)
	Shortcut                                        = bit(15)

@unique
class PanelInfoType(IntEnum):
	Unknown                                         = -1
	FilePanel                                       = 0
	TreePanel                                       = 1
	QViewPanel                                      = 2
	InfoPanel                                       = 3

class Rect:
	def __init__(self, left: int, top: int, right: int, bottom: int):
		self.left = left
		self.top = top
		self.right = right
		self.bottom = bottom

	def __repr__(self):
		return "Rect(left: {0}, top: {1}, right: {2}, bottom: {3})".format(self.left, self. top, self.right, self.bottom)

class PanelInfo:
	def __init__(self):
		self.PluginHandle = 0
		self.OwnerGuid = NullUuid
		self.Flags = PanelInfoFlags.Default
		self.ItemsNumber = 0
		self.SelectedItemsNumber = 0
		self.PanelRect = Rect(0, 0, 0, 0)
		self.CurrentItem = 0
		self.TopPanelItem = 0
		self.ViewMode = 0
		self.PanelType = PanelInfoType.Unknown
		self.SortMode = SortModes.Default

_EPOCH_AS_FILETIME = 116444736000000000  # January 1, 1970 as MS file time
_HUNDREDS_OF_NANOSECONDS = 10000000

class FileTime(datetime.datetime):
	def __new__(self, Value = 0):
		return datetime.datetime.utcfromtimestamp((max(Value, _EPOCH_AS_FILETIME) - _EPOCH_AS_FILETIME) / _HUNDREDS_OF_NANOSECONDS)

	def value(self):
		return int(self.replace(tzinfo=datetime.timezone.utc).timestamp() * _HUNDREDS_OF_NANOSECONDS + _EPOCH_AS_FILETIME)

@unique
class PluginPanelItemFlags(IntFlag):
	Default                                         = 0
	Selected                                        = bit(30)
	ProcessDescription                              = bit(31)

@dataclass
class PluginPanelItem:
	CreationTime: FileTime = FileTime()
	LastAccessTime: FileTime = FileTime()
	LastWriteTime: FileTime = FileTime()
	ChangeTime: FileTime = FileTime()
	FileSize: int = 0
	AllocationSize = 0
	FileName: str = ""
	AlternateFileName: str = ""
	Description: str = ""
	Owner: str = ""
	CustomColumnData: List[Any] = field(default_factory=list)
	Flags: PluginPanelItemFlags = PluginPanelItemFlags.Default
	#UserData = UserDataItem()
	FileAttributes: int = 0 # See `stat.*` and Windows API
	NumberOfLinks: int = 0
	CRC32: int = 0
	#Reserved

class PanelDirectory:
	def __init__(self):
		self.Name = ""
		self.Param = ""
		self.PluginId = NullUuid
		self.File = ""

class SetDirectoryInfo:
	def __init__(self):
		self.Panel = None
		self.Dir = ""

class CmdLineSelect:
	def __init__(self):
		self.SelStart = 0
		self.SelEnd = 0

def PanelControl(Panel: int, Command: FileControlCommands, Param1: int = 0, Param2 = None):
	return __invoke_api()

class GetFindDataInfo:
	def __init__(self):
		self.PanelItems: List[PluginPanelItem] = []
		self.OpMode = OperationModes.Default
