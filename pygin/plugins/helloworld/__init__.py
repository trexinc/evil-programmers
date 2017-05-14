import uuid

import pygin
from pygin.farapi_types import *

class HelloWorldPlugin(object):
	Title = "Hello Python world"
	Author = "Far group"
	Description = "Python plugin very basic example"
	Guid = uuid.UUID("{31A0D11E-B9D8-4A9B-88C7-2D2983802C51}")

	def __init__(self, far):
		self.far = far

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
