/*
    [ESC] Editor's settings changer plugin for FAR Manager
    Copyright (C) 2001 Ivan Sintyurin
    Copyright (C) 2008 Alex Yaroslavsky

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
#ifndef __myrtl_hpp
#define __myrtl_hpp

#ifdef __cplusplus
extern "C"
{
#endif
  int fseek(FILE *stream, long offset, int whence);
  long int ftell(FILE *stream);
  FILE *wfopen(const wchar_t *filename, const wchar_t *mode);
  int fclose(FILE *stream);
  size_t fread(void *ptr, size_t size, size_t n, FILE *stream);
  size_t fwrite(const void *ptr, size_t size, size_t n, FILE *stream);
#ifdef __cplusplus
}
#endif

#endif // __myrtl_hpp
