/*
    HXSView - list and extract files from HXS/HXI MS-Help 2 archives
    Copyright (C) 2004-2005 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>
//#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <limits.h>
//#include <errno.h>
#include <dir.h>

#include "litlib.h"

U32 start_of_lit;
static int error=0;

#if 0
void lit_error(int what, char * fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr,fmt, ap);
  printf("\n");
}
#endif

U8 *readat(void * v, U32 offset, int size)
{
  U8 *mem;
  int read_size;
  FILE *f;

  offset += start_of_lit;
  f = (FILE *)v;
  if (!f)
  {
    lit_error(0,"No filehandle passed in!");
    return NULL;
  }

  mem = malloc(size);
  if (!mem)
  {
    lit_error(ERR_LIBC,"malloc(%d) failed.", size);
    return NULL;
  }
  memset(mem, 0, size);
  if (fseek(f, offset, SEEK_SET) != 0)
  {
    free(mem);
    lit_error(ERR_LIBC|ERR_R,"fseek() failed.");
    return NULL;
  }

  read_size = fread(mem, 1, size, f);
  if (read_size != size)
  {
    if (feof(f))
    {
      lit_error(ERR_LIBC|ERR_R,"WARNING: Read went past the end of file by %d bytes. \n",size - read_size);
      return mem;
    }
    free(mem);
    lit_error(ERR_LIBC|ERR_R,"Unable to read %d chars at position %ld.",size, offset);
    return NULL;
  }
  return mem;
}

static char *pointtoname(char *path)
{
  char *p = path + strlen(path);
  while (p > path && *(p-1) != '/')
    --p;
  return p;
}

static int makedir(char *path)
{
  char *end;
  char *p;

  p = path+3;
  while ((end = strstr(p,"/")) != NULL)
  {
    long h;
    struct _finddata_t f;
    *end = '\0';
    h = _findfirst(path,&f);
    if (h!=-1)
    {
      _findclose(h);
      *end = '/';
      p = end + 1;
      continue;
    }

    if (_mkdir(path)!=0)
    {
      printf(" Error: Couldn't make directory %s\n", path);
      *end = '/';
      return 0;
    }
    *end = '/';
    p = end + 1;
  }
  return 1;
}

const char ILLEGAL_SYMB[] = "<>:|?*\"";
const char ILLEGAL_REPL[] = "()_!__'";

void ReplaceIllegalChars(char *ptr)
{
  char *org = ptr;
  while ( *ptr )
  {
    const char *zz;
    if ( (zz = strchr( ILLEGAL_SYMB, *ptr ) ) != NULL)
      *ptr = ILLEGAL_REPL[zz-ILLEGAL_SYMB];

    if ( *ptr < 32 || *ptr == 127 ) *ptr = 32;

    ptr++;
  }
}

static int savetofile(lit_file *l, char *target, int extractwithoutpath)
{
  int length;
  U8 *outbuf;
  char rtarget[PATH_MAX];
  char *p;
  FILE *f;
  char fullpath[PATH_MAX+1];

  if (target[0]!='/' && target[0]!=':')
    strcpy(rtarget,"/");
  else
    *rtarget=0;
  if (target[0]=='/' && target[1]==':' && target[2]==':')
    strcat(rtarget,target+1);
  else
    strcat(rtarget,target);
  if (lit_get_file(l, rtarget, &outbuf, &length) != 0)
    return 1;
  printf("Extracting %s",target);
  GetCurrentDirectory(sizeof(fullpath)-1,fullpath);
  while ((p = strstr(fullpath,"\\")) != NULL)
    *p = '/';
  if (fullpath[strlen(fullpath)-1] != '/')
    strcat(fullpath,"/");
  ReplaceIllegalChars(rtarget);
  strcat(fullpath,extractwithoutpath?pointtoname(rtarget):(rtarget[0]=='/'?rtarget+1:rtarget));
  if (!extractwithoutpath)
    if (!makedir(fullpath))
    {
      if (outbuf) free(outbuf);
      goto nosave;
    }
  if (fullpath[strlen(fullpath)-1]!='/')
  {
    f = fopen(fullpath, "wb");
    if (!f)
    {
      printf(" Error: Couldn't open %s\n", fullpath);
      goto nosave;
    }
    fwrite(outbuf, 1, length, f);
    fclose(f);
  }
  if (outbuf) free(outbuf);
  printf(" %s\n","OK");
  return 0;
nosave:
  error=1;
  printf(" %s\n","Error");
  return -1;
}

static int extractdir(lit_file *l, char *path, int extractwithoutpath)
{
  char filter[PATH_MAX];
  int i;
  int filterlen;
  entry_type *entry;

  if (path[0]!='/' && path[0]!=':')
    strcpy(filter,"/");
  else
    *filter=0;
  if (path[0]=='/' && path[1]==':' && path[2]==':')
    strcat(filter,path+1);
  else
    strcat(filter,path);
  //if (filter[strlen(filter)-1]!='/')
    //strcat(filter,"/");
  filterlen=strlen(filter);
  entry = l->entry;
  while (entry)
  {
    if (!strncmp(filter,&entry->name,filterlen))
      savetofile(l,&entry->name,extractwithoutpath);
      //if (savetofile(l,&entry->name,extractwithoutpath)!=0)
        //return -1;
    entry = entry->next;
  }
  return 0;
}

static void usage(void)
{
  printf("HXSView v1.0 beta 3 (March 7 2005)\n");
  printf("(c) Alex Yaroslavsky at yandex / trexinc.\n");
  printf("Using OpenCLit library by Dan Jackson at convertlit.com / drs\n");
  printf("and LZX library by Stuart Caie at 4u.net / kyzer.\n\n");
  printf("Usage: hxsview <option> hxsfile [/file_to_extract or @file_with_list]\n");
  printf("         Options:\n");
  printf("           l - list archive contents\n");
  printf("           e - extract without path names\n");
  printf("           x - extract\n");
  printf("         Notes:\n");
  printf("           When extracting specific files, file name MUST start with `/'.\n");
  printf("           No file masks should be used.\n");
}

int main(int argc, char *argv[])
{
  int status;
  lit_file lit;
  char *infname;
  char *command;
  FILE *fh;
  int i;
  entry_type *entry;

  if (argc < 3)
  {
    usage();
    exit(-1);
  }

  infname = argv[2];

  fh = fopen(infname, "rb");
  if (!fh)
  {
    printf("Error opening %s file.\n",infname);
    exit(-1);
  }
  else
  {
    char buf[64*1024];
    U32 size;
    U32 j;
    BOOL found=FALSE;

    fseek(fh, 0, SEEK_SET);
    size = fread(buf,1,sizeof(buf),fh);
    j=0;
    while (j<(size-8))
    {
      if (buf[j]=='I' && buf[j+1]=='T' && buf[j+2]=='O' && buf[j+3]=='L' && buf[j+4]=='I'
                      && buf[j+5]=='T' && buf[j+6]=='L' && buf[j+7]=='S')
      {
        found = TRUE;
        break;
      }
      j++;
    }
    if (found)
    {
      start_of_lit = j;
    }
    else
    {
      printf("%s is not an HSX file.\n",infname);
      exit(-1);
    }
  }

  memset(&lit, 0, sizeof(lit));
  lit.file_pointer = (void *)fh;
  lit.readat       = readat;
  (void)fseek(fh, 0, SEEK_END);
  lit.filesize = ftell(fh)-start_of_lit;

  status = lit_read_from_file(&lit);
  if (status)
  {
    lit_close(&lit);
    exit(-1);
  }

  command = argv[1];

  switch (command[0])
  {
    case 'l':
    {
      printf("--------\n");
      entry = lit.entry;
      while (entry)
      {
        if ((&entry->name)[0] == '/')
        {
          printf("%12.12u ",entry->size);
          printf("%s\n",&entry->name);
        }
        else
        {
          printf("%12.12u ",entry->size);
          printf("/");
          printf("%s\n",&entry->name);
        }
        entry = entry->next;
      }
      printf("--------\n");
      break;
    }

    case 'e':
    case 'x':
    {
      int extractwithoutpath=command[0]=='e'?1:0;

      if (argc == 4)
      {
        char *p;
        int s;
        char name[PATH_MAX];
        FILE *f;
        strcpy(name,argv[3]);
        f=NULL;
        if (name[0]=='@')
        {
          f = fopen(&name[1],"rt");
          if (f)
          {
            if (fgets(name,PATH_MAX,f))
            {
              if (name[strlen(name)-1]=='\n')
                name[strlen(name)-1]=0;
              if (name[strlen(name)-1]=='\r')
                name[strlen(name)-1]=0;
            }
            else
             name[0]=0;
          }
          else
            error=1;
        }
        while (name[0]!=0)
        {
          while ((p = strstr(name,"\\")) != NULL)
            *p = '/';
          if (savetofile(&lit,name,extractwithoutpath)==1)
            extractdir(&lit,name,extractwithoutpath);
          if (f)
          {
            if (fgets(name,PATH_MAX,f))
            {
              if (name[strlen(name)-1]=='\n')
                name[strlen(name)-1]=0;
              if (name[strlen(name)-1]=='\r')
                name[strlen(name)-1]=0;
            }
            else
             name[0]=0;
          }
          else
            break;
        }
        if (f)
          fclose(f);
      }
      else
      {
        int i;

        entry = lit.entry;
        while (entry)
        {
          savetofile(&lit,&entry->name,extractwithoutpath);
          //if (savetofile(&lit,&entry->name,extractwithoutpath)==1)
            //extractdir(&lit,&entry->name,extractwithoutpath);
          entry = entry->next;
        }
      }
      break;
    }
  }

  lit_close(&lit);

  return (error?-1:0);
}
