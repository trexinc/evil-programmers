OBJDIR = ../o
REOBJDIR = \.\.\/o\/
EXEDIR = ../bin
EXENAME = hxsview.exe
EXEFULLNAME = $(EXEDIR)/$(EXENAME)
SRCS = hxsview.c litlib.c litdirectory.c litheaders.c litsections.c litutil.c lzxd.c lzxglue.c
LIBDIR = g:/lng/mingw/lib

CC = gcc
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
CCFLAGS = -Os -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-exceptions
LNKFLAGS = -s -nostartfiles -lcrtdll

all: $(EXEFULLNAME)

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(SRCS))) $(LIBDIR)/crt1.o
DEPS = $(patsubst %.c,$(OBJDIR)/%.d,$(filter %.c,$(SRCS)))
MAP = $(patsubst %.exe,%.map,$(EXEFULLNAME))

$(OBJDIR)/%.d: %.c
	@echo making depends for $<
	@$(SHELL) -ec '$(CC) -c -MM $(CCFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(CC) $(CCFLAGS) -c -o $@ $<

$(EXEFULLNAME): $(OBJS) $(DEF)
	@echo linking $@
	@$(CC) -o $(EXENAME) $(OBJS) $(LNKFLAGS)
	@$(MV) $(EXENAME) $(EXEDIR)

-include $(DEPS)
