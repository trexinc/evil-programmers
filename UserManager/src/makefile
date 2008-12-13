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

WFLAGS=-ffor-scope -Wall -W -Wno-unused -Wpointer-arith -Winline
#-Wbad-function-cast
CFLAGS=-c -mtune=pentium4 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics $(CCWIDE) -I $(COMINC)
LFLAGS=-s
LIBS=-lnetapi32 -lntdll -lwinspool

RM=rm -f
CP=cp -f
CC=gcc
DLLTOOL=dlltool
M4=m4
MKDIR=mkdir -p

DEF=userman$(DIRSURF).def
OBJS1=../$(OBJDIR)/userman.o ../$(OBJDIR)/um_utils.o ../$(OBJDIR)/um_dialogs0.o ../$(OBJDIR)/um_dialogs1.o ../$(OBJDIR)/um_dialogs2.o ../$(OBJDIR)/um_computer.o ../$(OBJDIR)/um_adds.o ../$(OBJDIR)/um_adds_internal.o ../$(OBJDIR)/um_dels.o ../$(OBJDIR)/um_manage_user.o ../$(OBJDIR)/um_manage_group.o ../$(OBJDIR)/um_acl.o ../$(OBJDIR)/um_childs.o ../$(OBJDIR)/um_arrays.o ../$(OBJDIR)/um_config.o
OBJS2=../$(OBJDIR)/um_userdata.o ../$(OBJDIR)/um_sid_cache.o ../$(OBJDIR)/um_current_user.o
OBJS3=../$(OBJDIR)/userman.res.o

OBJS=$(OBJS1) $(OBJS2) $(OBJS3)
TARGET=../$(BINDIR)/userman.dll
MAP = $(patsubst %.dll,%.map,$(TARGET))

DOCTARGET=../$(BINDIR)/userman.lng ../file_id.diz  ../$(BINDIR)/userman.hlf

all: dirs $(TARGET) $(DOCTARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ $^ $@.exp $(LIBS) -Xlinker -Map -Xlinker $(MAP)
	$(RM) $@.base
	$(RM) $@.exp

$(OBJS1): ../$(OBJDIR)/%.o: %.cpp umplugin.h
	$(CC) $(CFLAGS) $< -o $@

$(OBJS2): ../$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/userman.res.o: userman.rc ./$(BOOTSTRAPDIR)/umversion.h
	windres -I ./$(BOOTSTRAPDIR) $(CCWIDE) -i $< -o $@

./$(BOOTSTRAPDIR)/umversion.h: umversion.m4 um_ver.m4
	$(M4) $< > $@

../$(BINDIR)/userman.hlf: userman_hlf.m4 um_ver.m4
	$(M4) $< > $@

../$(BINDIR)/userman.lng: userman.lng
	$(CP) $< $@

../file_id.diz: file_id_diz.m4 um_ver.m4
	$(M4) $< > $@

clear:
	$(RM) ../$(OBJDIR)/*
	$(RM) ../$(BINDIR)/*
	$(RM) ./$(BOOTSTRAPDIR)/*
	$(RM) ../file_id.diz

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)
	@$(MKDIR) ./$(BOOTSTRAPDIR)
