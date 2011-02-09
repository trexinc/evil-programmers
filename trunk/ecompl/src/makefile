.SUFFIXES: .cc .cpp .c .o

CCWIDE = -D UNICODE -D _UNICODE

ifndef COMMON
COMMON = ../../common
endif

COMINC = $(COMMON)

OBJDIR=o$(DIRSURF)
BINDIR=bin$(DIRSURF)

TARGET=../$(BINDIR)/EditCmpl.dll
SRCS=EditCmpl.cpp MatPat.cpp cmpl.cpp mcmpl.cpp acmpl.cpp avl_words.cpp avl_windows.cpp string.cpp memory.cpp SelColor.cpp
DEF=EditCmpl$(DIRSURF).def
OBJS1=$(patsubst %.cpp,../$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS)))
DEPS=$(patsubst %.cpp,../$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS)))
LIBS=-lcrtdll
OBJS=$(OBJS1) ../$(OBJDIR)/ListBoxEx.o
DOCTARGET=../$(BINDIR)/ECmplEng.hlf ../$(BINDIR)/ECmplRus.hlf ../$(BINDIR)/ECmplEng.lng ../$(BINDIR)/ECmplRus.lng

all: dirs $(TARGET) $(DOCTARGET)

#size
CFLAGS=-c -Os -Wall -Werror -Wno-unused -fno-rtti -fno-exceptions -fno-builtin -fomit-frame-pointer -fstrict-aliasing $(CCWIDE) -I $(COMINC)
#speed
#CFLAGS=-c -mcpu=pentiumpro -O3 -Wall -Werror -Wno-unused -fno-rtti -fno-exceptions -fomit-frame-pointer -fstrict-aliasing $(CCWIDE) -I $(COMINC)
LFLAGS=-mdll -s -nostartfiles

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

../$(OBJDIR)/ListBoxEx.o: ../../listboxex/ListBoxEx.cpp
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $@ -Xlinker --base-file -Xlinker $(TARGET).base $(OBJS) $(LIBS)
	$(DLLTOOL) --dllname $(TARGET) --base-file $(TARGET).base --output-exp $(TARGET).exp --def $(DEF)
	$(CC) $(LFLAGS) -o $@ $(OBJS) $(TARGET).exp $(LIBS)
	$(RM) $(TARGET).base
	$(RM) $(TARGET).exp

../$(BINDIR)/%.lng: %.lng
	$(CP) $< $@

../$(BINDIR)/%.hlf: %.hlf
	$(CP) $< $@

clear:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(DEPS)
