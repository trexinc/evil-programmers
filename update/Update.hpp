#pragma once

#define MSG(ID) Info.GetMsg(Info.ModuleNumber,ID) 

#ifndef UNICODE
#define EXP_NAME(function) function
#else
#define EXP_NAME(function) function ## W
#endif

#ifndef UNICODE
	#define MIN_FAR_MAJOR_VER 1
	#define MIN_FAR_MINOR_VER 71
	#define MIN_FAR_BUILD     2310
#else
	#define MIN_FAR_MAJOR_VER 2
	#define MIN_FAR_MINOR_VER 0
	#define MIN_FAR_BUILD     1006
#endif

