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

int FARMail::SendFromEditor( void )
{
  int err = 0;

  #ifdef TDEBUG
  try
  {
  #endif

    /*static*/ MAILSEND parm;
    int choice;
    struct FarMenuItem items[3];
    //{ { NULLSTR, 1, 0, 0 }, { NULLSTR, 0, 0, 1 } , { NULLSTR, 0, 0, 0 } };
    memset(items,0,sizeof(items));
    items[0].Selected = TRUE;
    items[1].Separator = TRUE;
    int Msgs[]={MesEditorMenu_Send};
    for(size_t i=0;i<sizeofa(Msgs);i++)
      FSF.sprintf(items[i].Text,"%c. %s",i+'1',GetMsg(Msgs[i]));
    FSF.sprintf(items[sizeofa(items)-1].Text,"%c. %s",'0',GetMsg(MesPluginEditor));

    choice=_Info.Menu(_Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,NULLSTR,NULL,NULL,NULL,NULL,items,sizeofa(items));

    if ( choice == -1 ) return 0;
    if(choice==2)
    {
      pm->ShowEditorMenu();
      return 0;
    }

    if ( !SelectMailbox() )
    {
       InitMAILSEND(&parm);
       if (*current->UserName)
         FSF.sprintf(parm.Sender,"%s <%s>",current->UserName,current->EMail);
       else
         FSF.sprintf(parm.Sender,"<%s>",current->EMail);
       parm.UseCC=Opt.UseCC;

       err = AnalizeFile( NULL , &parm );
       if ( !err )
       {
          err=MailSendDialog(&parm,NULL,false,false,false); //FIXME
          if ( !err )
          {
             SendFileWA( &parm, NULL , 0 );
             SaveOutgoingMessage( SAVE_KILL, NULL );
          }
       }
       FreeMailSend( &parm );
    }
    current = NULL;

    return err;
#ifdef TDEBUG
  }
#ifdef __BORLANDC__
  __except(1)
#else
  catch (...)
#endif
  {
    SayException( "SendFromEditor" );
    return -1;
  }
#endif
}
