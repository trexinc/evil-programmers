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

class MessageFlags(enum.IntFlag):
	Warning                = 0x0000000000000001
	ErrorType              = 0x0000000000000002
	KeepBackground         = 0x0000000000000004
	LeftAlign              = 0x0000000000000008
	ButtonOk               = 0x0000000000010000
	ButtonOkCancel         = 0x0000000000020000
	ButtonAbortRertyIgnore = 0x0000000000030000
	ButtonYesNo            = 0x0000000000040000
	ButtonYesNoCancel      = 0x0000000000050000
	ButtonRetryCancel      = 0x0000000000060000

def Message(PluginId, Id, Flags, HelpTopic, Title, Items, Buttons):
	return __Message(PluginId, Id, Flags, HelpTopic, Title, Items, Buttons)

def GetUserScreen():
	return __GetUserScreen()

def SetUserScreen():
	return __SetUserScreen()
