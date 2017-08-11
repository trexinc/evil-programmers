import uuid
import enum

import pygin

@enum.unique
class lng(enum.IntEnum):
	PluginMenuItem     = 0
	PluginDiskMenuItem = 1
	PluginConfigItem   = 2
	TheMessage         = 3


class HelloWorldPlugin(pygin.PluginBoilerplate):
	Title = "Hello Python world"
	Author = "Far group"
	Description = "Python plugin very basic example"
	Guid = uuid.UUID("{31A0D11E-B9D8-4A9B-88C7-2D2983802C51}")

	def __init__(self, far):
		super().__init__(far)

	def GetPluginInfoW(self):
		info = self.far.PluginInfo()

		PluginFlags = self.far.PluginFlags
		info.Flags = PluginFlags.Editor | PluginFlags.Viewer | PluginFlags.Dialog | PluginFlags.FullCmdLine

		info.PluginMenuItems = [
			(self.Msg(lng.PluginMenuItem), uuid.UUID("{DAF1257B-E011-4B5A-B5DC-732581BDF3BA}"))
		]

		info.DiskMenuItems = [
			(self.Msg(lng.PluginDiskMenuItem), uuid.UUID("{E1F617A7-A5EE-4672-8108-12500E4B9008}"))
		]

		info.PluginConfigItems = [
			(self.Msg(lng.PluginConfigItem), uuid.UUID("{BD5CD4D2-E12E-40B9-876E-C0047CF2CF38}"))
		]

		info.CommandPrefix = "hi";

		return info

	def OpenW(self, info):
		if info.OpenFrom == info.OpenFrom.FromMacro:
			self.far.GetUserScreen()
			try:
				for value in info.Data.Values:
					print(value.Type.name, ": ", value.Value)
			finally:
				self.far.SetUserScreen()
			return None

		BreakCode = [0]
		ItemId = self.far.Menu(
			self.Guid,
			uuid.UUID("{F86FD79D-21A7-4CCA-BD37-D2151AE5DA4E}"),
			-1,
			-1,
			0,
			self.far.MenuFlags.WrapMode,
			"Look, it's a menu! (opened from " + info.OpenFrom.name + ((": " + info.Data.CommandLine) if info.OpenFrom == info.OpenFrom.CommandLine else "") + ")",
			"A, B, C work as Enter",
			"MenuTopic",
			[
				self.far.FarKey(ord('A'), 0),
				self.far.FarKey(ord('B'), 0),
				self.far.FarKey(ord('C'), 0),
			],
			BreakCode,
			[
				self.far.MenuItem("Do Console Stuff", 0x263A),
				self.far.MenuItem("Do Input Box (press '7')", 0, self.far.FarKey(ord('7'), 0)),
				self.far.MenuItem("Do Message", self.far.MenuItemFlags.Checked),
				self.far.MenuItem("Do Help"),
			])

		if ItemId == 0:
			self.far.GetUserScreen()
			try:
				print("------------")
				print(self.Msg(lng.TheMessage))
				print("Plugin Menu id: " + str(info.Guid))
				print("BreakCode is " + str(BreakCode[0]))
				print("------------")
			finally:
				self.far.SetUserScreen()

		elif ItemId == 1:
			Response = self.far.InputBox(
				self.Guid,
				uuid.UUID("{0F87C22C-4F6F-4B71-90EC-C3C89B03010B}"),
				"Attention",
				"Your opinion is very important to us:",
				"InputHistory",
				"",
				1024,
				"InputBoxTopic",
				self.far.InputBoxFlags.Buttons)

			if Response is not None:
				Style = 0
				Message = "Isn't that awesome?"
			else:
				Response = "Nothing"
				Style = self.far.MessageFlags.Warning
				Message = "Shame on you."

			self.far.Message(
				self.Guid,
				uuid.UUID("{49A144B8-C0BB-4EDF-B8AD-8F3590BA9C64}"),
				Style,
				"Message1Topic",
				"Results",
				[
					"You have entered:",
					"\1",
					Response,
					"\1",
					Message
				],
				["Indeed"])

		elif ItemId == 2:
			fmf = self.far.MessageFlags
			self.far.Message(
				self.Guid,
				uuid.UUID("{49A144B8-C0BB-4EDF-B8AD-8F3590BA9C64}"),
				fmf.Warning | fmf.LeftAlign,
				"Message2Topic",
				"Behold!",
				[
					"Lorem ipsum dolor sit amet,",
					"consectetur adipiscing elit,",
					"sed do eiusmod tempor incididunt",
					"ut labore et dolore magna aliqua."
				],
				["Left", "Right"])

		elif ItemId == 3:
			self.far.ShowHelp(self.Guid, "Contents", self.far.HelpFlags.Guid)

		else:
			pass

		return None

	def ConfigureW(self, info):
		self.far.GetUserScreen()
		try:
			print("------------")
			print("Very configure. Wow.")
			print("Config Menu id: " + str(info.Guid))
			print("------------")
		finally:
			self.far.SetUserScreen()
		return True


FarPluginClass = HelloWorldPlugin
