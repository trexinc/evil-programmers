from pygin import far

class PluginBoilerplate:
	def Msg(self, Id: int) -> str:
		return far.GetMsg(self.Guid, Id)

class Console:
	def __enter__(self):
		far.PanelControl(far.Panels.Active, far.FileControlCommands.GetUserScreen)

	def __exit__(self, *args):
		far.PanelControl(far.Panels.Active, far.FileControlCommands.SetUserScreen)
