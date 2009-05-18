/*
    abarmgnuasm.h
    Copyright (C) 2009 zg

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

#define HC_COMMENT          0
#define HC_STRING1          2
#define HC_NUMBER1          4
#define HC_NUMBER2          6
#define HC_DEFINE           8
#define HC_FIXME            10
#define HC_HIGHLITE         12
#define HC_HL_ERROR         14
#define HC_KEYWORD1         16
#define HC_ARM_ARITHMETIC   18
#define HC_ARM_ARITHMETIC5  20
#define HC_THUMB_ARITHMETIC 22
#define HC_ARM_LOGICAL      24
#define HC_THUMB_LOGICAL    26
#define HC_ARM_MOVE         28
#define HC_THUMB_MOVE       30
#define HC_ARM_SHIFT        32
#define HC_THUMB_SHIFT      34
#define HC_ARM_BRANCH       36
#define HC_ARM_BRANCH5      38
#define HC_THUMB_BRANCH     40
#define HC_THUMB_BRANCH5    42
#define HC_ARM_MEMORY       44
#define HC_ARM_MEMORY5      46
#define HC_THUMB_MEMORY     48
#define HC_ARM_PSEUDO       50
#define HC_THUMB_PSEUDO     52
#define HC_ARM_REGS         54
#define HC_THUMB_REGS_LO    56
#define HC_THUMB_REGS_HI    58

#define PARSER_CLEAR    0
#define PARSER_COMMENT1 1
#define PARSER_STRING1  2
#define PARSER_THUMB    3
#define PARSER_COMMENT3 4
#define PARSER_STRING2  5

extern int colors[];
extern struct ColorizeInfo Info;
