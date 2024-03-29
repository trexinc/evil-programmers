/*
    ablua.h
    Copyright (C) 2012 zg

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

#define HC_COMMENT  0
#define HC_KEYWORD1 1
#define HC_LABEL    2
#define HC_STRING1  3
#define HC_NUMBER1  4
#define HC_NUMBER2  5
#define HC_FIXME    6
#define HC_HIGHLITE 7
#define HC_HL_ERROR 8

#define PARSER_CLEAR   0
#define PARSER_COMMENT 1
#define PARSER_STRING1 2
#define PARSER_STRING2 3
#define PARSER_STRING3 4

struct LuaState
{
  int State;
  int Level;
};

extern FarColor colors[];
extern struct ColorizeInfo Info;
