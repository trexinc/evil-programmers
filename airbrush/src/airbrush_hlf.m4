include(`ab_ver.m4')dnl
.Language=English,English
.PluginContents=Air Brush
.Options CtrlColorChar=\

@Contents
$  `#'Air Brush MAJOR.MINOR build BUILD TYPE`#'

  This plugin provide syntax highlighing capability in
  internal FAR editor.

  ~Configure~@Config@
  ~Syntax file description~@Syntax@
  ~About~@About@

@Config
$  #Air Brush - configure#

  Here you can change:
  ~main options~@Config1@
  ~file masks for coresponding types~@Config3@
  ~first lines for coresponding types~@Config5@
  ~colors for coresponding types~@Config7@

@Config1
$  #Air Brush - main options#

  #Turn on plugin module#     - enable plugin.

  #Don't highlite files with# - if line count in file exceeds this
  #line count greater then#     value - plugin doesn't highlite this
                              file.

@Config2
$  #Air Brush - select type#

  Here you can select type and change its ~file mask~@Config3@.

@Config3
$  #Air Brush - file mask#

  Here you can change file mask for coresponding type.
  File mask include one or more simle file masks delimited with commas.

  Simle file masks may contain common valid file name symbols and
  special expressions:

   *           any number of characters;

   ?           any single character;

   [c,x-z]     any character enclosed by the brackets.
               Both separate characters and character intervals
               are allowed.

@Config4
$  #Air Brush - select type#

  Here you can select type and change its ~first line~@Config5@.

@Config5
$  #Air Brush - first line#

  Here you can change first line for coresponding type.

@Config6
$  #Air Brush - select type#

  Here you can select type and change its ~colors~@Config7@.

@Config7
$  #Air Brush - colors#

  Here you can select color settings for coresponding type.

@Color

  Here you can select color.

@Syntax
$  #Air Brush - syntax file description#

  #Syntax# file consists of several sections.
  Each section starts with a line of the form:

  #file# description mask [file start]

  Each section consists of several contexts.
  Each context starts with a line of the form:

  #context# [#exclusive#] [#whole#|#wholeright#|#wholeleft#] [#linestart#] delimiter [#linestart#] delimiter [foreground] [background]

  One exception is the first context. It must start with the command:

  #context# #default# [foreground] [background]

  The #linestart# option dictates that delimiter must start
  at the beginning of a line.

  The whole option tells that delim must be a whole word. What
  constitutes a whole word are a set of characters that can be
  changed at any point in the file with the wholechars command.
  To specify that a word must be whole on the left only, you can
  use the wholeleft option, and similarly on the right. The left
  and right set of characters can be set seperately with:

  #wholechars# [#left#|#right#] characters

  The #exclusive# option causes the text between the delimiters to
  be highlighted, but not the delimiters themselves.

  Each context may contain rules.
  Each rule is a line of the form:

  #keyword# [#whole#|#wholeright#|#wholeleft#] [#linestart#] [#recursive#] string [foreground] [background]

  Comment may be included on a line of there own and begin with a ##.

  Another file may be included:

  #include# filename

  Special sequences:

  #\n# - line feed.
  #\r# - cariage return.
  #\t# - tab.
  #\s# - space.
  #*#  - any symbol except #\n#.
  #+#  - any symbol except #\n#, #\t# and #\s#.

  Also you can use character sets.
  Set definition must be between #/# symbols.
  Character sets examples:
  #/[a-zA-Z]+/#
  #/[0-9]?/#
  #/[\040-\377]*/#
  #/[0-9]{1,5}/#
  #/[-,\[\]]{1,}/#

@About
$  #Air Brush - about#

  Copyright (c) 2000-2011 by Vadim Yegorov.

  E-mail: ~zg@@matrica.apollo.lv~@mailto:zg@@matrica.apollo.lv?subject=Air Brush plugin@
