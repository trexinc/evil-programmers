TARGET = bci.exe

WFLAGS  =-Wall -W -Wpointer-arith -Wno-strict-aliasing #-Winline
EFLAGS  =-fomit-frame-pointer -fstrict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics #-mtune=core2
CXXFLAGS=-c -Os $(WFLAGS) $(EFLAGS)
LFLAGS  =-s -nostartfiles -mwindows
LIBS    =-lntdll -lgdi32 -lwinmm

CXX = g++
RC  = windres
RM  = rm -f

%.res: %.rc
	$(RC) -i $< -J rc -o $@ -O coff

%.o: %.cpp %.hpp %.h
	$(CXX) $(CXXFLAGS) $< -o $@

%.exe: %.res %.o
	$(CXX) $(LFLAGS) -o bin/$@ $^ $(LIBS)

all: $(TARGET)
