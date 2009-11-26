/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#ifndef __e_options_hpp
#define __e_options_hpp

#include "myrtl.hpp"
#include "TList.hpp"
#include "strcon.hpp"
#include "CUserMacros.hpp"
#include "CLockFile.hpp"

enum E_OPTIONS
{
   E_AutoIndent_On        = 0x00000001,
   E_AutoIndent_Off       = 0x00000002,

   E_KillSpaces_On        = 0x00000004,
   E_KillEmptyLines_On    = 0x00000008,

   E_ExpandTabs_On        = 0x00000010,
   E_ExpandTabs_Off       = 0x00000020,
   E_ExpandTabs_OnlyNew   = 0x00000040,

   E_CursorBeyondEOL_On   = 0x00000100,
   E_CursorBeyondEOL_Off  = 0x00000200,

   E_ProcessKeyEnd_On     = 0x00000400,

   E_AutoWrap_On          = 0x00000800,

   E_CharCodeBase_Oct     = 0x00001000,
   E_CharCodeBase_Dec     = 0x00002000,
   E_CharCodeBase_Hex     = 0x00004000,

   E_SmartHome_On         = 0x00008000,

   E_SkipPath_On          = 0x00010000,

   E_ProcessQuote_On      = 0x00020000,

   E_SaveFilePos_On       = 0x00040000,
   E_SaveFilePos_Off      = 0x00080000,

   E_AddSymbol_On         = 0x00100000,

   E_ForceKillEL_On       = 0x00200000,

   E_SmartTab_On          = 0x00400000,
   E_SmartBackSpace_On    = 0x00800000,

   E_QuoteEOL_On          = 0x01000000,

   E_Process_Minuses_On   = 0x02000000,

   E_LockMode_On          = 0x04000000,
   E_LockMode_Off         = 0x08000000,

   E_UserMacro_On         = 0x10000000,

   E_LockFile_On          = 0x20000000,

   E_OverLoad_On          = 0x80000000,
};

enum E2_OPTIONS
{
   E_WordSym_On           = 0x00000001,

   E_AlphaNum_On          = 0x00000020,

   E_Wrap_Justify         = 0x00000040,
   E_Wrap_Percent         = 0x00000080,

   E_Show_White_Space_On  = 0x00000100,
   E_Show_White_Space_Off = 0x00000200,
};

class NODEDATA
{
 public:
   NODEDATA();
   NODEDATA(const NODEDATA& rhs);
   ~NODEDATA() {}
   NODEDATA& operator=(const NODEDATA&rhs);
   bool operator==(const NODEDATA &rhs) const;
   bool operator<(const NODEDATA &rhs) const;
   void Inherit(NODEDATA &Dest);

 public:
   strcon Name;
   strcon mask;
   strcon AddSym_S;// если E_AddSymbol_On, то после этих символов добавлять
   strcon AddSym_E;// соответствующий символ из AddSymbol
   strcon AddSym_S_B, AddSym_E_B;
   strcon TableName;
   strcon AdditionalLetters;
   int TabSize;    // "-1" - не менять
   int WrapPos;    // Если больше 0 и меньше 512, то включить
   DWORD Table;    // 0 - ничего не менять
                   // 1 - установить для новых файлов кодировку OEM
                   // 2 - установить для новых файлов кодировку ANSI
                   // N - установить для новых файлов пользовательскую таблицу
                   //     номер (N-3)
   DWORD MinLinesNum;// Используется, если E_SaveFilePos_On
   DWORD EOL;
   DWORD Options;  // E_OPTIONS
   DWORD Options2; // E2_OPTIONS
   DWORD Lines;    // Сколько строк просматривать при SmartTab или 0,
                   // если SmartTab отключено
   int TableNum;
   CUserMacros KeyMacros, MouseMacros; // макросы для этого редактора
   TList<KeySequenceStorage> OnCreateMacros, OnLoadMacros;
   CLockFile LockFile;
   strcon StopChar;
};

#endif // __e_options_hpp
