OBJDIR=o
BINDIR=bin

WFLAGS=-Wall -W -Wpointer-arith -Winline -Wno-strict-aliasing -Wno-unused -Werror
CFLAGS=-c -mtune=core2 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing
CPPFLAGS=-c -mtune=core2 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics
LFLAGS=-s -nostartfiles
LIBS=-lmsvcrt

RM=rm -f
MV=mv -f
CP=cp -f
CC=gcc
DLLTOOL=dlltool
M4=m4
MKDIR=mkdir -p

DEF=bcn.def
OBJS=../$(OBJDIR)/bcn.o
TARGET=../$(BINDIR)/bcn.dll

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -o $@ $^ $@.exp $(LIBS)
	$(RM) $@.base
	$(RM) $@.exp

../$(OBJDIR)/bcn.o: bcn.cpp ../../src/bcsvc_interface.h
	$(CC) $(CPPFLAGS) $< -o $@

clear:
	$(RM) ../$(OBJDIR)/*
	$(RM) ../$(BINDIR)/*

.PHONY: dirs

dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)
