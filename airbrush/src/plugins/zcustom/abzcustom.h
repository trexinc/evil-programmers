/*
    abzcustom.h
    Copyright (C) 2000-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

struct Keyword
{
  char *keyword;
  int line_start;
  int fg;
  int bg;
  int exclusive;
  int recursive;
  char *whole_chars_left;
  char *whole_chars_right;
};

struct Context
{
  char *left;
  char *right;
  int fg;
  int bg;
  int exclusive;
  int exclusive_left;
  int exclusive_right;
  int line_start_left;
  int line_start_right;
  char *whole_chars_left;
  char *whole_chars_right;
  struct Keyword *keywords;
  int keywords_count;
};

struct Rules
{
  char *name;
  char *mask;
  char *start;
  struct Context *contexts;
  int contexts_count;
};

#define DEFAULT_WHOLE "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_01234567890"
#define WHOLE_SIZE 256

//character classes
struct CharacterClass
{
  unsigned char Class[256];
  unsigned int min;
  unsigned int max;
};

struct CharacterClassParam
{
  unsigned char *start;
  unsigned char *end;
  struct CharacterClass *char_class;
};

int yyparse(void *param);
