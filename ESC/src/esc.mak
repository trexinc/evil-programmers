.AUTODEPEND

PATHALL = D:\PrgTools\BC5
CC      = $(PATHALL)\BIN\Bcc32.EXE
LN      = $(PATHALL)\BIN\TLink32.EXE
RC      = $(PATHALL)\BIN\Brcc32.EXE
IMPLIB  = D:\BCC55\BIN\ImpLib.EXE
TMP     = Tmp
FINAL   = Final

!ifdef _DEBUG
CompDebugOpt=-D_DEBUG -M -R
LinkDebugOpt=
OBJS = \
   $(TMP)\xmem.obj\
   $(TMP)\xmlite.obj\
   $(TMP)\table.obj\
   $(TMP)\hash.obj\
   $(TMP)\CEditorOptions.obj\
   $(TMP)\mix.obj\
   $(TMP)\SaveAndLoadConfig.obj\
   $(TMP)\strcon.obj\
   $(TMP)\e_options.obj\
   $(TMP)\myrtl.obj\
   $(TMP)\syslog.obj\
   $(TMP)\CIndicator.obj\
   $(TMP)\KeySequenceStorage.obj\
   $(TMP)\CUserMacros.obj\
   $(TMP)\esc.obj
!else
OBJS = \
   $(TMP)\xmem.obj\
   $(TMP)\xmlite.obj\
   $(TMP)\table.obj\
   $(TMP)\hash.obj\
   $(TMP)\CEditorOptions.obj\
   $(TMP)\mix.obj\
   $(TMP)\SaveAndLoadConfig.obj\
   $(TMP)\strcon.obj\
   $(TMP)\e_options.obj\
   $(TMP)\myrtl.obj\
   $(TMP)\CIndicator.obj\
   $(TMP)\KeySequenceStorage.obj\
   $(TMP)\CUserMacros.obj\
   $(TMP)\esc.obj
CompDebugOpt=-M- -R-
LinkDebugOpt=-x
!endif

LinkOpt = -V4.0 -Tpd -ap -v- -m -n $(LinkDebugOpt) -L$(PATHALL)\LIB -j$(TMP)
Minus   = -x- -RT- -k- -H- -v-
CompOpt = -c -4 $(CompDebugOpt) $(Minus) -O2 -n$(TMP) -I$(PATHALL)\INCLUDE -D_RTLDLL
LIBS    = import32
IMPLOpt = -a

Dep_esc = \
   esc.dll

all : tmp plugin

tmp :
	@if not exist $(TMP) md $(TMP)

plugin : esc

esc : $(Dep_esc)
	@if exist $(FINAL)\esc.dll del $(FINAL)\esc.dll
	@copy /b esc.dll $(FINAL)
	@del esc.dll
	@del $(TMP)\esc.res
	@echo MakeNode

Dep_escddll = \
   $(OBJS)\
   $(TMP)\esc.res

esc.dll : $(Dep_escddll)
	$(LN) @&&|
	$(LinkOpt) $(OBJS)
	$<,$*
	$(LIBS),,esc.res
|

$(TMP)\esc.res: ver.awk
	@awk -f ver.awk>esc.rc
	@$(RC) -R esc.rc -fo$(TMP)\esc.res
	@del esc.rc

$(TMP)\esc.obj :  esc.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ esc.cpp
|

$(TMP)\xmem.obj :  xmem.c
	$(CC) -c @&&|
	$(CompOpt) -o$@ xmem.c
|

$(TMP)\xmlite.obj :  xmlite.c
	$(CC) -c @&&|
	$(CompOpt) -o$@ xmlite.c
|

$(TMP)\table.obj :  table.c
	$(CC) -c @&&|
	$(CompOpt) -o$@ table.c
|

$(TMP)\hash.obj :  hash.c
	$(CC) -c @&&|
	$(CompOpt) -o$@ hash.c
|

$(TMP)\mix.obj :  mix.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ mix.cpp
|

$(TMP)\SaveAndLoadConfig.obj :  SaveAndLoadConfig.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ SaveAndLoadConfig.cpp
|

$(TMP)\myrtl.obj : myrtl.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ myrtl.cpp
|

$(TMP)\syslog.obj : syslog.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ syslog.cpp
|

$(TMP)\CIndicator.obj : CIndicator.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ CIndicator.cpp
|

$(TMP)\CEditorOptions.obj : CEditorOptions.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ CEditorOptions.cpp
|

$(TMP)\strcon.obj : strcon.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ strcon.cpp
|

$(TMP)\e_options.obj : e_options.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ e_options.cpp
|

$(TMP)\KeySequenceStorage.obj : KeySequenceStorage.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ KeySequenceStorage.cpp
|

$(TMP)\CUserMacros.obj : CUserMacros.cpp
	$(CC) -c @&&|
	$(CompOpt) -o$@ CUserMacros.cpp
|
