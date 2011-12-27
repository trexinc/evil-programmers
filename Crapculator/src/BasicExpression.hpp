#ifndef __BASICEXPRESSION_HPP__
#define __BASICEXPRESSION_HPP__
/*
    Crapculator plugin for Far Manager
    Copyright (C) 2009 Alex Yaroslavsky

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

#define HIGHEST_PRECEDENCE (1000000)

template <typename T>
class BasicExpression
{
  protected:
    T lvars[L'z'-L'a'+1];

  protected: //pure virtual
    virtual bool GetUnaryActionPrefix(const wchar_t **p, int *a) = 0;
    virtual bool UnaryAction(T *r, int a) = 0;

    virtual bool GetAction(const wchar_t **p, int *a) = 0;
    virtual bool Action(T x, T y, T *r, int a) = 0;

    virtual int Precedence(int a) = 0;

    virtual bool GetNumber(const wchar_t **p, T *n) = 0;

  protected:
    bool IsEnd(int c) { return c == 0 || c == L')' || c == L'=' || c == L';'; }

    bool IsDigit(int c) { return (c >= L'0' && c <= L'9'); }

    void SkipSpace(const wchar_t **p) { while (**p == L' ' || **p == L'\t') (*p)++; }

    int SkipOpenBracket(const wchar_t **p)
    {
      int c=0;
      SkipSpace(p);
      while (**p == L'(')
      {
        (*p)++;
        c++;
        SkipSpace(p);
      }
      return c;
    }

    int SkipCloseBracket(const wchar_t **p)
    {
      int c=0;
      SkipSpace(p);
      while (**p == L')')
      {
        (*p)++;
        c++;
        SkipSpace(p);
      }
      return c;
    }

    bool Expression(const wchar_t **p, T *n, int pa, int *b)
    {
      if (!**p)
        return false;

      int o=SkipOpenBracket(p);
      *b+=o;

    start:

      if (**p == L'$')
      {
        (*p)++; if (**p < L'a' || **p > L'z') return false;

        int v=**p - L'a';

        (*p)++;

        SkipSpace(p);

        if (**p == L':')
        {
          (*p)++;
          SkipSpace(p);

          if (**p != L'=') return false;
          (*p)++;

          int bb=0;
          T nn=0;
          if (!Expression(p, &nn, 0 , &bb) || bb)
            return false;

          (*p)++;
          SkipSpace(p);

          lvars[v]=nn;
          goto start;
        }
        else
        {
          *n=lvars[v];
        }
      }
      else if (!GetNumber(p, n))
      {
        int u;
        if (!GetUnaryActionPrefix(p,&u))
          return false;

        int d=*b;
        if (!Expression(p, n, HIGHEST_PRECEDENCE , b))
          return false;

        if (!UnaryAction(n, u))
          return false;

        o-=d - *b;
        if (o<0)
          return true;
      }

      SkipSpace(p);

      if (*b)
      {
        int c=SkipCloseBracket(p);
        *b-=c;
        o-=c;
        if (o < 0)
          return true;
      }

      if (*b<0)
        return false;

      while (!IsEnd(**p))
      {
        int a;
        if (pa && o==0)
        {
          const wchar_t *s=*p;

          if (!GetAction(p, &a))
            return false;

          if (pa >= Precedence(a))
          {
            *p = s;
            return true;
          }
        }
        else
        {
          if (!GetAction(p, &a))
            return false;
        }

        T x;
        int d=*b;
        if (!Expression(p, &x, Precedence(a), b))
          return false;

        if (!Action(*n, x, n, a))
          return false;

        o-=d - *b;
        if (o<0)
          return true;
      }

      *b-=SkipCloseBracket(p);

      return true;
    }

  public:
    bool Calc(const wchar_t **p, T *n)
    {
      int b=0;
      *n=0;

      memset(lvars,0,sizeof(lvars));

      return Expression(p,n,0,&b) && !b;
    }

    virtual ~BasicExpression() { }
};

#endif // __BASICEXPRESSION_HPP__
