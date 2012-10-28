.SUFFIXES: .cc .cpp .c .o

CCWIDE = -D UNICODE -D _UNICODE

ifndef COMMON
COMMON = ../../common
endif

COMINC = $(COMMON)

OBJDIR=o$(DIRSURF)
BINDIR=bin$(DIRSURF)
BOOTSTRAPDIR=bootstrap$(DIRSURF)

TARGET=../$(BINDIR)/farmine.dll
DEF=farmine.def
OBJS=../$(OBJDIR)/farmine.o ../$(OBJDIR)/fm_config.o ../$(OBJDIR)/far_settings.o ../$(OBJDIR)/farmine.res.o
LIBS=-lmsvcrt
DOCTARGET=../file_id.diz ../$(BINDIR)/farmine.lng ../$(BINDIR)/farmine.hlf

all: dirs $(TARGET) $(DOCTARGET)

WFLAGS=-ffor-scope -Wall -W -Werror -Wpointer-arith -Wcast-qual -Wconversion -Winline
CFLAGS=-c -mtune=core2 -Os $(WFLAGS) -fno-rtti -fno-exceptions -fomit-frame-pointer -fstrict-aliasing $(CCWIDE) -I $(COMINC)
LFLAGS=-mdll -s -nostartfiles

RM=rm -fr
CC=gcc
DLLTOOL=dlltool
MAKE=make
M4=m4
WINDRES=windres
CP=cp
MKDIR=mkdir -p

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $@ -Xlinker --base-file -Xlinker $(TARGET).base $(OBJS) $(LIBS)
	$(DLLTOOL) --dllname $(TARGET) --base-file $(TARGET).base --output-exp $(TARGET).exp --def $(DEF)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(TARGET).exp $(LIBS)
	$(RM) $(TARGET).base
	$(RM) $(TARGET).exp

clear:
	$(RM) ../$(OBJDIR)
	$(RM) ../$(BINDIR)
	$(RM) ./$(BOOTSTRAPDIR)
	$(RM) ../file_id.diz

../$(OBJDIR)/farmine.o: farmine.cpp farmine.hpp ./$(BOOTSTRAPDIR)/fmversion.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/fm_config.o: fm_config.cpp farmine.hpp
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/far_settings.o: far_settings.cpp far_settings.hpp
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/farmine.res.o: farmine.rc ./$(BOOTSTRAPDIR)/fmversion.h
	$(WINDRES) -i $< -o $@

./$(BOOTSTRAPDIR)/fmversion.h: fmversion.m4 fm_ver.m4
	$(M4) $< > $@

../file_id.diz: file_id_diz.m4 fm_ver.m4
	$(M4) $< > $@

../$(BINDIR)/farmine.lng: farmine.lng
	$(CP) $< $@

../$(BINDIR)/farmine.hlf: farmine_hlf.m4 fm_ver.m4
	$(M4) $< > $@

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)
	@$(MKDIR) ./$(BOOTSTRAPDIR)
