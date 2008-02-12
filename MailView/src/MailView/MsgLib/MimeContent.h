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
#ifndef ___MimeContent_H___
#define ___MimeContent_H___

//#include "Mime.H"
#include <FarHash.h>
#include <ctype.h>

class CMimeContent
{
public:
  enum TType { ctUnknown = 0, ctText };
//  enum TEncoding { ceUnknown = 0, ce7bit, ce8bit, ceQuotedPrintable, ceBase64, ceXUue };
private:
  LPSTR         m_Type;
  LPSTR         m_SubType;
  FarStringHash m_Data;
//  TEncoding m_Encoding;
public:
  CMimeContent() : m_Type( NULL ), m_SubType( NULL ), m_Data( 4, false, true )
  {
  }
  CMimeContent( LPCSTR KludgeValue ) : m_Type( NULL ), m_Data( 4, false, true )
  {
    assign( KludgeValue );
  }
  ~CMimeContent()
  {
    delete [] m_Type;
  }

  void assign(LPCSTR KludgeValue)
  {
    far_assert( KludgeValue != NULL );

    m_Data.RemoveAll();

    if (m_Type) delete [] m_Type;

    //m_Type = m_SubType = NULL;

    far_assert( *KludgeValue == '\0' || FarSF::LIsAlpha( *KludgeValue ) );
    //while ( *KludgeValue != '\0' && isspace( (unsigned char)*KludgeValue ) )
    //  KludgeValue ++;


    m_Type = create char[strlen(KludgeValue) + 1];
    strcpy(m_Type, KludgeValue);

    if ((m_SubType = strchr(m_Type, '/')) == NULL)
    {
      extern LPCSTR STR_EmptyStr;
      m_SubType = (char*)STR_EmptyStr;
      return;
    }

    *m_SubType++ = '\0';

    LPSTR ptr = m_SubType;
    bool blWasSemicolon = false;

    while ( ptr && *ptr != '\0' )
    {
      LPSTR DataName;

      if (blWasSemicolon)
      {
        DataName = ptr;
      }
      else
      {
        DataName = strchr( ptr, ';' );
        if ( DataName == NULL ) return;

         *DataName = '\0';

         ptr = DataName;
         while( isspace( (unsigned char)*--ptr ) ) *ptr = '\0';
      }

      blWasSemicolon=false;

      do { DataName++;
      } while ( isspace( (unsigned char)*DataName ) && *DataName != 0 );

      if (*DataName == '\0')
        break;

      LPSTR DataValue = strchr( DataName, '=' );
      if ( DataValue == NULL )
        break;

      LPSTR StrEnd = DataValue;
      while( isspace( (unsigned char)*--StrEnd ) ) *StrEnd = '\0';

      *DataValue = '\0';

      do { DataValue++;
      } while ( isspace( (unsigned char)*DataValue ) && *DataValue != 0 );

      if (*DataValue == '\0')
        break;

      if ( *DataValue == '\"' )
      {
        ptr = strchr( ++DataValue, '\"' );

        if ( ptr != NULL )
          *ptr++ = '\0';
      }
      else
      {
        ptr = DataValue;

        while ( *ptr != '\0' && !isspace((unsigned char)*ptr) && *ptr != ';' ) ptr++;

        if (*ptr == ';') blWasSemicolon = true;
        if (*ptr != '\0')  *ptr++ = '\0';
      }

      if (*DataValue != '\0')
        m_Data[DataName] = (DWORD)DataValue;
    }
  }

  LPCSTR getType() const
  {
    return m_Type;
  }

  LPCSTR getSubType() const
  {
    return m_SubType;
  }

  LPCSTR getDataValue(LPCSTR dataName) const
  {
    return (LPCSTR)m_Data.Find(dataName);
  }
};

#endif //!defined(___MimeContent_H___)
