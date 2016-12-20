include(`ab_ver.m4')dnl
`#define VER_MAJOR' MAJOR
`#define VER_MINOR' MINOR
`#define VER_BUILD' BUILD
`#define VER_TYPE' TYPE2
`#define VER_ALLSTR' "MAJOR.MINOR build BUILD TYPE"
`#define VER_ALL' eval(MAJOR*1000000+MINOR*10000+BUILD)
`#define VER_API' API
`#define COPYRIGHT "Copyright © 2000-2016, Vadim Yegorov"'

`#define PLUG_DISPLAYNAME "Air Brush Plugin for FAR Manager 3.0+."'
`#define PLUG_INTERNALNAME "airbrush.dll"'
