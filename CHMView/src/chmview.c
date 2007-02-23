/*
    CHMView - list and extract files from CHM/CHI HtmlHelp archives
    Copyright (C) 2002-2005 Alex Yaroslavsky

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dir.h>

#include "chm_lib.h"
#include "utf8.h"

struct cb_data
{
  int extractwithoutpath;
  char *path;
  int pathlen;
};

static int error=0;

char *pointtoname(char *path)
{
  char *p = path + strlen(path);
  while (p > path && *(p-1) != '/')
    --p;
  return p;
}

int makedir(char *path)
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

static int savetofile(struct chmFile *c, struct chmUnitInfo *ui, int extractwithoutpath)
{
  LONGINT64 length=0;
  char *outbuf=NULL;
  char *p;
  FILE *f;
  char fullpath[CHM_MAX_PATHLEN*2+1];
  char target[CHM_MAX_PATHLEN*2+1];
  wchar_t temp[CHM_MAX_PATHLEN+1];

  if (ui->length)
  {
    outbuf = (char *)malloc((unsigned int)ui->length);
    if (!outbuf)
      return -1;
    length = chm_retrieve_object(c, ui, outbuf, 0, ui->length);
    if (length==0)
    {
      if (outbuf) free(outbuf);
      return -1;
    }
  }
  GetCurrentDirectory(sizeof(fullpath)-1,fullpath);
  while ((p = strstr(fullpath,"\\")) != NULL)
    *p = '/';
  if (fullpath[strlen(fullpath)-1] != '/')
    strcat(fullpath,"/");
  decode_UTF8(temp,ui->path);
  WideCharToMultiByte(CP_ACP,0,temp,-1,target,sizeof(target),NULL,NULL);
  ReplaceIllegalChars(target);
  strcat(fullpath,extractwithoutpath?pointtoname(target):(target[0]=='/'?target+1:target));
  if (!extractwithoutpath)
    if (!makedir(fullpath))
    {
      if (outbuf) free(outbuf);
      return -1;
    }
  if (fullpath[strlen(fullpath)-1]!='/')
  {
    f = fopen(fullpath, "wb");
    if (!f)
    {
      printf(" Error: Couldn't open %s\n", fullpath);
      return -1;
    }
    if (length)
      fwrite(outbuf, 1, length, f);
    fclose(f);
  }
  if (outbuf) free(outbuf);
  return 0;
}

void extract(struct chmFile *c, struct chmUnitInfo *ui, int extractwithoutpath)
{
  char target[CHM_MAX_PATHLEN*2+1];
  wchar_t temp[CHM_MAX_PATHLEN+1];

  decode_UTF8(temp,ui->path);
  WideCharToMultiByte(CP_OEMCP,0,temp,-1,target,sizeof(target),NULL,NULL);
  printf("Extracting %s ",target);
  if (savetofile(c, ui, extractwithoutpath))
  {
    printf("Error\n");
    error=1;
    //return CHM_ENUMERATOR_FAILURE;
  }
  printf("OK\n");
}

int _extract_callback_all(struct chmFile *c, struct chmUnitInfo *ui, void *context)
{
  extract(c,ui,(int)context);
  return CHM_ENUMERATOR_CONTINUE;
}

int _extract_callback_dir(struct chmFile *c, struct chmUnitInfo *ui, void *context)
{
  struct cb_data *data = (struct cb_data *)context;
  if (!_strnicmp(ui->path,data->path,data->pathlen))
  {
    extract(c,ui,data->extractwithoutpath);
  }
  return CHM_ENUMERATOR_CONTINUE;
}

extractdir(struct chmFile *c, char *path, int extractwithoutpath)
{
  struct cb_data data = {extractwithoutpath,path,strlen(path)};
  chm_enumerate(c,CHM_ENUMERATE_ALL,_extract_callback_dir,(void *)&data);
  return 0;
}

int _print_ui(struct chmFile *c, struct chmUnitInfo *ui, void *context)
{
  char target[CHM_MAX_PATHLEN*2+1];
  wchar_t temp[CHM_MAX_PATHLEN+1];

  decode_UTF8(temp,ui->path);
  WideCharToMultiByte(CP_OEMCP,0,temp,-1,target,sizeof(target),NULL,NULL);

  if ((target)[0] == '/')
  {
    printf("%12.12u ",ui->length);
    printf("%s\n",target);
  }
  else
  {
    printf("%12.12u ",ui->length);
    printf("/");
    printf("%s\n",target);
  }
  return CHM_ENUMERATOR_CONTINUE;
}

static void usage(void)
{
  printf("CHMView v2.0 beta 3 with UTF-8 support (March 7 2005)\n");
  printf("(c) Alex Yaroslavsky at yandex / trexinc.\n");
  printf("Using:\nCHMLib library by Jed Wing at ugcs.caltech.edu / jedwin\n");
  printf("LZX library by Stuart Caie at 4u.net / kyzer\n");
  printf("UTF8 en/decoder by Oleg Bondar at mail.ru / hobo-mts\n\n");
  printf("Usage: chmview <option> chmfile [/file_to_extract or @file_with_list]\n");
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
  struct chmFile *c;
  char *infname;
  char *command;
  FILE *fh;
  int i;
  struct chmUnitInfo ui;

  if (argc < 3)
  {
    usage();
    exit(-1);
  }

  infname = argv[2];

  c = chm_open(infname);
  if (!c)
    exit(-1);

  command = argv[1];

  switch (command[0])
  {
    case 'l':
    {
      printf("--------\n");
      chm_enumerate(c,CHM_ENUMERATE_ALL,_print_ui,NULL);
      printf("--------\n");
      break;
    }

    case 'e':
    case 'x':
    {
      char target[CHM_MAX_PATHLEN*2+1];
      wchar_t temp[CHM_MAX_PATHLEN+1];
      int status;
      int extractwithoutpath = command[0]=='e'?1:0;

      if (argc == 4)
      {
        char *p;
        int s;
        char name[CHM_MAX_PATHLEN*2+1];
        FILE *f;
        strcpy(name,argv[3]);
        f=NULL;
        if (name[0]=='@')
        {
          f = fopen(&name[1],"rt");
          if (f)
          {
            if (fgets(name,sizeof(name),f))
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
          if (name[0]!='/' && name[0]!=':')
            strcpy(target,"/");
          else
            *target=0;
          if (name[0]=='/' && name[1]==':' && name[2]==':')
            strcat(target,name+1);
          else
            strcat(target,name);
          MultiByteToWideChar(CP_ACP,0,target,-1,temp,sizeof(temp));
          encode_UTF8(target,temp);
          status = chm_resolve_object(c,target,&ui);
          if (status==CHM_RESOLVE_SUCCESS && ui.path[strlen(ui.path)-1]!='/')
          {
            s=savetofile(c,&ui,extractwithoutpath);
            if (s==-1)
              error=1;
          }
          else
          {
            if (extractdir(c,target,extractwithoutpath))
              error=1;
          }
          if (f)
          {
            if (fgets(name,sizeof(name),f))
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
        chm_enumerate(c,CHM_ENUMERATE_ALL,_extract_callback_all,(void *)extractwithoutpath);
      }
      break;
    }
  }

  chm_close(c);

  return (error?-1:0);
}
