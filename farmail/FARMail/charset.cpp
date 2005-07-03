/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "farmail.hpp"

const char MAINCHARSET1          [] = "MainCharset1";
const char MAINCHARSET2          [] = "MainCharset2";
const char MAINTABLE1            [] = "MainTable1";
const char MAINTABLE2            [] = "MainTable2";
const char CHARSET1              [] = "Charset1";
const char CHARSET2              [] = "Charset2";
const char CHARSET3              [] = "Charset3";
const char TABLE1                [] = "Table1";
const char TABLE2                [] = "Table2";
const char TABLE3                [] = "Table3";
const char USASCII               [] = "us-ascii";

void ReadCharsetTable(CHARSET_TABLE *CharsetTable)
{
  HKEY hRoot = HKEY_CURRENT_USER;

  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, MAINCHARSET1, CharsetTable[2].charset , GetMsg(MesDefKOICharset) , 20 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, MAINTABLE1, CharsetTable[2].table , GetMsg(MesDefKOITable) , 128 );

  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, MAINCHARSET2, CharsetTable[3].charset , GetMsg(MesDefWINCharset) , 20 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, MAINTABLE2, CharsetTable[3].table , GetMsg(MesDefWINTable) , 128 );

  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, CHARSET1,   CharsetTable[4].charset , NULLSTR , 20 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, CHARSET2,   CharsetTable[5].charset , NULLSTR , 20 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, CHARSET3,   CharsetTable[6].charset , NULLSTR , 20 );

  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, TABLE1,   CharsetTable[4].table , NULLSTR , 128 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, TABLE2,   CharsetTable[5].table , NULLSTR , 128 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, TABLE3,   CharsetTable[6].table , NULLSTR , 128 );
}


int ConstructCharset( CHARSET_TABLE **CharsetTable )
{
 *CharsetTable = (CHARSET_TABLE*)z_calloc( 7, sizeof( CHARSET_TABLE ) );
 if ( *CharsetTable ) {

    lstrcpy( (*CharsetTable)[1].charset , USASCII );

    //for (int i=2; i<7; i++)
      //(*CharsetTable)[i].num = UNKNOWN_CHARSET_TABLE;

    return 0;
 }
 return 1;
}



int InitCharset( CHARSET_TABLE **CharsetTable )
{

 if ( *CharsetTable ) {

    int j;
    CharTableSet ts;

    for ( j=2 ; j<7; j++ )  {

        int i = 0;

        (*CharsetTable)[j].num = UNKNOWN_CHARSET_TABLE;
        if (!*((*CharsetTable)[j].charset)) continue;

        while ( _Info.CharTable( i, (char*)&ts, sizeof( struct CharTableSet ) ) != -1 ) {
           if ( !FSF.LStrnicmp( ts.TableName, (*CharsetTable)[j].table, lstrlen((*CharsetTable)[j].table) ) &&
                *((*CharsetTable)[j].table)  ) {
              (*CharsetTable)[j].num = i;
              memcpy( (*CharsetTable)[j].DecodeTable, ts.DecodeTable, 256 );
              memcpy( (*CharsetTable)[j].EncodeTable, ts.EncodeTable, 256 );
           }
           i++;
        }

    }

 }
 return 0;
}


int DestructCharset( CHARSET_TABLE **CharsetTable )
{
 if ( *CharsetTable ) z_free(*CharsetTable);
 *CharsetTable = NULL;
 return 0;
}


int FindCharset( const char *charset , CHARSET_TABLE **CharsetTable )
{
 int i;

 for ( i=0 ; i<7; i++ )
 {
   if ( !FSF.LStricmp( (*CharsetTable)[i].charset, charset ) && (*CharsetTable)[i].num != UNKNOWN_CHARSET_TABLE) return i;
 }
 return -1;
}
