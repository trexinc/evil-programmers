include(`ab_ver.m4')dnl
`#define VER_MAJOR' MAJOR
`#define VER_MINOR' MINOR
`#define VER_BUILD' BUILD
`#define VER_ALLSTR' "MAJOR.MINOR build BUILD TYPE"
`#define VER_ALL' eval(MAJOR*1000000+MINOR*10000+BUILD)
`#define VER_API' API
`#define COPYRIGHT "Copyright © 2000-2003, ZG"'

`#define PLUG_DISPLAYNAME "Air Brush Plugin for FAR Manager 1.7+."'
`#define PLUG_INTERNALNAME "airbrush.dll"'
