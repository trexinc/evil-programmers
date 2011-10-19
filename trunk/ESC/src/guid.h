/*
  [ESC] Editor's settings changer plugin for FAR Manager
  Copyright (C) 2011 Alex Yaroslavsky

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

  This unit used internally by xmlite.
  Special hashtable implementation.
  This table can hold several values with one key.
*/

#ifndef __GUID_H__
#define __GUID_H__

// {A74EC909-41F1-409b-9C4C-CA389BB93F45}
DEFINE_GUID(MainGuid, 0xa74ec909, 0x41f1, 0x409b, 0x9c, 0x4c, 0xca, 0x38, 0x9b, 0xb9, 0x3f, 0x45);
// {B479E10C-401E-4992-A8CC-B0DED7F8BB78}
DEFINE_GUID(ConfigureGuid, 0xb479e10c, 0x401e, 0x4992, 0xa8, 0xcc, 0xb0, 0xde, 0xd7, 0xf8, 0xbb, 0x78);
// {5BDC9271-CB12-4a6f-9514-84D4334D4A80}
DEFINE_GUID(IndicatorGuid, 0x5bdc9271, 0xcb12, 0x4a6f, 0x95, 0x14, 0x84, 0xd4, 0x33, 0x4d, 0x4a, 0x80);
// {12C86C85-7F13-4e79-AF52-B5C7FAD0A9CC}
DEFINE_GUID(MenuGuid, 0x12c86c85, 0x7f13, 0x4e79, 0xaf, 0x52, 0xb5, 0xc7, 0xfa, 0xd0, 0xa9, 0xcc);
// {2A120D81-C9BB-4cb2-B1A1-FB778F53D3F4}
DEFINE_GUID(AllInOneMessageGuid, 0x2a120d81, 0xc9bb, 0x4cb2, 0xb1, 0xa1, 0xfb, 0x77, 0x8f, 0x53, 0xd3, 0xf4);
// {CB3DE57A-F2D0-4109-A841-1317F430D593}
DEFINE_GUID(FileErrorMessageGuid, 0xcb3de57a, 0xf2d0, 0x4109, 0xa8, 0x41, 0x13, 0x17, 0xf4, 0x30, 0xd5, 0x93);
// {E3C14B47-BC4E-410b-80FC-69AEFC90E0A4}
DEFINE_GUID(CfgErrorMessageGuid, 0xe3c14b47, 0xbc4e, 0x410b, 0x80, 0xfc, 0x69, 0xae, 0xfc, 0x90, 0xe0, 0xa4);

#endif
