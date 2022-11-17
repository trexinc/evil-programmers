/*
    abyacclex.h
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

#define STRCMP strncmp

#define HC_COMMENT  0
#define HC_STRING   1
#define HC_KEYWORD  2
#define HC_SET      3

#define INVALID_C_STATE -1
#define PARSER_CLEAR     0
#define PARSER_RULES     1
#define PARSER_COMMENT   2
#define PARSER_STRING    3
#define PARSER_SET       4

extern FarColor colors[];
