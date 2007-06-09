// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "os.h"
#include "os_graphics.h"

#include <set>
#include <string>
#include "string_util.h"
#include "file_selector.h"
#include "UserSettings.h"
#include "version.h"
#include "resource.h"

#include "CompatibleSystem.h"
#include "SynthesiaError.h"
#include "libmidi/Midi.h"
#include "libmidi/SynthVolume.h"

#include "Tga.h"
#include "Renderer.h"
#include "SharedState.h"
#include "GameState.h"
#include "State_Title.h"

using namespace std;

#ifdef WIN32

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#else

static std::wstring external_command_line(L"");

static AGLContext aglContext;
static WindowRef window(0);

static void InitEvents();
static pascal void GameLoop(EventLoopTimerRef inTimer, void *);

static EventLoopTimerRef GameLoopTimerRef;
static EventHandlerRef MouseEventHandlerRef(0);
static EventHandlerRef KeyEventHandlerRef(0);
static EventHandlerRef AppEventHandlerRef(0);
static EventHandlerRef MainWindowEventHandlerRef(0);
static EventHandlerRef OtherWindowEventHandlerRef(0);
static EventHandlerRef AppleEventHandlerRef(0);

static pascal OSStatus AppEventHandlerProc(EventHandlerCallRef callRef, EventRef inEvent, void *);
static pascal OSStatus MouseEventHandlerProc(EventHandlerCallRef callRef, EventRef inEvent, void *);
static pascal OSStatus KeyEventHandlerProc(EventHandlerCallRef callRef, EventRef inEvent, void *);
static pascal OSStatus WindowEventHandlerProc(EventHandlerCallRef callRef, EventRef inEvent, void *);
static pascal OSStatus AppleEventHandlerProc(EventHandlerCallRef callRef, EventRef inEvent, void *);

static pascal OSErr OpenEventHandlerProc(const AppleEvent *event, AppleEvent *, long);

#endif




static const int WindowWidth  = Compatible::GetDisplayWidth();
static const int WindowHeight = Compatible::GetDisplayHeight();

GameStateManager state_manager(WindowWidth, WindowHeight);

const static wstring application_name = L"Synthesia";
const static std::wstring friendly_app_name = WSTRING(L"Synthesia " << SynthesiaVersionString);

const static wstring error_header1 = L"Synthesia detected a";
const static wstring error_header2 = L" problem and must close:\n\n";
const static wstring error_footer = L"\n\nIf you don't think this should have happened, please\ncontact Nicholas (nicholas@synthesiagame.com) and\ndescribe what you were doing when the problem\noccurred.  Thanks.";

class EdgeTracker
{
public:
   EdgeTracker() : active(true), just_active(true) { }

   void Activate() { just_active = true; active = true; }
   void Deactivate() { just_active = false; active = false; }
   
   bool IsActive() { return active; }
   bool JustActivated()
   {
      bool was_active = just_active;
      just_active = false;
      return was_active;
   }

private:
   bool active;
   bool just_active;
};
static EdgeTracker window_state;


#ifdef WIN32
// Windows
int WINAPI WinMain (HINSTANCE instance, HINSTANCE, PSTR, int iCmdShow)
#else
// Mac
int main(int argc, char *argv[])
#endif
{

#ifdef WIN32
   WNDCLASS wndclass;
   wndclass.style         = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc   = WndProc;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = 0;
   wndclass.hInstance     = instance;
   wndclass.hIcon         = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN_ICON));
   wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
   wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
   wndclass.lpszMenuName  = NULL;
   wndclass.lpszClassName = application_name.c_str();

   if (!RegisterClass (&wndclass))
   {
      MessageBox(NULL, L"There was a problem registering the Window class!", application_name.c_str(), MB_ICONERROR);
      return 0;
   }

#endif

   try
   {
      wstring command_line;

      UserSetting::Initialize(application_name);

#ifdef WIN32
      // CommandLineToArgvW is only available in Windows XP or later.  So,
      // rather than maintain separate binaries for Win2K, I do a runtime
      // library load and check to see if the function I need is available.
      HINSTANCE shell32 = LoadLibrary(L"shell32");
      if (shell32)
      {
         typedef LPWSTR* (WINAPI *COMMANDLINETOARGVW_SIGNATURE)(LPCWSTR, int*);

         COMMANDLINETOARGVW_SIGNATURE function = 0;
         function = (COMMANDLINETOARGVW_SIGNATURE) GetProcAddress(shell32, "CommandLineToArgvW");

         if (function)
         {
            // Grab the file argument from the command line if there is one.
            wstring raw_command_line = GetCommandLine();

            int argument_count;
            LPWSTR *arguments = (function)(raw_command_line.c_str(), &argument_count);

            if (argument_count > 1)
            {
               command_line = arguments[1];
            }
         }

         FreeLibrary(shell32);
      }
#else

      OSStatus status;

      // Apparently the command-line isn't useful in Mac applications.  There
      // is just a weird system command-line argument.  Ignore.
      command_line = L"";
      
      // The actual way to accept dragged-in files is by registering an Apple Event.
      static const EventTypeSpec appleEvents[] = { { kEventClassAppleEvent, kEventAppleEvent } };
      status = InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(AppleEventHandlerProc), GetEventTypeCount(appleEvents), appleEvents, 0, &AppleEventHandlerRef);
      if (status != noErr) throw SynthesiaError(WSTRING(L"Unable to install Apple Event handler.  Error code: " << static_cast<int>(status)));

      OSErr err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, OpenEventHandlerProc, 0, false);
      if (err != noErr) throw SynthesiaError(WSTRING(L"Unable to install open-document event handler.  Error code: " << static_cast<int>(err)));

      // Now we run the application event loop a little early for a short duration
      // so we can intercept any dragged-in-file events before we proceed.
      EventRef event;
      while ((status = ReceiveNextEvent(0, 0, std::numeric_limits<double>::epsilon(), true, &event)) != eventLoopTimedOutErr)
      {
         if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't receive early event.  Error code: " << static_cast<int>(status)));

         SendEventToEventTarget(event, GetApplicationEventTarget());
         ReleaseEvent(event);
      }

      // Check to see if during that event processing we read a filename.
      if (external_command_line.length() > 0) command_line = external_command_line;
      
#endif

      // Strip any leading or trailing quotes from the filename
      // argument (to match the format returned by the open-file
      // dialog later).
      if (command_line.length() > 0 && command_line[0] == L'\"') command_line = command_line.substr(1, command_line.length() - 1);
      if (command_line.length() > 0 && command_line[command_line.length()-1] == L'\"') command_line = command_line.substr(0, command_line.length() - 1);

      Midi *midi = 0;

      // Attempt to open the midi file given on the command line first
      if (command_line != L"")
      {
         try
         {
            midi = new Midi(Midi::ReadFromFile(command_line));
         }
         catch (const MidiError &e)
         {
            wstring wrapped_description = WSTRING(L"Problem while loading file: " << command_line << L"\n") + e.GetErrorDescription();
            Compatible::ShowError(wrapped_description);

            command_line = L"";
            midi = 0;
         }
      }

      // If midi couldn't be opened from command line filename or there
      // simply was no command line filename, use a "file open" dialog.
      if (command_line == L"")
      {
         while (!midi)
         {
            std::wstring file_title;
            FileSelector::RequestMidiFilename(&command_line, &file_title);

            if (command_line != L"")
            {
               try
               {
                  midi = new Midi(Midi::ReadFromFile(command_line));
               }
               catch (const MidiError &e)
               {
                  wstring wrapped_description = WSTRING(L"Problem while loading file: " << file_title << L"\n") + e.GetErrorDescription();
                  Compatible::ShowError(wrapped_description);

                  midi = 0;
               }
            }
            else
            {
               // They pressed cancel, so they must not want to run
               // the app anymore.
               return 0;
            }
         }
      }

      // Save this filename for next time so we can
      // seek the "Open" dialog to the right folder.
      FileSelector::SetLastMidiFilename(command_line);

      // This does what is necessary in construction and
      // resets what it does during its destruction.  We
      // never actually have to reference it.
      ReasonableSynthVolume volume_correction;

#ifdef WIN32

      HWND hwnd = CreateWindow(application_name.c_str(), friendly_app_name.c_str(),
         WS_POPUP, 0, 0, WindowWidth, WindowHeight, HWND_DESKTOP, 0, instance, 0);

      HDC dc_win = GetDC(hwnd);
      if (!dc_win) throw SynthesiaError(L"Couldn't get window device context.");
      
      // Grab the current pixel format and change a few fields
      int pixel_format_id = GetPixelFormat(dc_win);
      PIXELFORMATDESCRIPTOR pfd;
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      DescribePixelFormat(dc_win, pixel_format_id, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.iLayerType = PFD_MAIN_PLANE;

      // After our changes, get the closest match the device has to offer
      pixel_format_id = ChoosePixelFormat(dc_win, &pfd);
      if (!pixel_format_id) throw SynthesiaError(L"Unable to find a good pixel format.");
      if (!SetPixelFormat(dc_win, pixel_format_id, &pfd)) throw SynthesiaError(L"Couldn't set (window) pixel format.");

      HGLRC glrc = wglCreateContext(dc_win);
      if (!glrc) throw SynthesiaError(L"Couldn't create OpenGL rendering context.");
      if (!wglMakeCurrent(dc_win, glrc)) throw SynthesiaError(L"Couldn't make OpenGL rendering context current.");
#else

      Rect windowRect;
      windowRect.top = 0;
      windowRect.left = 0;
      windowRect.right = (short)Compatible::GetDisplayWidth();
      windowRect.bottom = (short)Compatible::GetDisplayHeight();

      status = CreateNewWindow(kPlainWindowClass, kWindowStandardHandlerAttribute, &windowRect, &window);
      if (status != noErr) throw SynthesiaError(WSTRING(L"Unable to create window.  Error code: " << static_cast<int>(status)));

      SetWindowTitleWithCFString(window, MacStringFromWide(friendly_app_name).get());

      RGBColor windowColor;
      windowColor.red   = 65535 * 0.25;
      windowColor.green = 65535 * 0.25;
      windowColor.blue  = 65535 * 0.25;
      SetWindowContentColor(window, &windowColor);

      InitEvents();

      // MACTODO: The fade effect is way cooler
      status = TransitionWindow(window, kWindowZoomTransitionEffect, kWindowShowTransitionAction, 0);
      if (status != noErr) throw SynthesiaError(WSTRING(L"Unable to transition the window.  Error code: " << static_cast<int>(status)));

      SetPortWindowPort(window);

      GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_NONE };
      AGLPixelFormat aglPixelFormat = aglChoosePixelFormat(NULL, 0, attrib);
      if (!aglPixelFormat) throw SynthesiaError(L"Couldn't set AGL pixel format.");

      aglContext = aglCreateContext(aglPixelFormat, (aglGetCurrentContext() != 0) ? aglGetCurrentContext() : 0);
      aglSetDrawable(aglContext, GetWindowPort(window));
      if (!aglSetCurrentContext(aglContext)) throw SynthesiaError(L"Error in SetupAppleGLContext(): Could not set current AGL context.");

      aglSetCurrentContext(aglContext);
      aglUpdateContext(aglContext);

#endif

      // All of this OpenGL stuff only needs to be set once
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClearDepth(1.0f);
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);

      glShadeModel(GL_SMOOTH);

      glViewport(0, 0, WindowWidth, WindowHeight);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0, WindowWidth, 0, WindowHeight);

      SharedState state;
      state.song_title = FileSelector::TrimFilename(command_line);
      state.midi = midi;

      state_manager.SetInitialState(new TitleState(state));

      // LOGTODO: glGetString(): GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS

#ifdef WIN32
      ShowWindow(hwnd, iCmdShow);
      UpdateWindow(hwnd);

      MSG msg;
      ZeroMemory(&msg, sizeof(MSG));

      bool running = true;
      while (running)
      {
         if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
         {
            if (!GetMessage(&msg, 0, 0, 0)) running = false;
            DispatchMessage(&msg);
         }
         else
         {
            if (window_state.IsActive())
            {
               state_manager.Update(window_state.JustActivated());

               Renderer renderer(dc_win);
               renderer.SetVSyncInterval(1);
               state_manager.Draw(renderer);
            }
         }
      }

      wglMakeCurrent(dc_win, 0);
      wglDeleteContext(glrc);
      ReleaseDC(hwnd, dc_win);
      DestroyWindow(hwnd);

      UnregisterClass(application_name.c_str(), instance);

      return int(msg.wParam);
      
#else

      RunApplicationEventLoop();
      DisposeWindow(window);
      
      aglDestroyPixelFormat(aglPixelFormat);
      aglSetCurrentContext(0);
      aglSetDrawable(aglContext, 0);
      aglDestroyContext(aglContext);
      
      return 0;
#endif
   }
   catch (const SynthesiaError &e)
   {
      wstring wrapped_description = WSTRING(error_header1 << error_header2 << e.GetErrorDescription() << error_footer);
      Compatible::ShowError(wrapped_description);
   }
   catch (const MidiError &e)
   {
      wstring wrapped_description = WSTRING(error_header1 << L" MIDI" << error_header2 << e.GetErrorDescription() << error_footer);
      Compatible::ShowError(wrapped_description);
   }
   catch (const std::exception &e)
   {
      wstring wrapped_description = WSTRING(L"Synthesia detected an unknown problem and must close!  '" << e.what() << "'" << error_footer);
      Compatible::ShowError(wrapped_description);
   }
   catch (...)
   {
      wstring wrapped_description = WSTRING(L"Synthesia detected an unknown problem and must close!" << error_footer);
      Compatible::ShowError(wrapped_description);
   }

   return 1;
}


#ifdef WIN32

// Windows message callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_CREATE:
      {
         return 0;
      }

   case WM_DESTROY:
      {
         PostQuitMessage(0);
         return 0;
      }

   case WM_ACTIVATE:
      {
         if (LOWORD(wParam) != WA_INACTIVE) window_state.Activate();
         else window_state.Deactivate();
         
         return 0;
      }

   case WM_SYSCOMMAND:
      {
         // Prevent the screensaver or monitor power-save from kicking in.
         switch (wParam)
         {
         case SC_SCREENSAVE:
         case SC_MONITORPOWER:
            return 0;
         }
         break;
      }

   case WM_KEYDOWN:
      {
         switch (wParam)
         {
         case VK_UP:       state_manager.KeyPress(KeyUp);      break;
         case VK_DOWN:     state_manager.KeyPress(KeyDown);    break;
         case VK_LEFT:     state_manager.KeyPress(KeyLeft);    break;
         case VK_RIGHT:    state_manager.KeyPress(KeyRight);   break;
         case VK_SPACE:    state_manager.KeyPress(KeySpace);   break;
         case VK_RETURN:   state_manager.KeyPress(KeyEnter);   break;
         case VK_ESCAPE:   state_manager.KeyPress(KeyEscape);  break;

         case VK_F6:       state_manager.KeyPress(KeyF6);      break;

         case VK_OEM_PLUS: state_manager.KeyPress(KeyPlus);    break;
         case VK_OEM_MINUS:state_manager.KeyPress(KeyMinus);   break;
         }

         return 0;
      }

   case WM_CHAR:
      {
         return 0;
      }

   case WM_MOUSEMOVE:
      {
         POINT pt = { LOWORD(lParam), HIWORD(lParam) };
         state_manager.MouseMove(pt.x, pt.y);

         return 0;
      }

   case WM_LBUTTONUP:   state_manager.MouseRelease(MouseLeft);  return 0;
   case WM_LBUTTONDOWN: state_manager.MousePress(MouseLeft);    return 0;
   case WM_RBUTTONUP:   state_manager.MouseRelease(MouseRight); return 0;
   case WM_RBUTTONDOWN: state_manager.MousePress(MouseRight);   return 0;

   }
   return DefWindowProc (hwnd, message, wParam, lParam);
}

#else


void InitEvents()
{
   // Update as fast as possible
   InstallEventLoopTimer( GetCurrentEventLoop(), 0, kEventDurationSecond / 10000.0, NewEventLoopTimerUPP(GameLoop), 0, &GameLoopTimerRef);
   
   OSStatus ret;
   
   static const EventTypeSpec appControlEvents[] =
   {
   { kEventClassApplication, kEventAppLaunchNotification },
   { kEventClassApplication, kEventAppActivated },
   { kEventClassApplication, kEventAppDeactivated },
   { kEventClassApplication, kEventAppHidden },
   { kEventClassApplication, kEventAppShown },
   { kEventClassApplication, kEventAppTerminated },
   { kEventClassApplication, kEventAppQuit }
   };
   
   ret = InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(AppEventHandlerProc), GetEventTypeCount(appControlEvents), appControlEvents, 0, &AppEventHandlerRef);
   if (ret != noErr) throw SynthesiaError(WSTRING(L"Unable to install app event handler.  Error code: " << static_cast<int>(ret)));
   
   static const EventTypeSpec mouseControlEvents[] =
   {
   { kEventClassMouse, kEventMouseDown },
   { kEventClassMouse, kEventMouseUp },
   { kEventClassMouse, kEventMouseMoved }
   };
   
   ret = InstallEventHandler( GetApplicationEventTarget(), NewEventHandlerUPP( MouseEventHandlerProc ), GetEventTypeCount(mouseControlEvents), mouseControlEvents, 0, &MouseEventHandlerRef );
   if (ret != noErr) throw SynthesiaError(WSTRING(L"Unable to install mouse event handler.  Error code: " << static_cast<int>(ret)));
   
   static const EventTypeSpec keyControlEvents[] =
   {
   { kEventClassKeyboard, kEventRawKeyDown },
   { kEventClassKeyboard, kEventRawKeyRepeat },
   { kEventClassKeyboard, kEventRawKeyUp }
   };
   
   ret = InstallEventHandler( GetApplicationEventTarget(), NewEventHandlerUPP( KeyEventHandlerProc ), GetEventTypeCount(keyControlEvents), keyControlEvents, 0, &KeyEventHandlerRef );
   if (ret != noErr) throw SynthesiaError(WSTRING(L"Unable to install key event handler.  Error code: " << static_cast<int>(ret)));
      
   static const EventTypeSpec windowControlEvents[] = 
   {
   { kEventClassWindow, kEventWindowUpdate },
   { kEventClassWindow, kEventWindowDrawContent },
   { kEventClassWindow, kEventWindowActivated },
   { kEventClassWindow, kEventWindowDeactivated },
   { kEventClassWindow, kEventWindowGetClickActivation },
   { kEventClassWindow, kEventWindowBoundsChanging },
   { kEventClassWindow, kEventWindowBoundsChanged },
   { kEventClassWindow, kEventWindowShown },
   { kEventClassWindow, kEventWindowShowing },
   { kEventClassWindow, kEventWindowHidden },
   { kEventClassWindow, kEventWindowHiding },
   { kEventClassWindow, kEventWindowCursorChange },
   { kEventClassWindow, kEventWindowClosed }
   };
   
   if (OtherWindowEventHandlerRef != 0) RemoveEventHandler(OtherWindowEventHandlerRef);
   
   ret = InstallEventHandler(GetWindowEventTarget(window), NewEventHandlerUPP(WindowEventHandlerProc), GetEventTypeCount(windowControlEvents), windowControlEvents, 0, &OtherWindowEventHandlerRef );
   if (ret != noErr) throw SynthesiaError(WSTRING(L"Unable to install window event handler.  Error code: " << static_cast<int>(ret)));
}



static pascal void GameLoop(EventLoopTimerRef inTimer, void *)
{
   if (!window_state.IsActive()) return;

   try
   {
      state_manager.Update(window_state.JustActivated());

      Renderer renderer(aglContext);
      renderer.SetVSyncInterval(1);
      
      state_manager.Draw(renderer);
   }
   catch (const SynthesiaError &e)
   {
      wstring wrapped_description = WSTRING(error_header1 << error_header2 << e.GetErrorDescription() << error_footer);
      Compatible::ShowError(wrapped_description);
      Compatible::GracefulShutdown();
   }
   catch (const MidiError &e)
   {
      wstring wrapped_description = WSTRING(error_header1 << L" MIDI" << error_header2 << e.GetErrorDescription() << error_footer);
      Compatible::ShowError(wrapped_description);
      Compatible::GracefulShutdown();
   }
   catch (const std::exception &e)
   {
      wstring wrapped_description = WSTRING(L"Synthesia detected an unknown problem and must close!  '" << e.what() << "'" << error_footer);
      Compatible::ShowError(wrapped_description);
      Compatible::GracefulShutdown();
   }
   catch (...)
   {
      wstring wrapped_description = WSTRING(L"Synthesia detected an unknown problem and must close!" << error_footer);
      Compatible::ShowError(wrapped_description);
      Compatible::GracefulShutdown();
   }

}

static pascal OSErr OpenEventHandlerProc(const AppleEvent *event, AppleEvent *, long)
{
   AEDescList docs;
   OSStatus status = AEGetParamDesc(event, keyDirectObject, typeAEList, &docs);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get Apple Event parameter description.  Error code: " << static_cast<int>(status)));
   
   // We can only handle the first dragged-in file, so
   // all that matters is that the list isn't empty.
   long item_count = 0;
   AECountItems(&docs, &item_count);
   if (item_count == 0) return noErr;
   
   FSRef ref;
   status = AEGetNthPtr(&docs, 1, typeFSRef, 0, 0, &ref, sizeof(ref), 0);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't look up Apple Event pointer.  Error code: " << static_cast<int>(status)));
   
   const static int BufferSize(1024);
   char path_buffer[BufferSize];
   status = FSRefMakePath(&ref, (UInt8*)path_buffer, BufferSize);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get file path.  Error code: " << static_cast<int>(status)));

   std::string narrow_path(path_buffer);
   std::wstring path(narrow_path.begin(), narrow_path.end());

   external_command_line = path;
   
   return noErr;
}


OSStatus AppleEventHandlerProc(EventHandlerCallRef callRef, EventRef inEvent, void*)
{
    // Events of type kEventAppleEvent must be removed from the queue
    //  before being passed to AEProcessAppleEvent.
    bool release = false;
    if (IsEventInQueue(GetMainEventQueue(), inEvent))
    {
        // RemoveEventFromQueue will release the event, which will
        //  destroy it if we don't retain it first.
        RetainEvent(inEvent);
        release = true;
        RemoveEventFromQueue(GetMainEventQueue(), inEvent);
    }
 
    // Convert the event ref to the type AEProcessAppleEvent expects.
    EventRecord eventRecord;
    ConvertEventRefToEventRecord(inEvent, &eventRecord);
    AEProcessAppleEvent(&eventRecord);
 
    if (release) ReleaseEvent(inEvent);
 
    return noErr;
}

static pascal OSStatus AppEventHandlerProc(EventHandlerCallRef callRef, EventRef event, void *)
{
   UInt32 eventKind = GetEventKind(event);
   switch(eventKind)
   {
      case kEventAppLaunchNotification:
         break;
         
      case kEventAppShown:
      case kEventAppActivated:
         window_state.Activate();
         break;
         
      case kEventAppHidden:
      case kEventAppDeactivated:
         window_state.Deactivate();
         break;
         
      case kEventAppQuit:
      
         RemoveEventLoopTimer(GameLoopTimerRef);
         
         RemoveEventHandler(MouseEventHandlerRef);
         RemoveEventHandler(KeyEventHandlerRef);
         RemoveEventHandler(AppleEventHandlerRef);
         RemoveEventHandler(AppEventHandlerRef);
         RemoveEventHandler(MainWindowEventHandlerRef);

         AERemoveEventHandler(kCoreEventClass,  kAEOpenDocuments, OpenEventHandlerProc, false);

         break;
         
      case kEventAppTerminated:
         break;
   };
   
   return eventNotHandledErr;
}



static pascal OSStatus WindowEventHandlerProc(EventHandlerCallRef callRef, EventRef event, void *)
{
   WindowRef window;
   GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);
   
   switch(GetEventKind(event))
   {
      case kEventWindowClosed:
         QuitApplicationEventLoop();
         break;
   };
   
   return eventNotHandledErr;
}


static pascal OSStatus MouseEventHandlerProc(EventHandlerCallRef callRef, EventRef event, void *)
{
   switch (GetEventKind(event))
   {
      case kEventMouseDown:
      {
         EventMouseButton button;
         GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button );
         
         switch (button)
         {
         case kEventMouseButtonPrimary: state_manager.MousePress(MouseLeft); break;
         case kEventMouseButtonSecondary: state_manager.MousePress(MouseRight); break;
         }
         
         break;
      }
         
      case kEventMouseUp:
      {
         EventMouseButton button;
         GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button );
         
         switch (button)
         {
         case kEventMouseButtonPrimary: state_manager.MouseRelease(MouseLeft); break;
         case kEventMouseButtonSecondary: state_manager.MouseRelease(MouseRight); break;
         }
         
         break;
      }
         
      case kEventMouseMoved:
      {
         HIPoint loc;
         GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &loc);
         
         state_manager.MouseMove((int)loc.x, (int)loc.y);
            
         break;
      }
   };
   
   return eventNotHandledErr;
}

static pascal OSStatus KeyEventHandlerProc(EventHandlerCallRef callRef, EventRef event, void *inUserData )
{
   bool is_down = false;   
   switch(GetEventKind(event))
   {
      case kEventRawKeyDown:
      case kEventRawKeyRepeat:
         is_down = true;
         break;

      case kEventRawKeyUp:
         break;
   };
   
   if (is_down)
   {
      UInt32 keyCode;
      GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode);
   
      // Worst thing ever: I couldn't find a list of these
      // keycodes, so they're determined experimentally.
      switch (keyCode)
      {
      case 126: state_manager.KeyPress(KeyUp);     break;
      case 125: state_manager.KeyPress(KeyDown);   break;
      case 123: state_manager.KeyPress(KeyLeft);   break;
      case 124: state_manager.KeyPress(KeyRight);  break;
      case 49:  state_manager.KeyPress(KeySpace);  break;
      case 36:  state_manager.KeyPress(KeyEnter);  break;
      case 53:  state_manager.KeyPress(KeyEscape); break;

      case 97:  state_manager.KeyPress(KeyF6);     break;

      case 24:  state_manager.KeyPress(KeyPlus);   break;
      case 27:  state_manager.KeyPress(KeyMinus);  break;
      }
   }
   
   return eventNotHandledErr;
}





#endif

