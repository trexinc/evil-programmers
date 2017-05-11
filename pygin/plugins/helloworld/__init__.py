import pygin

# Far Manager API, passed through SetStartupInfoW call
far = None

class HelloWorldPlugin:
	def __init__(self, info):
		info["Title"] = "Hello Python world"
		info["Author"] = "Far group"
		info["Description"] = "Python plugin very basic example"
		info["Guid"] = "31A0D11E-B9D8-4A9B-88C7-2D2983802C51"

	def GetPluginInfo(self, info):
		info["MenuString"] = "Hello Python world"
		info["Guid"] = "DAF1257B-E011-4B5A-B5DC-732581BDF3BA";

	def Open(self, info):
		far.GetUserScreen()
		print("------------")
		pygin.SayHello()
		print("------------")
		far.SetUserScreen()


def GetGlobalInfoW(info):
	global Plugin
	Plugin = HelloWorldPlugin(info)

def SetStartupInfoW(farapi):
	global far
	far = farapi

def GetPluginInfoW(info):
	return Plugin.GetPluginInfo(info)

def OpenW(info):
	return Plugin.Open(info)

def ExitFARW(info):
	global Plugin
	Plugin = None

	global far
	far = None
