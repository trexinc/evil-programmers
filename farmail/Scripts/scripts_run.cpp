/*
    Scripts sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group

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
#include "parser.hpp"
#include "scripts.hpp"
#include "language.hpp"

void Parser::Edit(void)
{
  char error_name[100], error_loc[100], mes[100];
  FSF.sprintf(mes,GetMsg(SErr_Location,error_loc),yy_line+1,yy_col+1);
  SayError(ERR_SYNTAX,GetMsg(error_index,error_name),mes);
  FInfo.Editor(filename,NULL,0,0,-1,-1,EF_DISABLEHISTORY,yy_line+1,yy_col+1);
}

int run_script(char *name)
{
  char fn[MAX_PATH];
  lstrcpy(fn,name);
  ExpandFilename(fn);
  Parser parser(fn);
  if(parser.Compile())
  {
    parser.Edit();
  }
  else
  {
    Randomize();
    parser.Execute();
  }
  return 0;
}
