class PluginBoilerplate:
	def __init__(self, far):
		self.far = far

	def Msg(self, Id):
		return self.far.GetMsg(self.Guid, Id)
