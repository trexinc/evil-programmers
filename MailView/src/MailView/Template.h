/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

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
// Template.h: interface for the CTemplate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEMPLATE_H__AFFB9FA0_FB34_49D7_845C_38D32CD9F992__INCLUDED_)
#define AFX_TEMPLATE_H__AFFB9FA0_FB34_49D7_845C_38D32CD9F992__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTemplate
{
protected:

  CTemplate()
  {
  }

public:

  virtual ~CTemplate()
  {
  }

  class IWriteSink
  {
  public:
    virtual void write( LPCSTR str, int len ) = 0;
  };

  void processFile( LPCSTR fileName, IWriteSink *f );

  void processLine( LPCSTR str, IWriteSink *f );

protected:

  virtual void processMacro( LPCSTR str, int len, int fill, IWriteSink & f ) = 0;

};

#endif // !defined(AFX_TEMPLATE_H__AFFB9FA0_FB34_49D7_845C_38D32CD9F992__INCLUDED_)
