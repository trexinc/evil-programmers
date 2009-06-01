#path to the common folder from far plugins SVN
ifndef COMMON
COMMON = ../../../FAR/fardev/plugins/common
endif

ADDDEFINES=-D UNICODE -D _UNICODE
COMINC = $(COMMON)/unicode

#COMMONLIB = -L $(COMMON) -lCRT

OBJDIR = ../o
REOBJDIR = \.\.\/o\/
DLLDIR = ../bin
DLLNAME = Crapculator.dll
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = Crapculator.cpp LogicalExpression.cpp NumericalExpression.cpp
DEF = Crapculator.gcc.def

CXX = g++
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
CXXFLAGS = -mno-cygwin -Os -I $(COMMON) -I $(COMINC) -Wall -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions $(ADDDEFINES)
LNKFLAGS = -mno-cygwin -mdll -s -nostartfiles -lgcc

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS)))
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS)))
MAP = $(patsubst %.dll,%.map,$(DLLFULLNAME))

all: $(DLLFULLNAME)

$(OBJDIR)/%.d: %.cpp
	@echo making depends for $<
	@$(MKDIR) $(@D)
	@$(SHELL) -ec '$(CXX) -c -MM $(CXXFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.cpp
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DLLFULLNAME): $(OBJS) $(DEF)
	@echo linking $@
	@$(MKDIR) $(@D)
	@$(CXX) -mdll -o $(DLLNAME) -Xlinker --base-file -Xlinker $(DLLNAME).base $(OBJS) $(LNKFLAGS)
	@$(DLLTOOL) --dllname $(DLLNAME) --base-file $(DLLNAME).base --output-exp $(DLLNAME).exp --def $(DEF)
	@$(CXX) -mdll  -o $(DLLNAME) $(OBJS) $(DLLNAME).exp $(LNKFLAGS)	-Xlinker -Map -Xlinker $(MAP)
	@$(MV) $(DLLNAME) $(DLLDIR)
	@$(RM) $(DLLNAME).base
	@$(RM) $(DLLNAME).exp

-include $(DEPS)
