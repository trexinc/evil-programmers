#path to the common folder from far plugins SVN
ifndef COMMON
COMMON = ../../../FAR/fardev/plugins/common
endif

#ifdef WIDE
#SUFFIX=W
#ADDDEFINES=-D UNICODE -D _UNICODE
#COMINC = $(COMMON)/unicode
#else
COMINC = $(COMMON)/ascii
#endif

COMMONLIB = -L $(COMMON) -lCRT

OBJDIR = ../o$(SUFFIX)
REOBJDIR = \.\.\/o$(SUFFIX)\/
DLLDIR = ../bin$(SUFFIX)
DLLNAME = makeit.dll
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = makeit.cpp \
       config.cpp \
       pipesrv.cpp \
       hash.c \
       table.c \
       xmem.c \
       xmlite.c \
       RegExp.cpp \
       XTools.cpp

DEF = makeit$(SUFFIX).gcc.def

CC = gcc
CXX = g++
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
CCFLAGS  = -Os -I $(COMMON) -I $(COMINC) -Wall -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-exceptions $(ADDDEFINES)
CXXFLAGS = -Os -I $(COMMON) -I $(COMINC) -Wall -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions $(ADDDEFINES)
LNKFLAGS = -mdll -s -lwinmm

all: $(DLLFULLNAME)

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS))) $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(SRCS)))
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS))) $(patsubst %.c,$(OBJDIR)/%.d,$(filter %.c,$(SRCS)))
MAP = $(patsubst %.dll,%.map,$(DLLFULLNAME))

$(OBJDIR)/%.d: %.cpp
	@echo making depends for $<
	@$(MKDIR) $(@D)
	@$(SHELL) -ec '$(CXX) -c -MM $(CXXFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.cpp
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CC) $(CCFLAGS) -c -o $@ $<

$(DLLFULLNAME): $(OBJS) $(DEF)
	@echo linking $@
	@$(MKDIR) $(@D)
	@$(CXX) -mdll -o $(DLLNAME) -Xlinker --base-file -Xlinker $(DLLNAME).base $(OBJS) $(LNKFLAGS)
	@$(DLLTOOL) --dllname $(DLLNAME) --base-file $(DLLNAME).base --output-exp $(DLLNAME).exp --def $(DEF)
	@$(CXX) -mdll  -o $(DLLNAME) $(OBJS) $(DLLNAME).exp $(LNKFLAGS) -Xlinker -Map -Xlinker $(MAP)
	@$(MV) $(DLLNAME) $(DLLDIR)
	@$(RM) $(DLLNAME).base
	@$(RM) $(DLLNAME).exp
	@$(CP) makeitrus.hlf makeiteng.lng completed.wav makeit.xml $(DLLDIR)

-include $(DEPS)
