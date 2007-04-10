WFLAGS=-ffor-scope -Wall -W -Wno-unused -Wpointer-arith -Winline
#-Wbad-function-cast
CFLAGS=-c -mtune=pentium4 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing
LFLAGS=-s
LIBS=-lnetapi32 -lntdll -lwinspool

RM=rm -f
CP=cp -f
CC=gcc
DLLTOOL=dlltool
M4=m4

DEF=userman.def
OBJS1=../o/userman.o ../o/um_utils.o ../o/um_dialogs0.o ../o/um_dialogs1.o ../o/um_dialogs2.o ../o/um_computer.o ../o/um_adds.o ../o/um_adds_internal.o ../o/um_dels.o ../o/um_manage_user.o ../o/um_manage_group.o ../o/um_acl.o ../o/um_childs.o ../o/um_arrays.o ../o/um_config.o
OBJS2=../o/um_userdata.o ../o/um_sid_cache.o ../o/um_current_user.o
OBJS3=../o/userman.res.o

OBJS=$(OBJS1) $(OBJS2) $(OBJS3)
TARGET=../bin/userman.dll
MAP = $(patsubst %.dll,%.map,$(TARGET))

DOCTARGET=../bin/userman.lng ../file_id.diz  ../bin/userman.hlf

all: $(TARGET) $(DOCTARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o nul -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ $^ $@.exp $(LIBS) -Xlinker -Map -Xlinker $(MAP)
	$(RM) $@.base
	$(RM) $@.exp

$(OBJS1): ../o/%.o: %.cpp umplugin.h
	$(CC) $(CFLAGS) $< -o $@

$(OBJS2): ../o/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

../o/userman.res.o: userman.rc ./bootstrap/umversion.h
	windres -i $< -o $@

./bootstrap/umversion.h: umversion.m4 um_ver.m4
	$(M4) $< > $@

../bin/userman.hlf: userman_hlf.m4 um_ver.m4
	$(M4) $< > $@

../bin/userman.lng: userman.lng
	$(CP) $< $@

../file_id.diz: file_id_diz.m4 um_ver.m4
	$(M4) $< > $@

clear:
	$(RM) ../o/*
	$(RM) ../bin/*
	$(RM) ./bootstrap/*
	$(RM) ../file_id.diz
