from pygin import far

class PluginBoilerplate:
	def Msg(self, Id: int) -> str:
		return far.GetMsg(self.Guid, Id)

class Console:
	def __enter__(self):
		self.__cmd(far.FileControlCommands.GetUserScreen)

	def __exit__(self, *args):
		self.__cmd(far.FileControlCommands.SetUserScreen)

	def __cmd(self, Command):
		far.PanelControl(far.Panels.Active, Command)
