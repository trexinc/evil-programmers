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
#include "StdAfx.h"
#include <FarPlus.h>
#include "Person.h"
#include "Mime.h"
#include <ctype.h>

CPerson::CPerson() : m_Next( NULL )
{
  memset( &FTNAddr, 0, sizeof( FTNAddr ) );
}

CPerson::~CPerson()
{
  if ( m_Next )
    delete m_Next;
}

FarString CPerson::GetMailboxName() const
{
  if ( Addr.IsEmpty() && FTNAddr.Zone == 0 )
    return Name;

  FarString result;
  if ( !Addr.IsEmpty() )
    result += Addr;
  else
  {
    far_assert( FTNAddr.Zone != BAD_FTNADDR_FIELD );
    far_assert( FTNAddr.Net != BAD_FTNADDR_FIELD );
    far_assert( FTNAddr.Node != BAD_FTNADDR_FIELD );
    far_assert( FTNAddr.Point != BAD_FTNADDR_FIELD );

    // Dennis Trachuk <Dennis.Trachuk@p51.f654.n5030.z2.fidonet.org>

    FarString tmp = Name;
    int pos;
    while ( (pos = tmp.IndexOf( '\x20' )) != -1 )
      tmp[ pos ] = '.';

    result += tmp;
    result += '@';
    if ( FTNAddr.Point )
      result += 'p' + IntToString( FTNAddr.Point ) + '.';
    if ( FTNAddr.Node )
      result += 'f' + IntToString( FTNAddr.Node );
    result += ".n" + IntToString( FTNAddr.Net );
    result += ".z" + IntToString( FTNAddr.Zone );
    result += ".fidonet.org";
  }

  if ( !Name.IsEmpty() )
    result = Name + " <" + result + '>';

  return result;
}

#define SKIPWS( ptr ) while ( isspace( (unsigned char)*ptr ) ) ptr ++

#define terminate_string(a, b, c) do { if ((b) < (c)) a[(b)] = 0; else \
a[(c)] = 0; } while (0)

#define terminate_buffer(a, b) terminate_string(a, b, sizeof (a) - 1)

const char RFC822Specials[] = "@.,:;<>[]\\\"()";
#define is_special(x) strchr(RFC822Specials,x)

LPCSTR parse_comment( LPCSTR s, LPSTR comment, size_t *commentlen, size_t commentmax )
{
  int level = 1;

  while (*s && level)
  {
    if (*s == '(')
      level++;
    else if (*s == ')')
    {
      if (--level == 0)
      {
        s++;
        break;
      }
    }
    else if (*s == '\\')
    {
      if (!*++s)
        break;
    }
    if (*commentlen < commentmax)
      comment[(*commentlen)++] = *s;
    s++;
  }
  if (level)
  {
    //RFC822Error = ERR_MISMATCH_PAREN;
    return NULL;
  }
  return s;
}

LPCSTR parse_quote( LPCSTR s, LPSTR token, size_t *tokenlen, size_t tokenmax)
{
  if (*tokenlen < tokenmax)
    token[(*tokenlen)++] = '"';
  while (*s)
  {
    if (*tokenlen < tokenmax)
      token[*tokenlen] = *s;
    if (*s == '"')
    {
      (*tokenlen)++;
      return (s + 1);
    }
    if (*s == '\\')
    {
      if (!*++s)
        break;

      if (*tokenlen < tokenmax)
        token[*tokenlen] = *s;
    }
    (*tokenlen)++;
    s++;
  }
//  RFC822Error = ERR_MISMATCH_QUOTE;
  return NULL;
}

LPCSTR next_token( LPCSTR s, LPSTR token, size_t * tokenlen, size_t tokenmax )
{
  if (*s == '(')
    return ( parse_comment(s + 1, token, tokenlen, tokenmax ) );
  if (*s == '"')
    return ( parse_quote( s + 1, token, tokenlen, tokenmax ) );

  if ( is_special( *s ) )
  {
    if (*tokenlen < tokenmax)
      token[(*tokenlen)++] = *s;
    return (s + 1);
  }
  while (*s)
  {
    if ( isspace( (unsigned char)*s ) || is_special( *s ) )
      break;
    if ( *tokenlen < tokenmax )
      token[ (*tokenlen)++ ] = *s;
    s++;
  }
  return s;
}

LPCSTR parse_mailboxdomain( LPCSTR s, LPCSTR nonspecial,
         LPSTR mailbox, size_t * mailboxlen, size_t mailboxmax,
         LPSTR comment, size_t * commentlen, size_t commentmax )
{
  const char *ps;

  while (*s)
  {
    SKIPWS (s);
    if (strchr (nonspecial, *s) == NULL && is_special (*s))
      return s;

    if (*s == '(')
    {
      if (*commentlen && *commentlen < commentmax)
        comment[(*commentlen)++] = ' ';
      ps = next_token (s, comment, commentlen, commentmax);
    }
    else
      ps = next_token (s, mailbox, mailboxlen, mailboxmax);
    if (!ps)
      return NULL;
    s = ps;
  }

  return s;
}

LPCSTR parse_address( LPCSTR s,
               LPSTR token, size_t *tokenlen, size_t tokenmax,
         LPSTR comment, size_t *commentlen, size_t commentmax,
         PPerson addr )
{
  s = parse_mailboxdomain (s, ".\"(\\",
    token, tokenlen, tokenmax,
    comment, commentlen, commentmax);
  if (!s)
    return NULL;

  if (*s == '@')
  {
    if (*tokenlen < tokenmax)
      token[(*tokenlen)++] = '@';
    s = parse_mailboxdomain (s + 1, ".([]\\",
      token, tokenlen, tokenmax,
      comment, commentlen, commentmax);
    if (!s)
      return NULL;
  }

  terminate_string (token, *tokenlen, tokenmax);
  addr->Addr =token;

  if ( *commentlen && addr->Name.IsEmpty() )
  {
    terminate_string (comment, *commentlen, commentmax);
    addr->Name = comment;
  }

  return s;
}

LPCSTR parse_addr_spec( LPCSTR s, LPSTR comment, size_t * commentlen,
            size_t commentmax, PPerson addr )
{
  char token[ 128 ];
  size_t tokenlen = 0;

  s = parse_address( s, token, &tokenlen, sizeof( token ) - 1, comment, commentlen, commentmax, addr );
  if ( s && *s && *s != ',' && *s != ';' )
  {
//    RFC822Error = ERR_BAD_ADDR_SPEC;
    return NULL;
  }
  return s;
}

void add_addrspec( PPerson * Top, PPerson * Last, LPCSTR phrase,
        LPSTR comment, size_t * commentlen, size_t commentmax )
{
  PPerson Cur = create CPerson;

  if ( parse_addr_spec( phrase, comment, commentlen, commentmax, Cur ) == NULL )
  {
    delete Cur;
    Cur = NULL;
    return;
  }

  if ( *Last )
    (*Last)->m_Next = Cur;
  else
    *Top = Cur;
  *Last = Cur;
}

LPCSTR parse_route_addr( LPCSTR s,
             LPSTR comment, size_t *commentlen, size_t commentmax,
             PPerson addr )
{
  char token[128];
  size_t tokenlen = 0;

  SKIPWS (s);

  /* find the end of the route */
  if (*s == '@')
  {
    while (s && *s == '@')
    {
      if (tokenlen < sizeof (token) - 1)
        token[tokenlen++] = '@';
      s = parse_mailboxdomain (s + 1, ",.\\[](", token,
        &tokenlen, sizeof (token) - 1,
        comment, commentlen, commentmax);
    }
    if (!s || *s != ':')
    {
//      RFC822Error = ERR_BAD_ROUTE;
      return NULL; /* invalid route */
    }

    if (tokenlen < sizeof (token) - 1)
      token[tokenlen++] = ':';
    s++;
  }

  if ((s = parse_address (s, token, &tokenlen, sizeof (token) - 1, comment, commentlen, commentmax, addr)) == NULL)
    return NULL;

  if ( *s != '>' || addr->Addr.IsEmpty() )
  {
//    RFC822Error = ERR_BAD_ROUTE_ADDR;
    return NULL;
  }

  s ++;
  return s;
}

PPerson rfc822_parse_adrlist( PPerson Top, LPCSTR s )
{
  LPCSTR begin;
  LPCSTR ps;
  char comment[128], phrase[128];
  size_t phraselen = 0, commentlen = 0;

  PPerson Cur;
  PPerson Last = Top;

  while ( Last && Last->m_Next ) Last = Last->m_Next;

  SKIPWS( s );

  begin = s;
  while ( *s )
  {
    if ( *s == ',' )
    {
      if ( phraselen )
      {
        terminate_buffer( phrase, phraselen );
        add_addrspec( &Top, &Last, phrase, comment, &commentlen, sizeof( comment ) - 1 );
      }
      else if ( commentlen && Last && Last->Name.IsEmpty() )
      {
        terminate_buffer( comment, commentlen );
        Last->Name = comment;
      }

#ifdef EXACT_ADDRESS
      if ( Last && Last->val == NULL )
        Last->val = mutt_substrdup( begin, s );
#endif
      commentlen = 0;
      phraselen  = 0;
      s ++;
      begin = s;
      SKIPWS( begin );
    }
    else if ( *s == '(' )
    {
      if ( commentlen && commentlen < sizeof( comment ) - 1 )
        comment[ commentlen++ ] = ' ';
      if ( ( ps = next_token( s, comment, &commentlen, sizeof (comment) - 1)) == NULL )
      {
        delete Top;
        Top = NULL;
        return NULL;
      }
      s = ps;
    }
    else if ( *s == ':' )
    {
      terminate_buffer( phrase, phraselen );
      Cur = create CPerson;
      Cur->Addr = phrase;
      //Cur->group = 1;

      if ( Last )
        Last->m_Next = Cur;
      else
        Top = Cur;

      Last = Cur;

#ifdef EXACT_ADDRESS
      last->val = mutt_substrdup (begin, s);
#endif

      phraselen  = 0;
      commentlen = 0;
      s ++;
      begin = s;
      SKIPWS( begin );
    }
    else if ( *s == ';' )
    {
      if ( phraselen )
      {
        terminate_buffer (phrase, phraselen);
        add_addrspec( &Top, &Last, phrase, comment, &commentlen, sizeof( comment ) - 1 );
      }
      else if (commentlen && Last && Last->Name.IsEmpty() )
      {
        terminate_buffer (comment, commentlen);
        Last->Name = comment;
      }
#ifdef EXACT_ADDRESS
      if (last && !last->val)
        last->val = mutt_substrdup (begin, s);
#endif

      /* add group terminator */
      Cur = create CPerson;
      if ( Last )
      {
        Last->m_Next = Cur;
        Last = Cur;
      }

      phraselen  = 0;
      commentlen = 0;
      s ++;
      begin = s;
      SKIPWS( begin );
    }
    else if ( *s == '<' )
    {
      terminate_buffer( phrase, phraselen );
      Cur = create CPerson;
      if ( phraselen )
      {
        FarSF::Unquote( phrase );
        Cur->Name = phrase;
      }
      if ( ( ps = parse_route_addr( s + 1, comment, &commentlen, sizeof( comment ) - 1, Cur ) ) == NULL )
      {
        delete Top;
        delete Cur;
        Top = NULL;
        Cur = NULL;
        return NULL;
      }

      if ( Last )
        Last->m_Next = Cur;
      else
        Top = Cur;
      Last = Cur;

      phraselen  = 0;
      commentlen = 0;
      s = ps;
    }
    else
    {
      if ( phraselen && phraselen < sizeof( phrase ) - 1 && *s != '.' )
        phrase[phraselen++] = ' ';
      if ( ( ps = next_token( s, phrase, &phraselen, sizeof( phrase ) - 1 ) ) == NULL )
      {
        delete Top;
        Top = NULL;
        return NULL;
      }
      s = ps;
    }
    SKIPWS (s);
  }

  if (phraselen)
  {
    terminate_buffer (phrase, phraselen);
    terminate_buffer (comment, commentlen);
    add_addrspec( &Top, &Last, phrase, comment, &commentlen, sizeof( comment ) - 1 );
  }
  else if ( commentlen && Last && Last->Name.IsEmpty() )
  {
    terminate_buffer( comment, commentlen );
    Last->Name = comment;
  }
#ifdef EXACT_ADDRESS
  if ( Last )
    Last->val = mutt_substrdup( begin, s );
#endif

  return Top;
}

PPerson CPerson::Create( LPCSTR AddrList )
{
  return rfc822_parse_adrlist( NULL, AddrList );
}

FarString CPerson::getAddresses() const
{
  FarString addresses = GetMailboxName();
  for ( PPerson p = m_Next; p != NULL ; p = p->m_Next )
    addresses += ", " + m_Next->GetMailboxName();
  return addresses;
}
