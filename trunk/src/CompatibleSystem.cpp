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
#include <Carbon/Carbon.h>
#endif


namespace Compatible
{
   unsigned long GetMilliseconds()
   {
      unsigned long milliseconds = 0;

#ifdef WIN32
      milliseconds = timeGetTime();
#else
      timeval tv;
      gettimeofday(&tv, 0);
      
      milliseconds = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif

      return milliseconds;
   }


   void ShowError(const std::wstring &err)
   {
      const static std::wstring friendly_app_name = WSTRING(L"Synthesia " << SynthesiaVersionString);
      const static std::wstring message_box_title = WSTRING(friendly_app_name << L" Error");
      
#ifdef WIN32
      MessageBox(0, err.c_str(), message_box_title.c_str(), MB_ICONERROR);
#else
      
      DialogRef dialog;
      DialogItemIndex item;
      
      // TODO: Not Unicode!
      std::string narrow_err(err.begin(), err.end());
      std::string narrow_title(message_box_title.begin(), message_box_title.end());
      
      CFStringRef cf_err = CFStringCreateWithCString(0, narrow_err.c_str(), kCFStringEncodingMacRoman);      
      CFStringRef cf_title = CFStringCreateWithCString(0, narrow_title.c_str(), kCFStringEncodingMacRoman);

      WindowRef window = FrontWindow();
      HideWindow(window);
      
      // The cursor might have been hidden.
      ShowMouseCursor();

      CreateStandardAlert(kAlertStopAlert, cf_title, cf_err, 0, &dialog);
      RunStandardAlert(dialog, 0, &item);
      
      // We don't need to re-show the window.  This was an error.  The app is closing.
      
      CFRelease(cf_title);
      CFRelease(cf_err);

#endif
   }

   void HideMouseCursor()
   {
#ifdef WIN32
      ShowCursor(false);
#else
      CGDisplayHideCursor(kCGDirectMainDisplay);
#endif
   }
   
   void ShowMouseCursor()
   {
#ifdef WIN32
      ShowCursor(true);
#else
      CGDisplayShowCursor(kCGDirectMainDisplay);
#endif
   }


   int GetDisplayWidth()
   {
#ifdef WIN32
      return GetSystemMetrics(SM_CXSCREEN);
#else
      return int(CGDisplayBounds(kCGDirectMainDisplay).size.width);
#endif
   }

   int GetDisplayHeight()
   {
#ifdef WIN32
      return GetSystemMetrics(SM_CYSCREEN);
#else
      return int(CGDisplayBounds(kCGDirectMainDisplay).size.height);
#endif
   }


   void GracefulShutdown()
   {
#ifdef WIN32
      PostQuitMessage(0);
#else
      QuitApplicationEventLoop();
#endif
   }

}; // End namespace