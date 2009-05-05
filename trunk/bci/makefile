WFLAGS=-Wall -W -Wpointer-arith -Winline -Wno-strict-aliasing
CPPFLAGS=-c -mtune=core2 -Os $(WFLAGS) -fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics
LFLAGS=-s -nostartfiles -mwindows
LIBS=-lntdll -lgdi32 -lwinmm

RM=rm -f
CC=gcc

all: bci.exe

bci.exe: bci.o
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

bci.o: bci.cpp bci.h
	$(CC) $(CPPFLAGS) $< -o $@
