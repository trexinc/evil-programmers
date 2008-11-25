/*
    abhtml.h
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

#define HC_ERROR      0
#define HC_COMMENT    2
#define HC_OPENTAG    4
#define HC_CLOSETAG   6
#define HC_REFERENCE  8
#define HC_ATTRNAME  10
#define HC_ATTRVALUE 12
#define HC_MARKUP    14
#define HC_SUBSET    16
#define HC_PI        18

#define PARSER_CLEAR    0x00
#define PARSER_COMMENT  0x01
#define PARSER_OPENTAG  0x02
#define PARSER_CLOSETAG 0x03
#define PARSER_MARKUP   0x04
#define PARSER_SUBSET   0x05
#define PARSER_PI       0x06
#define PARSER_VALUES   0x07
#define PARSER_STRING1  0x08
#define PARSER_STRING2  0x09
#define PARSER_HTML     0x0fu

#define PARSER_PHP      0x10

extern int colors[];
extern struct ColorizeInfo Info;
