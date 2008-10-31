WFLAGS=-Wall -W -Wpointer-arith -Winline -Wno-strict-aliasing
# -Wno-unused
# -Werror -ffor-scope -Wbad-function-cast
CFLAGS=-c -mtune=pentium4 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing
CPPFLAGS=-c -mtune=pentium4 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics
LFLAGS=-s -nostartfiles
LIBS=-lmpr -lntdll
SVCLIBS=-lntdll

RM=rm -f
MV=mv -f
CP=cp -f
CC=gcc
DLLTOOL=dlltool
M4=m4

DEF=bcopy.def
OBJS=../o/bcopy.o ../o/bcopy_common.o ../o/bcopy.res.o ../o/bcopy_info.o ../o/bcopy_infomenu.o ../o/bcopy_selcolor.o ../o/bcopy_macros.o ../o/bcopy_macros_2.o ../o/bcopy_name.o ../o/bcopy_fast_redraw.o
TARGET=../bin/bcopy.dll

SVCOBJS=../o/bcsvc.o ../o/bcHandler.o ../o/bcThreads.o ../o/bcLog.o ../o/bcInfo.o ../o/bcNotify.o ../o/bcWipe.o ../o/bcsvc.res.o
SVCH=bcCommon.h bcsvc.h
SVCTARGET=../bin/bcsvc.exe

DOCTARGET=../bin/bcopy.hlf ../bin/bcopyru.hlf ../bin/bcopy.lng ../bin/bcopyru.lng ../file_id.diz

all: $(SVCTARGET) $(TARGET) $(DOCTARGET)

$(SVCTARGET): $(SVCOBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(SVCLIBS)

../o/bcHandler.o: bcHandler.c bcToken.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../o/bcsvc.o: bcsvc.c $(SVCH) ./bootstrap/bcversion.h
	$(CC) $(CFLAGS) $< -o $@

../o/bcThreads.o: bcThreads.c bcThreads_wrappers.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../o/bcLog.o: bcLog.c ./bootstrap/bcsvc.h ./bootstrap/bcversion.h ./bootstrap/bcsvc_MSG00001.bin
	$(CC) $(CFLAGS) $< -o $@

../o/bcInfo.o: bcInfo.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../o/bcNotify.o: bcNotify.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../o/bcWipe.o: bcWipe.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -o $@ $^ $@.exp $(LIBS)
	$(RM) $@.base
	$(RM) $@.exp

../o/bcopy.o: bcopy.cpp bcopy_find.cpp bcplugin.h bcplugdialogproc.cpp bcconfig.cpp bcopy_eject.cpp bcCommon.h dm_macro.hpp
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_common.o: bcopy_common.cpp bcplugin.h bcCommon.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_info.o: bcopy_info.cpp bcplugin.h bcCommon.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_name.o: bcopy_name.cpp bcplugin.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_infomenu.o: bcopy_infomenu.cpp bcplugin.h bcCommon.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_selcolor.o: bcopy_selcolor.cpp bcplugin.h bcCommon.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_macros.o: bcopy_macros.cpp bcplugin.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_macros_2.o: bcopy_macros_2.cpp bcplugin.h
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy_fast_redraw.o: bcopy_fast_redraw.cpp bcplugin.h bcopy_fast_redraw.h dm_macro.hpp
	$(CC) $(CPPFLAGS) $< -o $@

../o/bcopy.res.o: bcopy.rc ./bootstrap/bcversion.h
	windres -i $< -o $@

../o/bcsvc.res.o: bcsvc.rc ./bootstrap/bcversion.h ./bootstrap/bcsvc_MSG00001.bin
	windres -i $< -o $@

./bootstrap/bcsvc_MSG00001.bin ./bootstrap/bcsvc.h: bcsvc.mc
	windmc -b -r.\bootstrap -h.\bootstrap $<

./bootstrap/bcversion.h: bcversion.m4 bc_ver.m4
	$(M4) $< > $@

../bin/bcopy.hlf: bcopy_hlf.m4 bc_ver.m4
	$(M4) $< > $@

../bin/bcopyru.hlf: bcopyru_hlf.m4 bc_ver.m4
	$(M4) $< > $@

../bin/bcopy.lng: bcopy.lng
	$(CP) $< $@

../bin/bcopyru.lng: bcopyru.lng
	$(CP) $< $@

../file_id.diz: file_id_diz.m4 bc_ver.m4
	$(M4) $< > $@

clear:
	$(RM) ../o/*
	$(RM) ../bin/*
	$(RM) ./bootstrap/*
	$(RM) ../file_id.diz
