"""Far API implementation. DO NOT import this module directly."""

import uuid
import enum

NullUuid = uuid.UUID("{00000000-0000-0000-0000-000000000000}")

class PluginInfo(object):
	def __init__(self):
		# [(string, UUID), ...]
		self.DiskMenuItems = []
		self.PluginMenuItems = []
		self.PluginConfigItems = []

class OpenInfo(object):
	def __init__(self):
		self.Guid = NullUuid
		pass

class ConfigureInfo(object):
	def __init__(self):
		self.Guid = NullUuid
		pass

class ExitInfo(object):
	def __init__(self):
		pass

def GetMsg(PluginId, MsgId):
	return __GetMsg(PluginId, MsgId)

def bit(n):
	return 1 << n

class MessageFlags(enum.IntFlag):
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

def Message(PluginId, Id, Flags, HelpTopic, Title, Items, Buttons):
	return __Message(PluginId, Id, Flags, HelpTopic, Title, Items, Buttons)

class InputBoxFlags(enum.IntFlag):
	EnableEmpty      = bit(0)
	Password         = bit(1)
	ExpandEnv        = bit(2)
	NoUseLastHistory = bit(3)
	Buttons          = bit(4)
	NoAmpersand      = bit(5)
	EditPath         = bit(6)
	EditPathExec     = bit(7)

def InputBox(PluginId, Id, Title, SubTitle, HistoryName, SrcText, DestSize, HelpTopic, Flags):
	return __InputBox(PluginId, Id, Title, SubTitle, HistoryName, SrcText, DestSize, HelpTopic, Flags)

class MenuItemFlags(enum.IntFlag):
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

class MenuFlags(enum.IntFlag):
	ShowAmpersand        = bit(0)
	WrapMode             = bit(1)
	Autohighlight        = bit(2)
	ReverseAutohighlight = bit(3)
	ChangeConsoleTitle   = bit(4)

def Menu(PluginId, Id, X, Y, MaxHeight, Flags, Title, Bottom, HelpTopic, BreakKeys, BreakCode, Items):
	return __Menu(PluginId, Id, X, Y, MaxHeight, Flags, Title, Bottom, HelpTopic, BreakKeys, BreakCode, Items)

def GetUserScreen():
	return __GetUserScreen()

def SetUserScreen():
	return __SetUserScreen()
