include(`bc_ver.m4')dnl
define(DISPLAYNAME,"FAR Background Copy Service")dnl
dnl
`#define VER_MAJOR' MAJOR
`#define VER_MINOR' MINOR
`#define VER_BUILD' BUILD
`#define VER_ALLSTR' "MAJOR.MINOR build BUILD TYPE"
`#define COPYRIGHT "Copyright © 2000-2008, ZG"'

`#define PLUG_DISPLAYNAME "Background copy Plugin for FAR Manager 1.7+."'
`#define PLUG_INTERNELNAME "bcopy.dll"'

`#define SVC_INTERNELNAME "bcsvc.exe"'
`#define SVC_DISPLAYNAMEA' DISPLAYNAME
`#define SVC_DISPLAYNAMEW' `L'DISPLAYNAME
`#define VERSION SVC_DISPLAYNAMEA". Version "VER_ALLSTR", 'DATE`.\n"'
