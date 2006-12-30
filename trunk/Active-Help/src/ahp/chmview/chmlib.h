/* CHMtools v0.1 */
/* Copyright 2001 Matthew T. Russotto */
/*
    This file is part of CHMtools

    CHMtools is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    CHMtools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef __WINDOWS__
  #include "windows.h"
#else
  #include <stdlib.h>
  #include <stdio.h>
#endif
typedef unsigned long ulong;
typedef unsigned short ushort; //Already defined for Darwin
typedef unsigned char ubyte;

typedef struct guid_t
{
  ulong guid1;
  ushort guid2[2];
  ubyte guid3[8];
} guid_t;

typedef struct chmheader {
  char itsf[4];
  ulong version; /* CHMTools only deals with version 3 */
  ulong tot_hdrlen; /*  0x60 in version 3 */
  ulong unk3; /* always 0x01 */
  ulong last_modified; /* format unknown */
  ulong langid; /* usually 0x0409 (ENGLISH_US); I've seen 0x0407 (GERMAN_GERMAN) */
  guid_t unk_guid1; /* 7C01FD10-7BAA-11D0-9E0C-00A0-C922-E6EC */
  guid_t unk_guid2; /* 7C01FD11-7BAA-11D0-9E0C-00A0-C922-E6EC */
} chmheader;

typedef struct hsecentry {
  ulong offset;
  ulong offset_hi; /* note that CHMLib does not support >4GB files */
  ulong length;
  ulong length_hi;
} hsecentry;

typedef hsecentry hsectable[2];

typedef struct hsec1_t {
  ulong unk1; /* 0x1FE */
  ulong unk2; /* 0 */
  ulong file_length;
  ulong file_length_hi;
} hsec1_t;

typedef struct dirheader {
  char itsp[4];
  ulong unk1;        /* 0x01 */
  ulong length;      /* 0x54 */
  ulong unk3;        /* 0x0a */
  ulong chunksize;   /* 0x1000 */
  ulong unk5;        /* 0x02 */
  ulong tree_depth;        /* 1 if there is no index chunk, 2 if there is one.  CHMTools does not handle higher numbers */
  ulong indexchunk;  /* -1 if none, but 0 (probably erroneously) in at least one file */
  ulong firstpmglchunk;
  ulong lastpmglchunk;
  ulong unk8;        /* 0xFFFFFFFF */
  ulong ndirchunks;
  ulong langid;        /* 0x0409 (again?) */
  guid_t unk_guid1;   /* 5D02926A-212E-11D0-9DF9-00A0C922E6EC */
  ulong unk10;       /* 0x54 */
  ulong unk11[3];    /* 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF */
} dirheader;

typedef struct pmglchunkheader {
  char pmgl[4];
  ulong quickreflen; /* length of the quickref area at the end of section */
  ulong unk1; /* 0   probably intended to be this_chunk, but not necessary
                 because this_chunk is defined by position */
  ulong prev_chunk;
  ulong next_chunk;
} pmglchunkheader;

typedef struct direntry
{
  ulong section;
  ulong offset;
  ulong length;
  char name[256];
} direntry;

typedef struct chm_dir {
  int nentries;
  direntry entry[1];
} chm_dir;

typedef struct contsecentry {
  char name[256];
  int offset;
  short iscompressed;
  ubyte *cache;
  ulong cachesize;
} contsecentry;

typedef struct contsecs {
  int nentries;
  struct contsecentry entry[1];
} contsecs;

typedef struct chmfile
{
  #ifdef __WINDOWS__
    HANDLE cf;
  #else
    FILE *cf;
  #endif
  chmheader ch;
  hsectable hs;
  contsecs *cs;
  dirheader dh;
  chm_dir *dir;
  ulong file_length;
  ulong content_offset;
} chmfile;


  //int chm_getfile(chmfile *c, char *name, ulong *length, ubyte **outbuf);

  //chmfile *chm_openfile(const char *fname);

  //void chm_close(chmfile *l);
