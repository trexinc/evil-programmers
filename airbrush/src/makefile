.SUFFIXES: .cc .cpp .c .o

CCWIDE = -D UNICODE -D _UNICODE

ifndef COMMON
COMMON = ../../common
endif

COMINC = $(COMMON)

RM=rm -rf
CC=gcc
DLLTOOL=dlltool
MAKE=make
M4=m4
WINDRES=windres
WINDRESPRE=
CP=cp
MKDIR=mkdir -p

OBJDIR=o$(DIRSURF)
BINDIR=bin$(DIRSURF)
FORMATDIR=$(BINDIR)/Formats
BOOTSTRAPDIR=bootstrap$(DIRSURF)

DEF=airbrush$(DIRSURF).def

WFLAGS=-ffor-scope -Wall -Wextra -Werror -Wno-empty-body -Wno-unused -Wno-parentheses -Wpointer-arith
#WFLAGS=-ffor-scope -Wall -W -Werror -Wtraditional -Wno-unused -Wpointer-arith -Wbad-function-cast -Wconversion -Winline -Wcast-qual
# -Wstrict-prototypes -Wconversion -Winline -Wcast-qual
CFLAGSBASE=-c -mtune=core2 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics $(CCWIDE)
CFLAGS=$(CFLAGSBASE) -I $(COMINC) -I ./$(BOOTSTRAPDIR)
LFLAGS=-mdll -s -nostartfiles
RCFLAGS=-I ./$(BOOTSTRAPDIR)

TARGET=../$(BINDIR)/airbrush.dll
OBJS=../$(OBJDIR)/far_settings.o ../$(OBJDIR)/ab_main.o ../$(OBJDIR)/ab_plugs.o ../$(OBJDIR)/ab_classes.o ../$(OBJDIR)/ab_colors.o ../$(OBJDIR)/airbrush.res.o
LIBS=-lrpcrt4
SUBDIRS=c pas sql html php re2c yacclex armgnuasm awk lua xml moon $(CUSTOM)
DOCTARGET=../$(BINDIR)/file_id.diz ../$(BINDIR)/airbrush.hlf ../$(BINDIR)/airbrush.lng
MAP = $(patsubst %.dll,%.map,$(TARGET))

all: dirs $(TARGET) $(SUBDIRS) $(DOCTARGET)

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)
	@$(MKDIR) ../$(FORMATDIR)
	@$(MKDIR) ./$(BOOTSTRAPDIR)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $@ -Xlinker --base-file -Xlinker $(TARGET).base $(OBJS) $(LIBS)
	$(DLLTOOL) --dllname $(TARGET) --base-file $(TARGET).base --output-exp $(TARGET).exp --def $(DEF)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(TARGET).exp $(LIBS) -Xlinker -Map -Xlinker $(MAP)
	$(RM) $(TARGET).base
	$(RM) $(TARGET).exp

clean:
	$(RM) ../$(OBJDIR)/*
	$(RM) ../$(BINDIR)/airbrush.*
	$(RM) ../$(BINDIR)/Formats/*
	$(RM) ./$(BOOTSTRAPDIR)/*
	$(RM) ../$(BINDIR)/file_id.diz

../$(OBJDIR)/ab_main.o: ab_main.cpp ab_main.h far_settings.h ./$(BOOTSTRAPDIR)/abplugin.h ./$(BOOTSTRAPDIR)/abversion.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/ab_plugs.o: ab_plugs.cpp ab_main.h far_settings.h ./$(BOOTSTRAPDIR)/abplugin.h ./$(BOOTSTRAPDIR)/abversion.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/ab_classes.o: ab_classes.cpp ab_main.h far_settings.h ./$(BOOTSTRAPDIR)/abplugin.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/ab_colors.o: ab_colors.cpp ab_main.h far_settings.h ./$(BOOTSTRAPDIR)/abplugin.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/far_settings.o: far_settings.cpp far_settings.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/airbrush.res.o: airbrush.rc ./$(BOOTSTRAPDIR)/abversion.h
	$(WINDRES) $(RCFLAGS) -i $< -o $@

./$(BOOTSTRAPDIR)/abversion.h: abversion.m4 ab_ver.m4
	$(M4) $< > $@

./$(BOOTSTRAPDIR)/abplugin.h: abplugin.m4 ab_ver.m4
	$(M4) $< > $@

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	(cd ./plugins/$@ && $(MAKE) CC="$(CC)" RM="$(RM)" MKDIR="$(MKDIR)" CP="$(CP)" DLLTOOL="$(DLLTOOL)" M4="$(M4)" WINDRES="$(WINDRES) $(WINDRESPRE) -I ../../$(BOOTSTRAPDIR)" CFLAGS="$(CFLAGSBASE) -I ../../$(BOOTSTRAPDIR)" LFLAGS="$(LFLAGS)" OBJDIR="o$(DIRSURF)" BINDIR="bin$(DIRSURF)" FORMATDIR="$(BINDIR)/Formats" BOOTSTRAPDIR="bootstrap$(DIRSURF)" -f makefile.sub;)

../$(BINDIR)/file_id.diz: file_id_diz.m4 ab_ver.m4
	$(M4) $< > $@

../$(BINDIR)/airbrush.hlf: airbrush_hlf.m4 ab_ver.m4
	$(M4) $< > $@

../$(BINDIR)/airbrush.lng: airbrush.lng
	$(CP) $< $@
