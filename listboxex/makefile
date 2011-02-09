CCWIDE = -D UNICODE -D _UNICODE

ifndef COMMON
COMMON = ../common
endif

COMINC = $(COMMON)

CFLAGS=-c -march=pentium4 -Os -Wall -I $(COMINC) $(CCWIDE)
LFLAGS=-s
LIBS=

RM=rm
CC=gcc
DLLTOOL=dlltool
M4=m4

DEF=test$(DIRSURF).def
OBJS=test.o ListBoxEx.o
TARGET=test.dll
MAP=$(patsubst %.dll,%.map,$(TARGET))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -mdll -o $@ -Xlinker --base-file -Xlinker $@.base $^ $(LIBS)
	$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
	$(CC) $(LFLAGS) -mdll  -o $@ $^ $@.exp $(LIBS) -Xlinker -Map -Xlinker $(MAP)
	$(RM) $@.base
	$(RM) $@.exp

test.o: test.cpp ListBoxEx.hpp
	$(CC) $(CFLAGS) $< -o $@

ListBoxEx.o: ListBoxEx.cpp ListBoxEx.hpp
	$(CC) $(CFLAGS) $< -o $@

clear:
	$(RM) *.o
	$(RM) $(TARGET)
