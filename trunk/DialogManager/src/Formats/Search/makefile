PACKAGENAME = search
NAME = search
OBJDIR = ../../../o/$(PACKAGENAME)
REOBJDIR = \.\.\/\.\.\/\.\.\/o\/$(PACKAGENAME)\/
DLLDIR = ../../../bin/Formats/$(PACKAGENAME)
DLLNAME = $(NAME).fmt
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = $(NAME).cpp registry.cpp
DEF = $(NAME).gcc.def

CXX = g++
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4
MV = mv -f
MKDIR = mkdir -p
WINDRES=windres
CXXFLAGS = -Wall -Os -ffor-scope -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions
LNKFLAGS = -mdll -s -nostartfiles -lntdll


all: $(DLLFULLNAME)

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS))) $(OBJDIR)/$(NAME).res.o
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS)))
MAP = $(patsubst %.fmt,%.map,$(DLLFULLNAME))

$(OBJDIR)/%.d: %.cpp
	@echo making depends for $<
	@$(SHELL) -ec '$(CXX) -c -MM $(CXXFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/$(NAME).res.o: $(NAME).rc ../../bootstrap/dmversion.h
	$(WINDRES) -i $< -o $@

$(DLLFULLNAME): $(OBJS) $(DEF)
	@echo linking $@
	@$(CXX) -mdll -o $(DLLNAME) -Xlinker --base-file -Xlinker $(DLLNAME).base $(OBJS) $(LNKFLAGS)
	@$(DLLTOOL) --dllname $(DLLNAME) --base-file $(DLLNAME).base --output-exp $(DLLNAME).exp --def $(DEF)
	@$(CXX) -mdll  -o $(DLLNAME) $(OBJS) $(DLLNAME).exp $(LNKFLAGS)
#-Xlinker -Map -Xlinker $(MAP)
	@$(MV) $(DLLNAME) $(DLLDIR)
	@$(RM) $(DLLNAME).base
	@$(RM) $(DLLNAME).exp

-include $(DEPS)
