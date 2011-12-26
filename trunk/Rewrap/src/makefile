#path to the common folder from far plugins SVN
ifndef COMMON
COMMON = ../../../FAR/fardev/plugins/common
endif

ADDDEFINES=-D UNICODE -D _UNICODE
COMINC = $(COMMON)/unicode

COMMONLIB = -L $(COMMON) -lCRT

OBJDIR = ../o
REOBJDIR = \.\.\/o\/
DLLDIR = ../bin
DLLNAME = Rewrap.dll
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = rewrap.c
DEF = rewrap.gcc.def

CC = gcc
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
CCFLAGS = -Os -I $(COMMON) -I $(COMINC) -Wall -funsigned-char -fomit-frame-pointer -fstrict-aliasing $(ADDDEFINES)
LNKFLAGS = -mdll -s -nostartfiles -static-libgcc -static $(COMMONLIB)

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(SRCS)))
DEPS = $(patsubst %.c,$(OBJDIR)/%.d,$(filter %.c,$(SRCS)))
MAP = $(patsubst %.dll,%.map,$(DLLFULLNAME))

all: $(DLLFULLNAME)

$(OBJDIR)/%.d: %.c
	@echo making depends for $<
	@$(MKDIR) $(@D)
	@$(SHELL) -ec '$(CXX) -c -MM $(CXXFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CC) $(CCFLAGS) -c -o $@ $<

$(DLLFULLNAME): $(OBJS) $(DEF)
	@echo linking $@
	@$(MKDIR) $(@D)
	@$(CC) -mdll -o $(DLLNAME) -Xlinker --base-file -Xlinker $(DLLNAME).base $(OBJS) $(LNKFLAGS)
	@$(DLLTOOL) --dllname $(DLLNAME) --base-file $(DLLNAME).base --output-exp $(DLLNAME).exp --def $(DEF)
	@$(CC) -mdll  -o $(DLLNAME) $(OBJS) $(DLLNAME).exp $(LNKFLAGS)	-Xlinker -Map -Xlinker $(MAP)
	@$(MV) $(DLLNAME) $(DLLDIR)
	@$(RM) $(DLLNAME).base
	@$(RM) $(DLLNAME).exp

-include $(DEPS)
