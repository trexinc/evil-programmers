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

#include "chmlib.h"
#include "fixendian.h"
#include "lzx.h"
#ifdef __WINDOWS__
  #include "crt.hpp"
  #include "memory.hpp"
  #ifdef __FARMANAGER__
    #include "../../plugin.hpp"
    extern FARSTANDARDFUNCTIONS FSF;
    #define SPrintf FSF.sprintf
  #else
    #define SPrintf wsprintf
  #endif
  #define StrCmp lstrcmp
  #define StrCpy lstrcpy
#else
  #include <stdlib.h>
  #include <string.h>
  #define SPrintf sprintf
  #define StrCmp strcmp
  #define StrCpy strcpy
#endif

#define FILELEN_HSECT 0
#define DIR_HSECT 1
#define CONTENT_FORMAT "::DataSpace/Storage/%s/Content"
#define CONTROLDATA_FORMAT "::DataSpace/Storage/%s/ControlData"
#define SPANINFO_FORMAT "::DataSpace/Storage/%s/SpanInfo"
#define LIST_FORMAT "::DataSpace/Storage/%s/Transform/List"
#define INSTANCEDATA_FORMAT "::DataSpace/Storage/%s/Transform/%s/InstanceData/"
#define RT_FORMAT "::DataSpace/Storage/%s/Transform/%s/InstanceData/ResetTable"
#define TRANSFORM_FORMAT "::Transform/%s/"
#define NAMELIST "::DataSpace/NameList"

#ifdef DEBUG

static void get_guid(ubyte *buf, guid_t *guid)
{
  memcpy(guid, buf, sizeof(guid_t));
  FIXENDIAN32(guid->guid1);
  FIXENDIAN16(guid->guid2[0]);
  FIXENDIAN16(guid->guid2[1]);
}

static void make_guid_string(guid_t *guid, char *s)
{
  SPrintf(s, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
          guid->guid1, guid->guid2[0], guid->guid2[1],
          guid->guid3[0], guid->guid3[1], guid->guid3[2], guid->guid3[3],
          guid->guid3[4], guid->guid3[5], guid->guid3[6], guid->guid3[7]);
}

#endif

static void guid_fix_endian(guid_t *guid)
{
  FIXENDIAN32(guid->guid1);
  FIXENDIAN16(guid->guid2[0]);
  FIXENDIAN16(guid->guid2[1]);
}

static int readheader(chmfile *c)
{
  int ret;
  #ifdef __WINDOWS__
  DWORD t;
  #endif

  #ifdef __WINDOWS__
  SetFilePointer(c->cf,0,NULL,FILE_BEGIN);
  ReadFile(c->cf,&c->ch,sizeof(chmheader),&t,NULL);
  ret=t;
  #else
  fseek(c->cf, 0, SEEK_SET);
  ret = fread(&c->ch, 1, sizeof(chmheader), c->cf);
  #endif
  guid_fix_endian(&c->ch.unk_guid1);
  guid_fix_endian(&c->ch.unk_guid2);
  FIXENDIAN32(c->ch.tot_hdrlen);
  #ifdef __WINDOWS__
  SetFilePointer(c->cf,0x58,NULL,FILE_BEGIN);
  ReadFile(c->cf,&c->content_offset,sizeof(c->content_offset),&t,NULL);
  #else
  fseek(c->cf, 0x58, SEEK_SET);
  fread(&c->content_offset, 1, sizeof(c->content_offset), c->cf);
  #endif
  FIXENDIAN32(c->content_offset);

#ifdef DEBUG
  {
    char str[40];
    printf( "itsf: %-4.4s\n", c->ch.itsf);
    printf( "tot_hdrlen: %08x\n", c->ch.tot_hdrlen);
    printf( "content_offset: %08x\n", c->content_offset);
    make_guid_string(&c->ch.unk_guid1, str);
    printf( "guid1: %s\n", str);
    make_guid_string(&c->ch.unk_guid2, str);
    printf( "guid2: %s\n", str);
  }
#endif

  #ifdef __WINDOWS__
  SetFilePointer(c->cf,sizeof(chmheader),NULL,FILE_BEGIN);
  #else
  fseek(c->cf, sizeof(chmheader), SEEK_SET);
  #endif
  return ret;
}

static int readhsectable(chmfile *c)
{
  int i;
  int result;
  int nhsecs = 2;
  #ifdef __WINDOWS__
  DWORD t;
  #endif

  #ifdef __WINDOWS__
  ReadFile(c->cf,c->hs,nhsecs*sizeof(hsecentry),&t,NULL);
  result = t/nhsecs;
  #else
  result = fread(c->hs, nhsecs, sizeof(hsecentry), c->cf);
  #endif
  for (i = 0; i < nhsecs; i++) {
    FIXENDIAN32(c->hs[i].offset);
    FIXENDIAN32(c->hs[i].length);
  }
  return result;
}

static int readdirheader(chmfile *c)
{
  int result;
  #ifdef __WINDOWS__
  DWORD t;
  #endif

  #ifdef __WINDOWS__
  SetFilePointer(c->cf,c->hs[DIR_HSECT].offset,NULL,FILE_BEGIN);
  ReadFile(c->cf,&c->dh,sizeof(dirheader),&t,NULL);
  result=t;
  #else
  fseek(c->cf, c->hs[DIR_HSECT].offset, SEEK_SET);
  result = fread(&c->dh, 1, sizeof(dirheader), c->cf);
  #endif
  FIXENDIAN32(c->dh.chunksize);
  FIXENDIAN32(c->dh.indexchunk);
  FIXENDIAN32(c->dh.ndirchunks);
  FIXENDIAN32(c->dh.firstpmglchunk);
  FIXENDIAN32(c->dh.lastpmglchunk);
  FIXENDIAN32(c->dh.length);
  guid_fix_endian(&c->dh.unk_guid1);
#ifdef DEBUG
  {
    char str[40];
    printf( "directory header length: %x\n", c->dh.length);
    printf( "chunksize: %x\n", c->dh.chunksize);
    printf( "ndirchunks: %x\n", c->dh.ndirchunks);
    printf( "indexchunk: %x\n", c->dh.indexchunk);
    printf( "firstpmglchunk: %x\n", c->dh.firstpmglchunk);
    printf( "lastpmglchunk: %x\n", c->dh.lastpmglchunk);
    make_guid_string(&c->dh.unk_guid1, str);
    printf( "guid1: %s\n", str);
  }
#endif
  return result;
}

static ulong getencint(ubyte **p)
{
  ulong accum = 0;
  while ((**p) & 0x80) {
    accum = (accum << 7) | ((*(*p)++)&0x7F);
  }
  accum = (accum << 7) | (*(*p)++);
  return accum;
}

static int read_chm_dir(chmfile *c)
{
  int length;
  ubyte *buf;
  ubyte *bufend;
  ubyte *p, *oldp;
  int namelen;
  int i;
  unsigned long section, offset, dlength;
  int nentries;
  int nchunks;
  pmglchunkheader pmglch;
  #ifdef __WINDOWS__
  DWORD t;
  DWORD chunkstart;
  #else
  fpos_t chunkstart;
  #endif

  readdirheader(c);
  #ifdef __WINDOWS__
  chunkstart = SetFilePointer(c->cf,0,NULL,FILE_CURRENT);
  #else
  chunkstart = ftell(c->cf);
  #endif
  length = c->dh.chunksize * c->dh.ndirchunks;
  p = buf = (ubyte*) malloc(length);
  bufend = buf;
  nchunks = 0;
  pmglch.next_chunk = 0;
  do {
    #ifdef __WINDOWS__
    SetFilePointer(c->cf,chunkstart + pmglch.next_chunk * c->dh.chunksize,NULL,FILE_BEGIN);
    ReadFile(c->cf,&pmglch,sizeof(pmglch),&t,NULL);
    #else
    fseek(c->cf, chunkstart + pmglch.next_chunk * c->dh.chunksize, SEEK_SET);
    fread(&pmglch, 1, sizeof(pmglch), c->cf);
    #endif
    FIXENDIAN32(pmglch.next_chunk);
    FIXENDIAN32(pmglch.prev_chunk);
    FIXENDIAN32(pmglch.quickreflen);
#ifdef DEBUG
    printf( "prev_chunk: %d\n", pmglch.prev_chunk);
    printf( "next_chunk: %d\n", pmglch.next_chunk);
    printf( "quickreflen: %x\n", pmglch.quickreflen);
#endif
    #ifdef __WINDOWS__
    ReadFile(c->cf,bufend,c->dh.chunksize - sizeof(pmglch) - pmglch.quickreflen,&t,NULL);
    length = t;
    #else
    length = fread(bufend, 1, c->dh.chunksize - sizeof(pmglch) - pmglch.quickreflen, c->cf);
    #endif
    bufend += length;
    nchunks++;
  }
  while ((pmglch.next_chunk != -1) && (nchunks < c->dh.ndirchunks));

  nentries = 0;

  while (p < bufend) {
    nentries++;
    namelen = *p++;
    p += namelen;
    section = getencint(&p);
    offset = getencint(&p);
    dlength = getencint(&p);
  }

#ifdef DEBUG
  printf( "nentries (calculated): %x\n", nentries);
#endif

  c->dir = (chm_dir *)malloc(sizeof(chm_dir) + (nentries-1) * sizeof(direntry));
  p = buf;
  for (i = 0; i < nentries; i++) {
    oldp = p;
    memset(&c->dir->entry[i], 0, sizeof(direntry));
    namelen = *p++;
    memcpy(c->dir->entry[i].name, p, namelen);
    c->dir->entry[i].name[namelen] = 0;
    p+=namelen;
    c->dir->entry[i].section = getencint(&p);
    offset = getencint(&p);
    dlength = getencint(&p);
    c->dir->entry[i].offset = offset;
    c->dir->entry[i].length = dlength;
  }
#ifdef DEBUG
  for (i = 0; i < nentries; i++)
    printf( "%08lx %08lx %08lx %s\n", c->dir->entry[i].section,
           c->dir->entry[i].offset, c->dir->entry[i].length,
           c->dir->entry[i].name);
#endif
  c->dir->nentries = nentries;
  free(buf);
  return 0;
}

static direntry *getdirentry(char *name, chm_dir *dir)
{
  int i;

  for (i = 0; i < dir->nentries; i++)
        if (!StrCmp(name, dir->entry[i].name))
          return &dir->entry[i];
  return NULL;
}

int chm_getfile(chmfile *c, char *name, ulong *length, ubyte **outbuf)
{
  int section;
  int offset = c->content_offset;
  direntry *de;
  #ifdef __WINDOWS__
  DWORD t;
  #endif

  *length = 0;
  *outbuf = NULL;
  de = getdirentry(name, c->dir);
#ifdef DEBUG
  printf( "Getting %s, de = %08x %s\n", name, de, de?de->name:"");
#endif
  if (!de)
      return -1;
  section = de->section;
  if (!c->cs || !c->cs->entry[section].iscompressed) {
      if (c->cs)
          offset += c->cs->entry[section].offset;

      #ifdef __WINDOWS__
      SetFilePointer(c->cf,de->offset + offset,NULL,FILE_BEGIN);
      #else
      fseek(c->cf, de->offset + offset, SEEK_SET);
      #endif
      *length = de->length;
      *outbuf = (ubyte*) malloc(*length);
      #ifdef __WINDOWS__
      ReadFile(c->cf,*outbuf,*length,&t,NULL);
      #else
      fread(*outbuf, 1, *length, c->cf);
      #endif
  }
  else if (c->cs->entry[section].cache) {
      *length = de->length;
      *outbuf = (ubyte*) malloc(*length);
      memcpy(*outbuf,
             c->cs->entry[section].cache + de->offset, *length);
  }
  else
  {
      char fname[4096];
      char guid_str[80];
//      ubyte *lbp;
      ulong flength;
      ubyte *rtfile;
      ubyte *cdfile;
      ubyte *cbp;
      ubyte *contbuf;
      ubyte *secbuf;
//      ulong rtindex;
      ulong uclength, clength;
      ulong contlength;
      ulong rtlength;
      ulong window_size;
//      guid_t guid;
      int result;

      SPrintf(fname, CONTROLDATA_FORMAT, c->cs->entry[section].name);

      /* get controldata */
      chm_getfile(c, fname, &flength, &cdfile);
      if (!cdfile)
        return -1;
      cbp = cdfile;
      if (memcmp(cbp+4, "LZXC", 4)) {
        //fprintf(stderr, "Compression method not LZXC: %-4.4s\n", cbp+4);
        free(cdfile);
        return -1;
      }
      window_size = *(ulong *)(cbp+0x10);
      FIXENDIAN32(window_size);
      free(cdfile);
      switch(window_size) {
      case 1: window_size = 15; break;
      case 2: window_size = 16; break;
      case 4: window_size = 17; break;
      case 8: window_size = 18; break;
      case 0x10: window_size = 19; break;
      case 0x20: window_size = 20; break;
      case 0x40: window_size = 21; break;
      default:
        //fprintf(stderr, "Window size invalid: %x\n", window_size);
        return -1;
      }
      StrCpy(guid_str, "{7FC28940-9D31-11D0-9B27-00A0C91E9C7C}");
      /* hardcoded string because transform list is broken */
      SPrintf(fname, RT_FORMAT, c->cs->entry[section].name, guid_str);
#ifdef DEBUG
      printf( "%s\n", fname);
#endif
      chm_getfile(c, fname, &rtlength, &rtfile);
      if (rtfile) {
        uclength = (rtfile[0x10]) | (rtfile[0x11]<<8) |
          (rtfile[0x12]<<16) | (rtfile[0x13] << 24);

        clength = (rtfile[0x18]) | (rtfile[0x19]<<8) |
          (rtfile[0x1a]<<16) | (rtfile[0x1b] << 24);
#ifdef DEBUG
      printf( "uclength = %x, clength = %x\n", uclength, clength);
#endif
        SPrintf(fname, CONTENT_FORMAT, c->cs->entry[section].name);
        chm_getfile(c,fname, &contlength, &contbuf);
        //if (clength != contlength)
          //fprintf(stderr, "Warning: Content Length not same as Compressed Length (without padding) %d %d \n", contlength, clength);

        secbuf = (ubyte*) malloc(uclength);
        /* uncompress it */
        LZXinit(window_size);
        result = LZXdecompress(contbuf, secbuf, clength, uclength);
#ifdef DEBUG
      printf( "LZXResult: %d\n", result);
#endif
        free (contbuf);
        free(rtfile);
        if (result != 0) {
          free(secbuf);
          return -1;
        }
        /* and get the file we want out of it */
#ifdef DEBUG
      printf( "offset = %x\n", de->offset);
      printf( "length = %x\n", de->length);
#endif
        *length = de->length;
        *outbuf = (ubyte*) malloc(*length);
        memcpy(*outbuf, secbuf + de->offset, *length);
        //                      free(secbuf);
        c->cs->entry[section].cache = secbuf;
        c->cs->entry[section].cachesize = uclength;
      }
  }
  return 0;
}

static int readcontsecs(chmfile *c)
{
  ulong length;
  ubyte *buf;
  ubyte *bufptr;
  int nentries;
  int nmlen;
  int i,j;
  char secname[4096];

  chm_getfile(c, NAMELIST, &length, &buf);
  nentries = buf[2] | (buf[3]<<8);
  c->cs = (contsecs *)malloc(sizeof (contsecs) +
                             (nentries-1)*sizeof(contsecentry));
  c->cs->nentries = nentries;
  bufptr = buf + 4;
  for (i = 0; i < nentries; i++) {
        nmlen = bufptr[0] | (bufptr[1]<<8);
        bufptr += 2;
        for (j = 0; j <= nmlen; j++) {  /* this is the lazy way to do wide characters.  Since the string is pretty much always "MSCompressed" or "Uncompressed", it'll do */
          c->cs->entry[i].name[j] = *bufptr;
          bufptr += 2;
        }
#ifdef DEBUG
      printf( "name = %s\n", c->cs->entry[i].name);
#endif
        if (!StrCmp(c->cs->entry[i].name, "MSCompressed")) {
          c->cs->entry[i].iscompressed = 1;
          /* this is the wrong way to figure out if a file is compressed.
             The real way is to examine the transform.  But the transform
             list is corrupt in most (all?) CHM files, apparently because
             the length was calculated in characters whereas the guid
             is recorded as wide characters
          */
        }
        else {
          c->cs->entry[i].iscompressed = 0;
        }
        c->cs->entry[i].cache = NULL;
        SPrintf(secname, CONTENT_FORMAT, c->cs->entry[i].name);
        c->cs->entry[i].offset = 0;
        if (i == 0) continue;
        for (j = 0; j < c->dir->nentries; j++) {
          if (!StrCmp(secname, c->dir->entry[j].name)) {
                c->cs->entry[i].offset = c->dir->entry[j].offset;
          }
        }
  }
  free(buf);
  return 0;
}

chmfile *chm_openfile(const char *fname)
{
    chmfile *result;
    #ifdef __WINDOWS__
    HANDLE f;
    #else
    FILE *f;
    #endif

    #ifdef __WINDOWS__
    f = CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (f==INVALID_HANDLE_VALUE)
        return NULL;
    #else
    f = fopen(fname, "rb");
    if (!f)
        return NULL;
    #endif
    result = (chmfile*) malloc(sizeof(chmfile));
    if (result)
    {
      result->cf = f;
      readheader(result);
      readhsectable(result);
      read_chm_dir(result);
      readcontsecs(result);
    }
    return result;
}

void chm_close(chmfile *c)
{
  int i;

  #ifdef __WINDOWS__
  CloseHandle(c->cf);
  #else
  fclose(c->cf);
  #endif
  for (i = 0; i < c->cs->nentries; i++) {
    if (c->cs->entry[i].cache)
      free(c->cs->entry[i].cache);
  }
  free(c->cs);
  free(c->dir);
}
