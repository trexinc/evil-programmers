/*
    MatPat.cpp
    Copyright (C) 2000-2001 Igor Lyubimov
    Copyright (C) 2002-2008 zg

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

#include "MatPat.h"

bool MatchPattern(UTCHAR* raw,UTCHAR* pat,UTCHAR asterisk)
{
  int i,slraw;
  if((*pat=='\0')&&(*raw=='\0'))                            /*  if it is end of both   */
    return true;                                            /*  strings,then match     */
  if(*pat=='\0')                                            /*  if it is end of only   */
    return false;                                           /*  pat then mismatch      */
  if(*pat==asterisk)                                        /* if pattern is a '*'     */
  {
    if(*(pat+1)=='\0')                                      /*    if it is end of pat  */
      return true;                                          /*    then match           */
    for(i=0,slraw=_tcslen((TCHAR*)raw);i<=slraw;i++)        /*    else hunt for match  */
      if((*(raw+i)==*(pat+1))||(*(pat+1)=='?'))             /*         or wild card    */
        if(MatchPattern(raw+i+1,pat+2,asterisk)==true)      /*      if found,match     */
          return true;                                      /*        rest of pat      */
  }
  else
  {
    if(*raw=='\0')                                          /*  if end of raw then     */
      return false;                                         /*     mismatch            */
    if((*pat=='?')||(*pat==*raw))                           /*  if chars match then    */
      if(MatchPattern(raw+1,pat+1,asterisk)==1)             /*  try & match rest of it */
        return true;
  }
  return false;                                             /*  no match found         */
}
