import uuid

import pygin
from pygin.farapi_types import *

class HelloWorldPlugin(object):
	def __init__(self):
		pass

	def GetGlobalInfoW(self):
		info = GlobalInfo()
		info.Title = "Hello Python world"
		info.Author = "Far group"
		info.Description = "Python plugin very basic example"
		info.Guid = uuid.UUID("{31A0D11E-B9D8-4A9B-88C7-2D2983802C51}")
		return info

	def SetStartupInfoW(self, info):
		self.far = info

	def GetPluginInfoW(self):
		info = PluginInfo()
		info.PluginMenuItems = [
			("Hello Python world", uuid.UUID("{DAF1257B-E011-4B5A-B5DC-732581BDF3BA}"))
		]

		info.PluginConfigItems = [
			("Hello Python world Settings", uuid.UUID("{BD5CD4D2-E12E-40B9-876E-C0047CF2CF38}"))
		]
		return info

	def OpenW(self, info):
		self.far.GetUserScreen()
		print("------------")
		pygin.SayHello()
		print("Menu id: " + str(info.Guid))
		print("------------")
		self.far.SetUserScreen()
		return None

	def ConfigureW(self, info):
		self.far.GetUserScreen()
		print("------------")
		print("Very configure. Wow.")
		print("Menu id: " + str(info.Guid))
		print("------------")
		self.far.SetUserScreen()
		return True

FarPluginClass = HelloWorldPlugin
