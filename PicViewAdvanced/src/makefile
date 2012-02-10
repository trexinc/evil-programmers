.SUFFIXES: .cc .cpp .c .o

CCWIDE = -D UNICODE -D _UNICODE

ifndef COMMON
COMMON = ../../common
endif

COMINC = $(COMMON)

OBJDIR = ../o
REOBJDIR = \.\.\/o\/
DLLDIR = ../bin
DLLNAME = PicViewAdv.dll
DLLFULLNAME = $(DLLDIR)/$(DLLNAME)
SRCS = PicViewAdv.cpp crt.cpp
DEF = PicViewAdv.gcc.def
DOCTARGET=$(DLLDIR)/PicViewAdv.lng
GFLLIB=libgfl340

CXX = g++
DLLTOOL = dlltool
RM = rm -f
CP = cp -f
M4 = m4 -P
MV = mv -f
MKDIR = mkdir -p
CXXFLAGS = -Os -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions $(CCWIDE) -I $(COMINC)
LNKFLAGS = -mdll -s -nostartfiles -nodefaultlibs -static-libgcc -luser32 -lkernel32 -ladvapi32 -lgdi32 -lmsvcrt -lgcc -L $(OBJDIR) -llibgfl340

all: dirs $(DLLFULLNAME) $(DOCTARGET)

OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(SRCS)))
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(filter %.cpp,$(SRCS)))
MAP = $(patsubst %.dll,%.map,$(DLLFULLNAME))

$(OBJDIR)/%.d: %.cpp
	@echo making depends for $<
	@$(MKDIR) $(OBJDIR)
	@$(SHELL) -ec '$(CXX) -c -MM $(CXXFLAGS) $< \
                | sed '\''s/\($*\)\.o[ :]*/$(REOBJDIR)\1.o $(REOBJDIR)\1.d: /g'\'' > $@; [ -s $@ ] || $(RM) $@'

$(OBJDIR)/%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DLLFULLNAME): $(OBJS) $(DEF) $(OBJDIR)/lib$(GFLLIB).a
	@echo linking $@
	@$(CXX) -mdll -o $(DLLNAME) -Xlinker --base-file -Xlinker $(DLLNAME).base $(OBJS) $(LNKFLAGS)
	@$(DLLTOOL) --dllname $(DLLNAME) --base-file $(DLLNAME).base --output-exp $(DLLNAME).exp --def $(DEF)
	@$(CXX) -mdll  -o $(DLLNAME) $(OBJS) $(DLLNAME).exp $(LNKFLAGS)
#	-Xlinker -Map -Xlinker $(MAP)
	@$(MV) $(DLLNAME) $(DLLDIR)
	@$(RM) $(DLLNAME).base
	@$(RM) $(DLLNAME).exp

$(DLLDIR)/%.lng: %.lng
	$(CP) $< $@

$(OBJDIR)/lib$(GFLLIB).a: $(GFLLIB).def
	dlltool --as=as -k --dllname $(GFLLIB).dll --output-lib $@ --def $<

.PHONY: dirs
dirs:
	@echo making folders
	@$(MKDIR) $(OBJDIR)
	@$(MKDIR) $(DLLDIR)

-include $(DEPS)
