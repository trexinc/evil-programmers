#path to the common folder from far plugins SVN
ifndef COMMON
COMMON = ../../../FAR/fardev/plugins/common
endif

ifdef WIDE
SUFFIX=W
ADDDEFINES=-D UNICODE -D _UNICODE
COMINC = $(COMMON)/unicode
else
COMINC = $(COMMON)/ascii
endif

#COMMONLIB = -L $(COMMON) -lCRT

OBJDIR = ../o$(SUFFIX)
REOBJDIR = \.\.\/o$(SUFFIX)\/
DLLDIR = ../bin$(SUFFIX)
DLLNAME = esc.dll
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = hash.c \
table.c \
xmem.c \
xmlite.c \
CEditorOptions.cpp \
CIndicator.cpp \
CUserMacros.cpp \
e_options.cpp \
esc.cpp \
KeySequenceStorage.cpp \
mix.cpp \
myrtl.cpp \
rbtree.cpp \
SaveAndLoadConfig.cpp \
strcon.cpp \
syslog.cpp \
TArray.cpp \
XMLite2.cpp
DEF = esc$(SUFFIX).gcc.def

CC = gcc
CXX = g++
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
WINDRES = windres
CCFLAGS = -mno-cygwin -Os -I $(COMMON) -I $(COMINC) -Wall -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-exceptions $(ADDDEFINES)
CXXFLAGS = -mno-cygwin -Os -I $(COMMON) -I $(COMINC) -Wall -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions -fno-threadsafe-statics $(ADDDEFINES)
LNKFLAGS = -mno-cygwin -mdll -s -nostartfiles
#-lgcc $(COMMONLIB)

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS))) $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(SRCS))) $(OBJDIR)/esc.res.o
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS))) $(patsubst %.c,$(OBJDIR)/%.d,$(filter %.c,$(SRCS)))
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

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(MKDIR) $(@D)
	@$(CC) $(CCFLAGS) -c -o $@ $<

esc.rc: ver.awk
	@echo Generating rc
	@awk -f ver.awk > esc.rc

$(OBJDIR)/esc.res.o: esc.rc
	@echo compiling $<
	@$(MKDIR) -p $(@D)
	@$(WINDRES) $(RCFLAGS) -i $< -o $@

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
