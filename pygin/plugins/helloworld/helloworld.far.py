import uuid
import enum

import pygin
from pygin import far

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
	Version = far.VersionInfo(1, 0, 0, 1, far.VersionStage.Alpha)

	def GetPluginInfoW(self):
		info = far.PluginInfo()

		info.Flags = far.PluginFlags.Editor | far.PluginFlags.Viewer | far.PluginFlags.Dialog | far.PluginFlags.FullCmdLine

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
			with(pygin.Console()):
				for value in info.Data.Values:
					print(value.Type.name, ": ", value.Value, sep="")
			return None

		BreakCode = [0]
		ItemId = far.Menu(
			self.Guid,
			uuid.UUID("{F86FD79D-21A7-4CCA-BD37-D2151AE5DA4E}"),
			-1,
			-1,
			0,
			far.MenuFlags.WrapMode,
			"Look, it's a menu! (opened from " + info.OpenFrom.name + ((": " + info.Data.CommandLine) if info.OpenFrom == info.OpenFrom.CommandLine else "") + ")",
			"A, B, C work as Enter",
			"MenuTopic",
			[
				far.FarKey(ord('A'), 0),
				far.FarKey(ord('B'), 0),
				far.FarKey(ord('C'), 0),
			],
			BreakCode,
			[
				far.MenuItem("Do Console Stuff", 0x263A),
				far.MenuItem("Do Input Box (press '7')", 0, far.FarKey(ord('7'), 0)),
				far.MenuItem("Do Message", far.MenuItemFlags.Checked),
				far.MenuItem("Do Help"),
			])

		if ItemId == 0:
			self.DoConsoleStuff(info, BreakCode)

		elif ItemId == 1:
			far.AdvControl(self.Guid, far.AdvancedControlCommands.RedrawAll)
			Response = far.InputBox(
				self.Guid,
				uuid.UUID("{0F87C22C-4F6F-4B71-90EC-C3C89B03010B}"),
				"Attention",
				"Your opinion is very important to us:",
				"InputHistory",
				"",
				1024,
				"InputBoxTopic",
				far.InputBoxFlags.Buttons)

			if Response is not None:
				Style = 0
				Message = "Isn't that awesome?"
			else:
				Response = "Nothing"
				Style = far.MessageFlags.Warning
				Message = "Shame on you."

			far.Message(
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
			fmf = far.MessageFlags
			far.Message(
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
			far.ShowHelp(self.Guid, "Contents", far.HelpFlags.Guid)

		else:
			pass

		# for panel:
		# return id(Instance)

		# for nothing
		return None

	def ConfigureW(self, info):
		with(pygin.Console()):
			print("------------")
			print("Very configure. Wow.")
			print("Config Menu id: " + str(info.Guid))
			print("------------")
		return True

	def DoConsoleStuff(self, info, BreakCode):
		WindowType = far.AdvControl(self.Guid, far.AdvancedControlCommands.GetWindowType)

		with(pygin.Console()):
			print("------------")
			print(self.Msg(lng.TheMessage))
			print("Plugin Menu id: " + str(info.Guid))
			print("BreakCode is " + str(BreakCode[0]))

			Version = far.AdvControl(self.Guid, far.AdvancedControlCommands.GetFarManagerVersion)
			if Version is not None:
				print("Far version: {0}".format(Version))

			if WindowType is not None:
				print("Current window: {0}".format(WindowType.Type.name))

			print("------------")

FarPluginClass = HelloWorldPlugin
