/*
    HtmlHelp sub-plugin for Active-Help plugin for FAR Manager
    Copyright (C) 2003-2005 Alex Yaroslavsky

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
*/
asm
(
  ".globl _MHWND, _PID, _PERROR\n"
  ".section .shared, \"ds\"\n"
  "_MHWND:\n"
  "\t.long 0\n"
  "_PID:\n"
  "\t.long 0\n"
  "_PERROR:\n"
  "\t.fill 128,1,0\n"
);
