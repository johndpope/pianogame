// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "CompatibleSystem.h"
#include "string_util.h"
#include "version.h"
#include "os.h"


namespace Compatible
{
   unsigned long GetMilliseconds()
   {
#ifdef WIN32
      return timeGetTime();
#else
      timeval tv;
      gettimeofday(&tv, 0);
      return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
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

      // We need to hide the window else the dialog is drawn *under* the main window
      WindowRef window = FrontWindow();
      HideWindow(window);
      
      // The cursor might have been hidden.
      ShowMouseCursor();

      CreateStandardAlert(kAlertStopAlert, MacStringFromWide(message_box_title).get(), MacStringFromWide(err).get(), 0, &dialog);
      RunStandardAlert(dialog, 0, &item);
      
      // We don't need to re-show the window.  This was an error.  The app is closing.

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
