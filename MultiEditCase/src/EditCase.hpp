/*
    Multilingual Editor Case Converter plugin for FAR Manager
    Copyright (C) 2001-2005 Alex Yaroslavsky

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
#ifndef __EDITCASE_H__
#define __EDITCASE_H__

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

BOOL DefaultLoaded;

// Menu item numbers
#define CCLower     0
#define CCUpper     1
#define CCTitle     2
#define CCToggle    3
#define CCCyclic    4

// Plugin Registry root
char PluginRootKey[80];
char PluginRootDir[260];

// This chars aren't letters
char WordDiv[80];
int WordDivLen;

unsigned char to_lower[256];
unsigned char to_upper[256];

void LangSelect(void);
BOOL MyIsAlpha(unsigned char c);
BOOL FindBounds(char *Str, int Len, int Pos, int *Start, int *End);
int FindEnd(char *Str, int Len, int Pos);
int FindStart(char *Str, int Len, int Pos);
int GetNextCCType(char *Str, int StrLen, int Start, int End);
int ChangeCase(char *NewString, int Start, int End, int CCType);
unsigned char LLower(unsigned char ch);
unsigned char LUpper(unsigned char ch);
int LIsLower(unsigned char ch);
unsigned char LLower(unsigned char ch);
void LLowerBuf(unsigned char *Buf, int Length);
void LUpperBuf(unsigned char *Buf, int Length);
void SetDefaultTables(void);
BOOL SetTables(const char *FileName);

#endif
