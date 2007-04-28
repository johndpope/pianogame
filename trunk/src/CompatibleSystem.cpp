// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "CompatibleSystem.h"
#include "string_util.h"
#include "version.h"

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
// MACTODO: Timeval include
#endif


namespace Compatible
{
   unsigned long GetMilliseconds()
   {
      unsigned long milliseconds = 0;

#ifdef WIN32
      milliseconds = timeGetTime();
#else
      // MACTODO: Timeval
#endif

      return milliseconds;
   }


   const static std::wstring friendly_app_name = WSTRING(L"Synthesia " << SynthesiaVersionString);

   void ShowError(const std::wstring &err)
   {
#ifdef WIN32
      MessageBox(0, err.c_str(), WSTRING(friendly_app_name << L" Error")).c_str(), MB_ICONERROR);
#else
      // MACTODO: some kind of message box
#endif
   }

   void HideMouseCursor()
   {
#ifdef WIN32
      ShowCursor(false);
#else
      // MACTODO: hide the mouse cursor
#endif
   }
   
   void ShowMouseCursor()
   {
#ifdef WIN32
      ShowCursor(true);
#else
      // MACTODO: show the mouse cursor
#endif
   }


   void GracefulShutdown()
   {
#ifdef WIN32
      PostQuitMessage(0);
#else
      // MACTODO: Message to shut down application loop
#endif
   }

}; // End namespace