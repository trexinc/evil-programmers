"""Far API implementation. DO NOT import this module directly."""

import uuid

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
