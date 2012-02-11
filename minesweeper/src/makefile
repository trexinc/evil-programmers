.SUFFIXES: .cc .cpp .c .o

TARGET=../bin/farmine.dll
DEF=farmine.def
OBJS=../o/farmine.o ../o/fm_config.o ../o/farmine.res.o
LIBS=-lcrtdll
DOCTARGET=../file_id.diz ../bin/farmine.lng ../bin/farmine.hlf

all: $(TARGET) $(DOCTARGET)

WFLAGS=-ffor-scope -Wall -W -Werror -Wno-unused -Wpointer-arith -Wcast-qual -Wconversion -Winline
CFLAGS=-c -mcpu=pentium -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing
LFLAGS=-mdll -s -nostartfiles

RM=rm -f
CC=gcc
DLLTOOL=dlltool
MAKE=make
M4=m4
WINDRES=windres
CP=cp

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o nul -Xlinker --base-file -Xlinker $(TARGET).base $(OBJS) $(LIBS)
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

../o/farmine.o: farmine.cpp farmine.hpp
	$(CC) $(CFLAGS) $< -o $@

../o/fm_config.o: fm_config.cpp farmine.hpp
	$(CC) $(CFLAGS) $< -o $@

../o/farmine.res.o: farmine.rc ./bootstrap/fmversion.h
	$(WINDRES) -i $< -o $@

./bootstrap/fmversion.h: fmversion.m4 fm_ver.m4
	$(M4) $< > $@

../file_id.diz: file_id_diz.m4 fm_ver.m4
	$(M4) $< > $@

../bin/farmine.lng: farmine.lng
	$(CP) $< $@

../bin/farmine.hlf: farmine_hlf.m4 fm_ver.m4
	$(M4) $< > $@
