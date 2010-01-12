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

//static const char *CodeHistory2 = "UserEncodeTo";
//extern long WINAPI SendDialogFunc(HANDLE hDlg,int Msg,int Param1,long Param2);


int Random(int x)
{
  return((int)_lrand()%x);
}

int FARMail::DoAllInOne( void )
{
  const char *ask[4];

  ask[0] = NULLSTR;
  ask[1] = GetMsg(MesAllInOne_Question);
  ask[2] = GetMsg(MesAllInOne_YesBtn);
  ask[3] = GetMsg(MesAllInOne_NoBtn);

  return _Info.Message( _Info.ModuleNumber, 0, NULL, ask, 4, 2 );
}


void InitBoundary( char *bound )
{
  srand(GetTickCount());
  FSF.sprintf( bound, "--=_NextPart_%04lX_%04lX_%04lX.%04lX", Random( 0xFFFF ), Random( 0xFFFF ), Random( 0xFFFF ), Random( 0xFFFF ) );
}

int FARMail::AnalizeFileMulti( MAILSEND *parm )
{
  parm->nbit = 8;
  parm->how  = 3;
  parm->bitcontrol = 1;
  parm->all  = 1;

  lstrcpy( parm->charset , CharsetTable[0].charset );
  lstrcpy( parm->encode  , Opt.DefOutCharset );

  InitBoundary( parm->boundary );

  return 0;
}

int FARMail::BeginSendFileMulti( MAILSEND * parm )
{
  parm->multipart = MULTI_START;
  return SendFile( parm, NULL , 0 );
}


int FARMail::SendFileMulti( MAILSEND * parm, const char *FileName , long tsize )
{
  parm->multipart = MULTI_SENDFILE;
  return SendFile( parm, FileName , tsize );
}

int FARMail::EndSendFileMulti( MAILSEND * parm )
{
  int stat;
  parm->multipart = MULTI_END;
  stat = SendFile( parm, NULL , 0 );
  parm->multipart = 0;
  return stat;
}
