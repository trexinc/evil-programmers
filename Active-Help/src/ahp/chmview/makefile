OBJDIR = ../../../o/chmview
REOBJDIR = \.\.\/\.\.\/\.\.\/o\/chmview\/
DLLDIR = ../../../bin/ahp/chmview
DLLNAME = chmview.ahp
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = ahp_chmview.cpp chmlib.c lzx.c crt.cpp registry.cpp memory.cpp
DEF = chmview.gcc.def

CC = gcc
CXX = g++
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
CCFLAGS = -Wall -D__FARMANAGER__ -D__WINDOWS__ -Os -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-exceptions
CXXFLAGS = -Wall -D__FARMANAGER__ -D__WINDOWS__ -Os -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions
LNKFLAGS = -mdll -s -nostartfiles -static-libstdc++ -static-libgcc

all: $(DLLFULLNAME)

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS))) $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(SRCS)))
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS))) $(patsubst %.c,$(OBJDIR)/%.d,$(filter %.c,$(SRCS)))
MAP = $(patsubst %.ahp,%.map,$(DLLFULLNAME))

$(OBJDIR)/%.d: %.c
	@echo making depends for $<
	@$(MKDIR) $(@D)
	@$(SHELL) -ec '$(CC) -c -MM $(CCFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.d: %.cpp
	@echo making depends for $<
	@$(MKDIR) $(@D)
	@$(SHELL) -ec '$(CXX) -c -MM $(CXXFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DLLFULLNAME): $(OBJS) $(DEF)
	@echo linking $@
	@$(MKDIR) $(@D)
	@$(CXX) -mdll -o $(DLLNAME) -Xlinker --base-file -Xlinker $(DLLNAME).base $(OBJS) $(LNKFLAGS)
	@$(DLLTOOL) --dllname $(DLLNAME) --base-file $(DLLNAME).base --output-exp $(DLLNAME).exp --def $(DEF)
	@$(CXX) -mdll  -o $(DLLNAME) $(OBJS) $(DLLNAME).exp $(LNKFLAGS)
#-Xlinker -Map -Xlinker $(MAP)
	@$(MV) $(DLLNAME) $(DLLDIR)
	@$(RM) $(DLLNAME).base
	@$(RM) $(DLLNAME).exp

-include $(DEPS)
