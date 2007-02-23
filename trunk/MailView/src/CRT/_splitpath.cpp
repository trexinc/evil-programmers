#include "_splitpath.hpp"
#include "strlen.hpp"
#include "strncpy.hpp"

#define __min(a,b) ((a)>(b)?(b):(a))

void _splitpath(register const char *path, char *drive, char *dir, char *fname, char *ext)
{
  register char *p;
  char *last_slash = NULL, *dot = NULL;
  unsigned len;

  if ((strlen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == ':'))
  {
      if (drive)
      {
          strncpy(drive, path, _MAX_DRIVE - 1);
          *(drive + _MAX_DRIVE-1) = '\0';
      }
      path += _MAX_DRIVE - 1;
  }
  else if (drive)
  {
    *drive = '\0';
  }

  for (last_slash = NULL, p = (char *)path; *p; p++)
  {
    if (*p == '/' || *p == '\\')
      last_slash = p + 1;
    else if (*p == '.')
      dot = p;
  }

  if (last_slash)
  {
    if (dir)
    {
      len = __min(((char *)last_slash - (char *)path) / sizeof(char),(_MAX_DIR - 1));
      strncpy(dir, path, len);
      *(dir + len) = '\0';
    }
    path = last_slash;
  }
  else if (dir)
  {
    *dir = '\0';
  }

  if (dot && (dot >= path))
  {
    if (fname)
    {
      len = __min(((char *)dot - (char *)path) / sizeof(char), (_MAX_FNAME - 1));
      strncpy(fname, path, len);
      *(fname + len) = '\0';
    }
    if (ext)
    {
      len = __min(((char *)p - (char *)dot) / sizeof(char), (_MAX_EXT - 1));
      strncpy(ext, dot, len);
      *(ext + len) = '\0';
    }
  }
  else
  {
    if (fname)
    {
        len = __min(((char *)p - (char *)path) / sizeof(char), (_MAX_FNAME - 1));
        strncpy(fname, path, len);
        *(fname + len) = '\0';
    }
    if (ext)
    {
        *ext = '\0';
    }
  }
}
