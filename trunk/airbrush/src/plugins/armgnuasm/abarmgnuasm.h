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
#define HC_STRING1          1
#define HC_NUMBER1          2
#define HC_NUMBER2          3
#define HC_DEFINE           4
#define HC_FIXME            5
#define HC_HIGHLITE         6
#define HC_HL_ERROR         7
#define HC_KEYWORD1         8
#define HC_ARM_ARITHMETIC   9
#define HC_ARM_ARITHMETIC5  10
#define HC_THUMB_ARITHMETIC 11
#define HC_ARM_LOGICAL      12
#define HC_THUMB_LOGICAL    13
#define HC_ARM_MOVE         14
#define HC_THUMB_MOVE       15
#define HC_ARM_SHIFT        16
#define HC_THUMB_SHIFT      17
#define HC_ARM_BRANCH       18
#define HC_ARM_BRANCH5      19
#define HC_THUMB_BRANCH     20
#define HC_THUMB_BRANCH5    21
#define HC_ARM_MEMORY       22
#define HC_ARM_MEMORY5      23
#define HC_THUMB_MEMORY     24
#define HC_ARM_PSEUDO       25
#define HC_THUMB_PSEUDO     26
#define HC_ARM_REGS         27
#define HC_THUMB_REGS_LO    28
#define HC_THUMB_REGS_HI    29

#define PARSER_CLEAR    0
#define PARSER_COMMENT1 1
#define PARSER_STRING1  2
#define PARSER_THUMB    3
#define PARSER_COMMENT3 4
#define PARSER_STRING2  5

extern ABColor colors[];
extern struct ColorizeInfo Info;
