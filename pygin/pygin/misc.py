import pygin

class PluginBoilerplate:
	def Msg(self, Id: int) -> str:
		return pygin.far.GetMsg(self.Guid, Id)
