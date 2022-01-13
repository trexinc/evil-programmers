"""
"Hello, world" plugin
"""
"""
helloworld.far.py
"""
"""
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

import uuid
import enum
import os
from pprint import pprint
import stat

import pygin
from pygin import far

@enum.unique
class lng(enum.IntEnum):
	PluginMenuItem     = 0
	PluginDiskMenuItem = 1
	PluginConfigItem   = 2
	TheMessage         = 3

class Panel:
	def __init__(self):
		self.__cwd = "/"

	def cwd(self):
		return self.__cwd

	def chdir(self, d):
		if d == "..":
			self.__cwd = os.path.dirname(self.__cwd)
		else:
			if self.__cwd[-1] != "/":
				self.__cwd += "/"
			self.__cwd += d

class HelloWorldPlugin(pygin.Plugin):
	Title = "Hello Python world"
	Author = "Alex Alabuzhev"
	Description = "Python plugin example"
	Guid = uuid.UUID("{31A0D11E-B9D8-4A9B-88C7-2D2983802C51}")
	Version = far.VersionInfo(1, 0, 0, 1, far.VersionStage.Alpha)

	def GetPluginInfoW(self):
		info = far.PluginInfo()

		info.Flags = far.PluginFlags.Editor | far.PluginFlags.Viewer | far.PluginFlags.Dialog

		info.PluginMenuItems = [
			(self.GetMsg(lng.PluginMenuItem), uuid.UUID("{DAF1257B-E011-4B5A-B5DC-732581BDF3BA}"))
		]

		info.DiskMenuItems = [
			(self.GetMsg(lng.PluginDiskMenuItem), uuid.UUID("{E1F617A7-A5EE-4672-8108-12500E4B9008}"))
		]

		info.PluginConfigItems = [
			(self.GetMsg(lng.PluginConfigItem), uuid.UUID("{BD5CD4D2-E12E-40B9-876E-C0047CF2CF38}"))
		]

		info.CommandPrefix = "hi";

		return info

	def OpenW(self, info):
		if info.OpenFrom == far.OpenFrom.FromMacro:
			with pygin.Console():
				for value in info.Data.Values:
					print(value.Type.name, ": ", value.Value, sep="")
			return None

		elif info.OpenFrom == far.OpenFrom.CommandLine:
			with pygin.Console():
				if not len(info.Data.CommandLine):
					print("Usage: hi:<Python code>")
				else:
					Result = eval(info.Data.CommandLine)
					if Result is not None:
						print("Result: {0}".format(Result))
			return None

		BreakCode = [0]
		ItemId = self.Menu(
			uuid.UUID("{F86FD79D-21A7-4CCA-BD37-D2151AE5DA4E}"),
			-1,
			-1,
			0,
			far.MenuFlags.WrapMode,
			"Look, it's a menu! (opened from " + info.OpenFrom.name + ")",
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
				far.MenuItem("Goto C:\\windows"),
				far.MenuItem("Open custom panel"),
				far.MenuItem("Open custom dialog"),
			])

		if ItemId == 0:
			self.DoConsoleStuff(info, BreakCode)

		elif ItemId == 1:
			self.AdvControl(far.AdvancedControlCommands.RedrawAll)
			Response = self.InputBox(
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

			self.Message(
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
			self.Message(
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
			self.ShowHelp("Contents", far.HelpFlags.Guid)

		elif ItemId == 4:
			Dir = far.PanelDirectory()
			Dir.Name = "C:\\windows"
			self.ActivePanel.PanelControl(far.FileControlCommands.SetPanelDirectory, 0, Dir)

		elif ItemId == 5:
			return Panel()

		elif ItemId == 6:
			self.DialogRun(
				uuid.UUID("{63FB716A-1568-4B6B-8FDC-0E5B5B73E090}"),
				-1, -1, 60, 22,
				"",
				[far.DialogDoubleBox(2, 1, 57, 20, "Test Dialog"),
				 far.DialogText(3, 2, 30, "test text"),
				 far.DialogVerticalText(56, 2, 19, "test vertical text"),
				 far.DialogCheckbox(3, 3, "unchecked", False),
				 far.DialogCheckbox(30, 3, "checked", True),
				 far.DialogSingleBox(3, 4, 30, 7, "Group 1"),
				 far.DialogRadiobutton(4, 5, "unchecked", False, flags=far.DialogItemFlags.GROUP),
				 far.DialogRadiobutton(4, 6, "checked", True),
				 far.DialogSingleBox(31, 4, 55, 7, "Group 2"),
				 far.DialogRadiobutton(32, 5, "checked", True, flags=far.DialogItemFlags.GROUP),
				 far.DialogRadiobutton(32, 6, "unchecked", False),
				 far.DialogComboBox(3, 8, 28, 
				 		[far.ListItem(x) for x in ["item 1", "item 2", "item 3"]],
						"test combo box"),
				 far.DialogComboBox(31, 8, 54, [
						far.ListItem("item 1"),
						far.ListItem("item 2 (pre-selected)", far.ListItemFlags.SELECTED),
						far.ListItem("item 3")]),
				 far.DialogEdit(3, 10, 29, "edit text"),
				 far.DialogFixEdit(31, 10, 54, "fixed edit text"),
				 far.DialogListBox(3, 11, 28, 14, [
						far.ListItem("item 1"),
						far.ListItem("item 2 (pre-selected)", far.ListItemFlags.SELECTED),
						far.ListItem("item 3")],
						"test list box"),
				 far.DialogPasswordEdit(31, 12, 54, "password"),
				 far.DialogButton(3, 15, "default button",
				 				  far.DialogItemFlags.DEFAULTBUTTON),
				 far.DialogButton(30, 15, "test button")
				],
				0
			)

		else:
			pass

		# for nothing
		return None

	def ConfigureW(self, info):
		with pygin.Console():
			print("------------")
			print("Very configure. Wow.")
			print("Config Menu id: " + str(info.Guid))
			print("------------")
		return True

	def GetOpenPanelInfoW(self, panel: Panel):
		info = far.OpenPanelInfo()
		cwd = panel.cwd()
		info.CurDir = cwd if cwd != "/" else "";
		info.PanelTitle = "Test Panel"
		return info

	def GetFindDataW(self, panel: Panel):
		info = far.GetFindDataInfo()
		cwd = panel.cwd()
		level = 1 if cwd == "/" else len(cwd.split("/"))
		info.PanelItems = [
			far.PluginPanelItem(FileName=".."),
			far.PluginPanelItem(
				FileName=f"file{level}",
				FileAttributes=stat.FILE_ATTRIBUTE_ARCHIVE)
		]
		if level <= 3:
			info.PanelItems.append(far.PluginPanelItem(
				FileName=f"dir{level}",
				FileAttributes=stat.FILE_ATTRIBUTE_DIRECTORY))
		return info

	def SetDirectoryW(self, info: far.SetDirectoryInfo):
		info.Panel.chdir(info.Dir)
		return True

	def GetFilesW(self, info: far.GetFilesInfo):
		cwd = info.Panel.cwd()
		if not cwd.endswith("/"):
			cwd += "/"
		for item in info.PanelItems:
			virt_path = cwd + item.FileName
			with open(os.path.join(info.DestPath, item.FileName), "w") as f:
 				f.write(f"'{virt_path}' content")
		return True

	def DoConsoleStuff(self, info, BreakCode):
		WindowType = self.AdvControl(far.AdvancedControlCommands.GetWindowType)

		with pygin.Console():
			print("------------")
			print(self.GetMsg(lng.TheMessage))
			print("Plugin Menu id: " + str(info.Guid))
			print("BreakCode is " + str(BreakCode[0]))

			Version = self.AdvControl(far.AdvancedControlCommands.GetFarManagerVersion)
			if Version is not None:
				print("Far version: {0}".format(Version))

			if WindowType is not None:
				print("Current window: {0}".format(WindowType.Type.name))

			PanelInfo = self.ActivePanel.PanelControl(far.FileControlCommands.GetPanelInfo)
			if PanelInfo is not None:
				print("Panel info:")
				pprint(vars(PanelInfo))
			PanelItem = self.ActivePanel.PanelControl(far.FileControlCommands.GetCurrentPanelItem)
			if PanelItem is not None:
				print("Current panel item:")
				pprint(vars(PanelItem))

			for Command in [
				far.FileControlCommands.GetColumnTypes,
				far.FileControlCommands.GetColumnWidths,
				far.FileControlCommands.GetPanelPrefix,
				far.FileControlCommands.GetPanelHostFile,
				far.FileControlCommands.GetPanelFormat,
			]:
				Value = self.ActivePanel.PanelControl(Command)
				if Value is not None:
					print("{0}: {1}".format(Command.name, Value))

			print("------------")

FarPluginClass = HelloWorldPlugin
