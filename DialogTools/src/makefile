.SUFFIXES: .cc .cpp .c .o

CCWIDE = -D UNICODE -D _UNICODE

ifndef COMMON
COMMON = ../../common
endif

COMINC = $(COMMON)

OBJDIR=o$(DIRSURF)
BINDIR=bin$(DIRSURF)

TARGET=../$(BINDIR)/dt.dll
SRCS=dt.cpp case.cpp pwd.cpp open_file_dialog.cpp openfiledialog.cpp undo.cpp pasteselection.cpp search.cpp replace.cpp edit.cpp far_helper.cpp memory.cpp
DEF=dt$(DIRSURF).def
OBJS=$(patsubst %.cpp,../$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS)))
DEPS=$(patsubst %.cpp,../$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS)))
LIBS=-lmsvcrt
DOCTARGET=../$(BINDIR)/dtEng.lng ../$(BINDIR)/dtRus.lng ../$(BINDIR)/Dialog_DialogTools.lua
MAP = $(patsubst %.dll,%.map,$(TARGET))

all: dirs $(TARGET) $(DOCTARGET)

CFLAGS=-c -Os -Wall -Werror -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-builtin -fomit-frame-pointer -fstrict-aliasing $(CCWIDE) -I $(COMINC)
LFLAGS=-s -nostartfiles

AS=as
RM=rm -f
CC=gcc
DLLTOOL=dlltool
MKDIR=mkdir -p
CP=cp

../$(OBJDIR)/%.d: %.cpp
	@$(MKDIR) ../$(OBJDIR)
	$(SHELL) -ec '$(CC) -c -MM $(CFLAGS) $< | sed '\''s/\($*\)\.o[ :]*/\.\.\/$(OBJDIR)\/\1.o \.\.\/$(OBJDIR)\/\1.d: /g'\'' > $@; [ -s $@ ] || rm -f $@'

../$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(DEF) $(OBJS) $(LIBS) $(LFLAGS) -Wl,--kill-at -shared -Xlinker -Map -Xlinker $(MAP)

../$(BINDIR)/%.lng: %.lng
	$(CP) $< $@

../$(BINDIR)/%.hlf: %.hlf
	$(CP) $< $@

../$(BINDIR)/%.lua: %.lua
	$(CP) $< $@

clear:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(DEPS)
