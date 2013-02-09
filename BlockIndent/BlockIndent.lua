--[[
    Block Indent for Far Manager
	Version: 2.0
    Copyright (C) 2001 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Features:
  - Right or Left Indentation of the selected block (or the  current  line)  by
    a tab or a space.
  - Each line in the block is indented relatively to its own position  with  no
    respect to other lines in the block.
  - Tab indentation as in Borland Turbo IDE, text is indented  to  the  nearest
    tab position and not just by tab size.
  - Works correctly with any editor settings (expand tabs or not, inserts  real
    tabs).
--]]

local Indent = function (IndentByTabSize, Forward)
	local F = far.Flags
	local ei = editor.GetInfo(nil);
	local IndentSize = IndentByTabSize and ei.TabSize or 1
	local IndentStr = IndentByTabSize and "\t" or " "

	local line = ei.CurLine;
	local loop = false
	if ei.BlockType ~= F.BTYPE_NONE then
		local s = editor.GetString()
		if s.SelStart ~= -1 then
			loop = true;
			line = ei.BlockStartLine;
		end
	end

	editor.UndoRedo(nil, F.EUR_BEGIN)

	repeat
		editor.SetPosition(nil, line, 0, -1, -1, 0, ei.Overtype)
		local s = editor.GetString(nil, -1, 0)
		if not s or (loop and ((s.SelStart == -1) or (s.SelStart == s.SelEnd))) then
			break
		end
		local j = s.StringText:find("[^ \t]")
		if j and (j>1 or Forward) then
			local TabPos = editor.RealToTab(nil, -1, j) - 1
			local x = math.floor(TabPos/IndentSize)
			if ((TabPos%IndentSize) == 0) and not Forward then
				x = x - 1
			end
			x = Forward and x + 1 or x
			editor.SetString(nil, -1, s.StringText:sub(j), s.StringEOL)
			for i=1,x,1 do editor.InsertText(nil,IndentStr) end
		end
		line = line + 1
	until not loop

	editor.SetPosition(nil, ei.CurLine, ei.CurPos, -1, ei.TopScreenLine, ei.LeftPos, ei.Overtype)
	editor.UndoRedo(nil, F.EUR_END)
end

Macro {
  area="Editor"; key="ShiftTab"; flags="DisableOutput"; description="Indent right by tab size"; action = function()
Indent(true, true)
  end;
}

Macro {
  area="Editor"; key="ShiftBS"; flags="DisableOutput"; description="Indent left by tab size"; action = function()
Indent(true, false)
  end;
}

NoMacro {
  area="Editor"; key=""; flags="DisableOutput"; description="Indent right by space"; action = function()
Indent(false, true)
  end;
}

NoMacro {
  area="Editor"; key=""; flags="DisableOutput"; description="Indent left by space"; action = function()
Indent(false, false)
  end;
}
