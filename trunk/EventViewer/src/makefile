ifdef WIDE
CCWIDE = -D UNICODE -D _UNICODE
DIRSURF = W
endif
ifndef COMMON
COMMON = ../../common
endif

ifdef WIDE
COMINC = $(COMMON)/unicode
else
COMINC = $(COMMON)/ascii
endif

OBJDIR=o$(DIRSURF)
BINDIR=bin$(DIRSURF)
BOOTSTRAPDIR=bootstrap$(DIRSURF)

CFLAGS=-c -mtune=pentium4 -Os -Wall -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics $(CCWIDE) -I $(COMINC)
LFLAGS=-s
LIBS=-lntdll

RM=rm
CC=gcc
DLLTOOL=dlltool
M4=m4
CP=cp
MKDIR=mkdir -p

DEF=ntevent$(DIRSURF).def
OBJS=../$(OBJDIR)/ntevent.o ../$(OBJDIR)/el_sid_cache.o ../$(OBJDIR)/el_category_cache.o ../$(OBJDIR)/ntevent.res.o
TARGET=../$(BINDIR)/ntevent.dll

DOCTARGET=../$(BINDIR)/ntevent.hlf ../$(BINDIR)/nteventr.hlf ../file_id.diz ../$(BINDIR)/ntevent.lng ../$(BINDIR)/nteventr.lng

all: dirs $(TARGET) $(DOCTARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ $^ $@.exp $(LIBS)
	$(RM) $@.base
	$(RM) $@.exp

../$(OBJDIR)/ntevent.o: ntevent.cpp el_utils.cpp evplugin.h el_config.cpp
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/el_sid_cache.o: el_sid_cache.cpp
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/el_category_cache.o: el_category_cache.cpp evplugin.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/ntevent.res.o: ntevent.rc ./$(BOOTSTRAPDIR)/evversion.h
	windres -I ./$(BOOTSTRAPDIR) -i $< -o $@

./$(BOOTSTRAPDIR)/evversion.h: evversion.m4 ev_ver.m4
	$(M4) $< > $@

../$(BINDIR)/ntevent.hlf: ntevent_hlf.m4 ev_ver.m4
	$(M4) $< > $@

../$(BINDIR)/nteventr.hlf: nteventr_hlf.m4 ev_ver.m4
	$(M4) $< > $@

../file_id.diz: file_id_diz.m4 ev_ver.m4
	$(M4) $< > $@

clear:
	$(RM) ../$(OBJDIR)/*
	$(RM) ../$(BINDIR)/*
	$(RM) ./$(BOOTSTRAPDIR)/*
	$(RM) ../file_id.diz

../$(BINDIR)/ntevent.lng: ntevent.lng
	$(CP) $< $@

../$(BINDIR)/nteventr.lng: nteventr.lng
	$(CP) $< $@

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)
	@$(MKDIR) ./$(BOOTSTRAPDIR)
