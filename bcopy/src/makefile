ifdef WIDE
CCWIDE = -D UNICODE -D _UNICODE
DIRSURF = W
endif
ifndef COMMON
COMMON = ../../common
endif

ifdef WIDE
COMINC = $(COMMON)/unicode
else
COMINC = $(COMMON)/ascii
endif

OBJDIR=o$(DIRSURF)
BINDIR=bin$(DIRSURF)
BOOTSTRAPDIR=bootstrap$(DIRSURF)

WFLAGS=-Wall -W -Wpointer-arith -Winline -Wno-strict-aliasing
# -Wno-unused
# -Werror -ffor-scope -Wbad-function-cast
CFLAGS=-c -mtune=core2 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -I ./$(BOOTSTRAPDIR)
CPPFLAGS=-c -mtune=core2 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics $(CCWIDE) -I $(COMINC)
LFLAGS=-s -nostartfiles
LIBS=-lmpr -lntdll
SVCLIBS=-lntdll
RCFLAGS=-I ./$(BOOTSTRAPDIR)

RM=rm -f
MV=mv -f
CP=cp -f
CC=gcc
DLLTOOL=dlltool
M4=m4
MKDIR=mkdir -p

DEF=bcopy$(DIRSURF).def
OBJS=../$(OBJDIR)/bcopy.o ../$(OBJDIR)/bcopy_common.o ../$(OBJDIR)/bcopy.res.o ../$(OBJDIR)/bcopy_info.o ../$(OBJDIR)/bcopy_infomenu.o ../$(OBJDIR)/bcopy_selcolor.o ../$(OBJDIR)/bcopy_macros_2.o ../$(OBJDIR)/bcopy_name.o ../$(OBJDIR)/bcopy_fast_redraw.o ../$(OBJDIR)/far_helper.o
TARGET=../$(BINDIR)/bcopy.dll

SVCOBJS=../$(OBJDIR)/bcsvc.o ../$(OBJDIR)/bcHandler.o ../$(OBJDIR)/bcThreads.o ../$(OBJDIR)/bcLog.o ../$(OBJDIR)/bcInfo.o ../$(OBJDIR)/bcNotify.o ../$(OBJDIR)/bcWipe.o ../$(OBJDIR)/bcsvc.res.o
SVCH=bcCommon.h bcsvc.h
SVCTARGET=../$(BINDIR)/bcsvc.exe

DOCTARGET=../$(BINDIR)/bcopy.hlf ../$(BINDIR)/bcopyru.hlf ../$(BINDIR)/bcopy.lng ../$(BINDIR)/bcopyru.lng ../file_id.diz

all: dirs $(SVCTARGET) $(TARGET) $(DOCTARGET)

$(SVCTARGET): $(SVCOBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(SVCLIBS)

../$(OBJDIR)/bcHandler.o: bcHandler.c bcToken.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/bcsvc.o: bcsvc.c $(SVCH) ./$(BOOTSTRAPDIR)/bcversion.h
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/bcThreads.o: bcThreads.c bcThreads_wrappers.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/bcLog.o: bcLog.c ./$(BOOTSTRAPDIR)/bcsvc.hxx ./$(BOOTSTRAPDIR)/bcversion.h ./$(BOOTSTRAPDIR)/bcsvc_MSG00001.bin
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/bcInfo.o: bcInfo.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/bcNotify.o: bcNotify.c $(SVCH)
	$(CC) $(CFLAGS) $< -o $@

../$(OBJDIR)/bcWipe.o: bcWipe.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll -o $@ $^ $@.exp $(LIBS)
	$(RM) $@.base
	$(RM) $@.exp

../$(OBJDIR)/bcopy.o: bcopy.cpp bcopy_find.cpp bcplugin.h bcplugdialogproc.cpp bcconfig.cpp bcopy_eject.cpp bcCommon.h dm_macro.hpp far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_common.o: bcopy_common.cpp bcplugin.h bcCommon.h far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_info.o: bcopy_info.cpp bcplugin.h bcCommon.h far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_name.o: bcopy_name.cpp bcplugin.h far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_infomenu.o: bcopy_infomenu.cpp bcplugin.h bcCommon.h far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_selcolor.o: bcopy_selcolor.cpp bcplugin.h bcCommon.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_macros_2.o: bcopy_macros_2.cpp bcplugin.h far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy_fast_redraw.o: bcopy_fast_redraw.cpp bcplugin.h bcopy_fast_redraw.h dm_macro.hpp far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/far_helper.o: far_helper.cpp far_helper.h
	$(CC) $(CPPFLAGS) $< -o $@

../$(OBJDIR)/bcopy.res.o: bcopy.rc ./$(BOOTSTRAPDIR)/bcversion.h
	windres $(RCFLAGS) -i $< -o $@

../$(OBJDIR)/bcsvc.res.o: bcsvc.rc ./$(BOOTSTRAPDIR)/bcversion.h ./$(BOOTSTRAPDIR)/bcsvc_MSG00001.bin
	windres $(RCFLAGS) -i $< -o $@

./$(BOOTSTRAPDIR)/bcsvc_MSG00001.bin ./$(BOOTSTRAPDIR)/bcsvc.hxx: bcsvc.mc
	windmc -b -r./$(BOOTSTRAPDIR) -h./$(BOOTSTRAPDIR) -ehxx $<

./$(BOOTSTRAPDIR)/bcversion.h: bcversion.m4 bc_ver.m4
	$(M4) $< > $@

../$(BINDIR)/bcopy.hlf: bcopy_hlf.m4 bc_ver.m4
	$(M4) $< > $@

../$(BINDIR)/bcopyru.hlf: bcopyru_hlf.m4 bc_ver.m4
	$(M4) $< > $@

../$(BINDIR)/bcopy.lng: bcopy.lng
	$(CP) $< $@

../$(BINDIR)/bcopyru.lng: bcopyru.lng
	$(CP) $< $@

../file_id.diz: file_id_diz.m4 bc_ver.m4
	$(M4) $< > $@

clear:
	$(RM) ../$(OBJDIR)/*
	$(RM) ../$(BINDIR)/*
	$(RM) ./$(BOOTSTRAPDIR)/*
	$(RM) ../file_id.diz

.PHONY: dirs
dirs:
	@$(MKDIR) ../$(OBJDIR)
	@$(MKDIR) ../$(BINDIR)
	@$(MKDIR) ./$(BOOTSTRAPDIR)
