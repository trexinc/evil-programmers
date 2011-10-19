/*
    open_file_dialog.hpp
    Copyright (C) 2003-2005 Vadim Yegorov and Alex Yaroslavsky
    Copyright (C) 2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __OPEN_FILE_DIALOG_HPP__
#define __OPEN_FILE_DIALOG_HPP__

bool open_file_dialog(const TCHAR *curr_dir,TCHAR *filename);

#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyAltMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)
#define ControlKeyNonAltMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyCtrlMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED)
#define ControlKeyNonCtrlMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED)
#define IsShift(rec) static_cast<bool>(((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==SHIFT_PRESSED)
#define IsAlt(rec) static_cast<bool>(((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAltMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonAltMask))
#define IsCtrl(rec) static_cast<bool>(((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyCtrlMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonCtrlMask))
#define IsNone(rec) static_cast<bool>(((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)

#endif /* __OPEN_FILE_DIALOG_HPP__ */
