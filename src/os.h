
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __OS_H
#define __OS_H



#ifdef WIN32

// Don't use the Windows supplied min/max macros.  We use
// the std::min and std::max functions.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#else

#include <Carbon/Carbon.h>

#endif



#endif
