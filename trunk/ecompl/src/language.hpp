/*
    language.hpp
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

#ifndef __LANGUAGE_HPP__
#define __LANGUAGE_HPP__

enum
{
  MEditCmpl,
  MComplete,

  MOk,
  MCancel,

  MMenuCfg,
  MAutoCfg,

  MWorkInsideWord,
  MCaseSensitive,
  MBrowseDownward,
  MConsiderDigitAsChar,
  MAddTrailingSpace,

  MBrowseLineCnt,
  MWordsToFindCnt,
  MMinWordLen,
  MAdditionalLetters,

  MAcceptFromMenu,
  MPartialCompletion,
  MMinPreWordLen,

  MColor,
  MAcceptChars,
  MAcceptKey,
  MDeleteKey,
  MKeyCfg,
  MPressDesiredKey,

  MSingleVariantInMenu,

  MNotFoundSound,
  MSortListCount,
  MAsteriskSymbol,

  MChooseWord,
  MHave,
};

#endif
