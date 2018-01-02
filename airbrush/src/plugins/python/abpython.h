/*
    abpython.h
    Copyright (C) 2018 zg

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
#define HC_KEYWORD3 2
#define HC_STRING1  3
#define HC_NUMBER1  4
#define HC_HIGHLITE 5
#define HC_HL_ERROR 6

#define PARSER_CLEAR                  0
#define PARSER_COMMENT                1
#define PARSER_STRING_LONG_SINGLE     2
#define PARSER_STRING_LONG_DOUBLE     3
#define PARSER_STRING_LONG_SINGLE_RAW 4
#define PARSER_STRING_LONG_DOUBLE_RAW 5

#define PAIR_STRING_SHORT_SINGLE      1
#define PAIR_STRING_SHORT_DOUBLE      2
#define PAIR_STRING_SHORT_SINGLE_RAW  3
#define PAIR_STRING_SHORT_DOUBLE_RAW  4
#define PAIR_STRING_LONG_SINGLE       5
#define PAIR_STRING_LONG_DOUBLE       6
#define PAIR_STRING_LONG_SINGLE_RAW   7
#define PAIR_STRING_LONG_DOUBLE_RAW   8
#define PAIR_BRACKETS                 9
#define PAIR_SQ_BRACKETS             10
#define PAIR_BRACES                  11

extern ABColor colors[];
extern struct ColorizeInfo Info;
