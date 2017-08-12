import pygin

class PluginBoilerplate:
	def __init__(self):
		pass

	def Msg(self, Id):
		return pygin.far.GetMsg(self.Guid, Id)
