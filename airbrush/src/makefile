.SUFFIXES: .cc .cpp .c .o

TARGET=../bin/airbrush.dll
DEF=airbrush.def
OBJS=../o/ab_main.o ../o/ab_plugs.o ../o/ab_selcolor.o ../o/ab_classes.o ../o/airbrush.res.o
LIBS=
SUBDIRS=php html zcustom c re2c pas sql yacclex
DOCTARGET=../file_id.diz ../bin/airbrush.hlf ../bin/airbrush.lng

all: $(TARGET) $(SUBDIRS) $(DOCTARGET)

WFLAGS=-ffor-scope -Wall -Wextra -Werror -Wno-empty-body -Wno-unused -Wno-parentheses -Wpointer-arith -Wcast-qual
#WFLAGS=-ffor-scope -Wall -W -Werror -Wtraditional -Wno-unused -Wpointer-arith -Wbad-function-cast -Wconversion -Winline -Wcast-qual
# -Wstrict-prototypes -Wconversion -Winline
CFLAGS=-c -mtune=pentium4 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti
LFLAGS=-mdll -s -nostartfiles

RM=rm -f
CC=gcc
DLLTOOL=dlltool
MAKE=make
M4=m4
WINDRES=windres
WINDRESPRE=--preprocessor \"\\\"gcc -E -xc -DRC_INVOKED\\\"\"
CP=cp

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $@ -Xlinker --base-file -Xlinker $(TARGET).base $(OBJS) $(LIBS)
	$(DLLTOOL) --dllname $(TARGET) --base-file $(TARGET).base --output-exp $(TARGET).exp --def $(DEF)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(TARGET).exp $(LIBS)
	$(RM) $(TARGET).base
	$(RM) $(TARGET).exp

clear:
	$(RM) ../o/*
	$(RM) ../bin/airbrush.*
	$(RM) ../bin/Formats/*
	$(RM) ./bootstrap/*
	$(RM) ../file_id.diz

../o/ab_main.o: ab_main.cpp ab_main.h ./bootstrap/abplugin.h ./bootstrap/abversion.h
	$(CC) $(CFLAGS) $< -o $@

../o/ab_plugs.o: ab_plugs.cpp ab_main.h ./bootstrap/abplugin.h ./bootstrap/abversion.h
	$(CC) $(CFLAGS) $< -o $@

../o/ab_selcolor.o: ab_selcolor.cpp ab_main.h
	$(CC) $(CFLAGS) $< -o $@

../o/ab_classes.o: ab_classes.cpp ab_main.h ./bootstrap/abplugin.h
	$(CC) $(CFLAGS) $< -o $@

../o/airbrush.res.o: airbrush.rc ./bootstrap/abversion.h
	$(WINDRES) -i $< -o $@

./bootstrap/abversion.h: abversion.m4 ab_ver.m4
	$(M4) $< > $@

./bootstrap/abplugin.h: abplugin.m4 ab_ver.m4
	$(M4) $< > $@

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	(cd ./plugins/$@ && $(MAKE) CC="$(CC)" RM="$(RM)" CP="$(CP)" DLLTOOL="$(DLLTOOL)" M4="$(M4)" WINDRES="$(WINDRES) $(WINDRESPRE)" CFLAGS="$(CFLAGS)" LFLAGS="$(LFLAGS)" -f makefile.sub;)

../file_id.diz: file_id_diz.m4 ab_ver.m4
	$(M4) $< > $@

../bin/airbrush.hlf: airbrush_hlf.m4 ab_ver.m4
	$(M4) $< > $@

../bin/airbrush.lng: airbrush.lng
	$(CP) $< $@
