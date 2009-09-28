# Target
TARGET  =bcn
APP_TP  =dll
EXT     =dll
DIR_BIN =bin32
DIR_OBJ =obj32
SRCS    =$(TARGET).cpp resource.rc
DEF     =$(TARGET).gcc.def
TARGETFULLNAME =$(DIR_BIN)/$(TARGET).$(EXT)

# Defines
DEFINES=-D UNICODE -D _UNICODE

# Flags
FLG_WARN =-Wall -W
FLG_COMM =-mno-cygwin
FLG_OPTS =-Os -funsigned-char -fomit-frame-pointer -fstrict-aliasing -fno-rtti -fno-exceptions -fno-threadsafe-statics #-mtune=core2
FLG_COMP =$(FLG_COMM) $(FLG_WARN) $(DEFINES) -I $(COM_INC) $(FLG_OPTS)
FLG_TYPE =-m$(APP_TP)
FLG_LIBS =-lgcc -lgdi32 -lwinmm
FLG_LINK =$(FLG_COMM) $(FLG_TYPE) -s

# Include
VPATH = ../inc/
#COM_INC = D:\src\inc
#COM_LIB = -L $(COMMON) -lCRT

# Commands
CXX   = g++
RCC   = windres
DLLTL = dlltool
RM    = rm -f
RMD   = rmdir --ignore-fail-on-non-empty
CP    = cp -f
M4    = m4 -P
MV    = mv -f
MKDIR = mkdir -p

#
OBJS = $(patsubst %.cpp, $(DIR_OBJ)/%.o,   $(filter %.cpp,$(SRCS)))
RESS = $(patsubst %.rc,  $(DIR_OBJ)/%.res, $(filter %.rc, $(SRCS)))

# make objs
$(DIR_OBJ)/%.o: %.cpp
	@echo compiling $<
	@$(MKDIR) $(DIR_OBJ)
	@$(CXX) $(FLG_COMP) -c -o $@ $<

# make resources
$(DIR_OBJ)/%.res: %.rc
	@echo compiling $<
	@$(MKDIR) $(DIR_OBJ)
	@$(RCC) -i $< -J rc -o $@ -O coff

# link
$(TARGETFULLNAME): $(OBJS) $(RESS) $(DEF)
	@echo linking $@
	@$(MKDIR) $(@D)
	@$(CXX) -o $(TARGETFULLNAME) $(FLG_LINK) $(OBJS) $(RESS) $(DEF) $(FLG_LIBS)

all: $(TARGETFULLNAME)

clean:
	@echo cleaning...
	@$(RM) $(TARGETFULLNAME)
	@$(RMD) $(DIR_BIN)
	@$(RM) -r $(DIR_OBJ)
