/*
    abphp.h
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

#define HC_COMMON   0
#define HC_COMMENT  2
#define HC_KEYWORD1 4
#define HC_KEYWORD2 6
#define HC_STRING   8
#define HC_NUMBER   10
#define HC_VAR      12

#define PARSER_CLEAR   0x10
#define PARSER_COMMENT 0x20
#define PARSER_STRING1 0x30
#define PARSER_STRING2 0x40

#define PARSER_PHP     0xf0

extern int colors[];
extern struct ColorizeInfo Info;
