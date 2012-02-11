include(`fm_ver.m4')dnl
`#define VER_MAJOR' MAJOR
`#define VER_MINOR' MINOR
`#define VER_BUILD' BUILD
`#define VER_ALLSTR' "MAJOR.MINOR build BUILD TYPE"
`#define VER_ALL' eval(MAJOR*1000000+MINOR*10000+BUILD)
`#define COPYRIGHT "Copyright © 2002-2012, zg"'

`#define PLUG_DISPLAYNAME "FAR Mine Plugin for FAR Manager 3.0."'
`#define PLUG_INTERNALNAME "farmine.dll"'
