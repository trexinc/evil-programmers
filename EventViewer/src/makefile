CFLAGS=-c -mcpu=pentiumpro -Os -Wall -fomit-frame-pointer -fstrict-aliasing
LFLAGS=-s
LIBS=-lntdll

RM=rm
CC=gcc
DLLTOOL=dlltool
M4=m4
CP=cp

DEF=ntevent.def
OBJS=../o/ntevent.o ../o/el_sid_cache.o ../o/el_category_cache.o ../o/ntevent.res.o
TARGET=../bin/ntevent.dll

DOCTARGET=../bin/ntevent.hlf ../bin/nteventr.hlf ../file_id.diz ../bin/ntevent.lng ../bin/nteventr.lng

all: $(TARGET) $(DOCTARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -nostartfiles -o $@ $^ $@.exp $(LIBS)
	$(RM) $@.base
	$(RM) $@.exp

../o/ntevent.o: ntevent.cpp el_utils.cpp evplugin.h el_config.cpp
	$(CC) $(CFLAGS) $< -o $@

../o/el_sid_cache.o: el_sid_cache.cpp
	$(CC) $(CFLAGS) $< -o $@

../o/el_category_cache.o: el_category_cache.cpp evplugin.h
	$(CC) $(CFLAGS) $< -o $@

../o/ntevent.res.o: ntevent.rc ./bootstrap/evversion.h
	windres -i $< -o $@

./bootstrap/evversion.h: evversion.m4 ev_ver.m4
	$(M4) $< > $@

../bin/ntevent.hlf: ntevent_hlf.m4 ev_ver.m4
	$(M4) $< > $@

../bin/nteventr.hlf: nteventr_hlf.m4 ev_ver.m4
	$(M4) $< > $@

../file_id.diz: file_id_diz.m4 ev_ver.m4
	$(M4) $< > $@

clear:
	$(RM) ../o/*
	$(RM) ../bin/*
	$(RM) ./bootstrap/*
	$(RM) ../file_id.diz

../bin/ntevent.lng: ntevent.lng
	$(CP) $< $@

../bin/nteventr.lng: nteventr.lng
	$(CP) $< $@
