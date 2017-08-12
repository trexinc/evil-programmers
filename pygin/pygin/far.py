"""Far API implementation"""

import sys
import uuid
import enum
from enum import unique

def dynamic(function):
	def function_wrapper(*args):
		try:
			return getattr(sys.modules[__name__], "__" + function.__name__)(*args)
		except AttributeError as e:
			raise NameError("'{0}' cannot be called before plugin is loaded".format(function.__name__))
	return function_wrapper

def bit(n):
	return 1 << n

NullUuid = uuid.UUID("{00000000-0000-0000-0000-000000000000}")

@unique
class VersionStage(enum.IntEnum):
	Release = 0
	Alpha   = 1
	Beta    = 2
	RC      = 3

class VersionInfo:
	def __init__(self, Major, Minor, Revision, Build, Stage):
		self.Major = Major
		self.Minor = Minor
		self.Revision = Revision
		self.Build = Build
		self.Stage = Stage

@unique
class PluginFlags(enum.IntFlag):
	Default        = 0
	Preload        = bit(0)
	DisablePanels  = bit(1)
	Editor         = bit(2)
	Viewer         = bit(3)
	FullCmdLine    = bit(4)
	Dialog         = bit(5)

class PluginInfo:
	def __init__(self):
		self.Flags = PluginFlags.Default
		# [(string, UUID), ...]
		self.DiskMenuItems = []
		self.PluginMenuItems = []
		self.PluginConfigItems = []
		self.CommandPrefix = ""

@unique
class OpenFrom(enum.IntEnum):
	Unknown       = -1
	LeftDiskMenu  = 0
	PluginsMenu   = 1
	FindList      = 2
	Shortcut      = 3
	CommandLine   = 4
	Editor        = 5
	Viewer        = 6
	Filepanel     = 7
	Dialog        = 8
	Analyse       = 9
	RightDiskMenu = 10
	FromMacro     = 11
	LuaMacro      = 100

class OpenCommandLineInfo:
	def __init__(self):
		self.CommandLine = ""

@unique
class FarMacroVarType(enum.IntEnum):
	Unknown = 0
	Integer = 1
	String  = 2
	Double  = 3
	Boolean = 4
	Binary  = 5
	Pointer = 6
	Nil     = 7
	Array   = 8
	Panel   = 9

class FarMacroValue:
	def __init__(self):
		self.Type = FarMacroVarType.Unknown
		self.Value = None

class OpenMacroInfo:
	def __init__(self):
		# [FarMacroValue, ...]
		self.Values = []

@unique
class OperationModes(enum.IntFlag):
	Default   = 0
	Silent    = bit(0)
	Find      = bit(1)
	View      = bit(2)
	Edit      = bit(3)
	TopLevel  = bit(4)
	Descr     = bit(5)
	QuickView = bit(6)
	PgDn      = bit(7)
	Commands  = bit(8)

class AnalyseInfo:
	def __init__(self):
		self.FileName = ""
		self.Buffer = bytes()
		self.OpMode = OperationModes.Default

@unique
class OpenShortcutFlags(enum.IntFlag):
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

@dynamic
def GetMsg(PluginId, MsgId): pass

@unique
class MessageFlags(enum.IntFlag):
	Default                = 0
	Warning                = bit(0)
	ErrorType              = bit(1)
	KeepBackground         = bit(2)
	LeftAlign              = bit(3)
	ButtonOk               = bit(16) * 1
	ButtonOkCancel         = bit(16) * 2
	ButtonAbortRertyIgnore = bit(16) * 3
	ButtonYesNo            = bit(16) * 4
	ButtonYesNoCancel      = bit(16) * 5
	ButtonRetryCancel      = bit(16) * 6

@dynamic
def Message(PluginId, Id, Flags, HelpTopic, Title, Items, Buttons): pass

@unique
class InputBoxFlags(enum.IntFlag):
	Default          = 0
	EnableEmpty      = bit(0)
	Password         = bit(1)
	ExpandEnv        = bit(2)
	NoUseLastHistory = bit(3)
	Buttons          = bit(4)
	NoAmpersand      = bit(5)
	EditPath         = bit(6)
	EditPathExec     = bit(7)

@dynamic
def InputBox(PluginId, Id, Title, SubTitle, HistoryName, SrcText, DestSize, HelpTopic, Flags): pass

@unique
class MenuItemFlags(enum.IntFlag):
	Default    = 0
	Selected   = bit(16)
	Checked    = bit(17)
	Separator  = bit(18)
	Disabled   = bit(19)
	Grayed     = bit(20)
	Hidden     = bit(21)

class FarKey:
	def __init__(self, VirtualKeyCode, ControlKeyState):
		self.VirtualKeyCode = VirtualKeyCode
		self.ControlKeyState = ControlKeyState

class MenuItem:
	def __init__(self, Text, Flags = 0, AccelKey = None, UserData = 0):
		self.Text = Text
		self.Flags = Flags
		self.AccelKey = AccelKey
		self.UserData = UserData

@unique
class MenuFlags(enum.IntFlag):
	Default              = 0
	ShowAmpersand        = bit(0)
	WrapMode             = bit(1)
	Autohighlight        = bit(2)
	ReverseAutohighlight = bit(3)
	ChangeConsoleTitle   = bit(4)

@dynamic
def Menu(PluginId, Id, X, Y, MaxHeight, Flags, Title, Bottom, HelpTopic, BreakKeys, BreakCode, Items): pass

@unique
class HelpFlags(enum.IntFlag):
	# enum part
	SelfHelp    = 0
	FarHelp     = bit(0)
	CustomFile  = bit(1)
	CustomPath  = bit(2)
	Guid        = bit(3)

	#flags part
	UseContents = bit(30)
	NoShowError = bit(31)

@dynamic
def ShowHelp(ModuleName, HelpTopic, Flags): pass

@dynamic
def GetUserScreen(): pass

@dynamic
def SetUserScreen(): pass
