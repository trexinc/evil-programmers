OBJDIR = ../o
REOBJDIR = \.\.\/o\/
EXEDIR = ../bin
EXENAME = chmview.exe
EXEFULLNAME = $(EXEDIR)/$(EXENAME)
SRCS = chmview.c chm_lib.c lzx.c utf8.c

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
#MAP = $(patsubst %.exe,%.map,$(EXEFULLNAME))

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
#	@$(RM) $(EXENAME).base
#	@$(RM) $(EXENAME).exp

-include $(DEPS)
