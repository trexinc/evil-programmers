m4_include(`fm_version.m4')m4_dnl
`#define VER_HI' "MAJOR"
`#define VER_LO' "MINOR"
`#define BUILD' "BLD"
`#define BUILD_YEAR' "BLD_YEAR"
`#define BUILD_MONTH' "BLD_MONTH"
`#define BUILD_DAY' "BLD_DAY"
#ifdef FARMAIL_SSL
#define COPYRIGHT1 "FARMail v%s.%s (build %s) with SSL support, compiled on %s.%s.%s."
#else
#define COPYRIGHT1 "FARMail v%s.%s (build %s), compiled on %s.%s.%s."
#endif
`#define COPYRIGHT2 "Copyright (C)' COPYRIGHT FARMail Group."
#define COPYRIGHT3 "Copyright (C) 1999-2000 Serge Alexandrov."
