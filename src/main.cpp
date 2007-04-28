// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#ifdef WIN32
#include <gl/gl.h>
#include <gl/glu.h>
#else
#include <OpenGL/OpenGL.h>
#include <AGL/gl.h>
#include <AGL/glu.h>
#endif


#include <set>
#include <string>
#include "string_util.h"
#include "file_selector.h"
#include "UserSettings.h"
#include "version.h"
#include "resource.h"

#include "CompatibleSystem.h"
#include "SynthesiaError.h"
#include "KeyboardDisplay.h"
#include "libmidi/Midi.h"

#ifdef WIN32
// The problem this bit of code solves is only in Windows
#include "libmidi/SynthVolume.h"
#endif

#include "Tga.h"
#include "Renderer.h"
#include "SharedState.h"
#include "GameState.h"
#include "State_Title.h"

using namespace std;

#ifdef WIN32

static const int WindowWidth  = GetSystemMetrics(SM_CXSCREEN);
static const int WindowHeight = GetSystemMetrics(SM_CYSCREEN);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// TODO: Make this better
HWND g_hwnd;

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

#else

// MACTODO: Window/screen dimensions
static const int WindowWidth = 1024;
static const int WindowHeight = 768;

#endif

GameStateManager state_manager(WindowWidth, WindowHeight);

static bool WindowActive = true;


void setVSync(int interval=1)
{
#ifdef WIN32
  const char *extensions = reinterpret_cast<const char*>(static_cast<const unsigned char*>(glGetString( GL_EXTENSIONS )));

  if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 )
    return; // Error: WGL_EXT_swap_control extension not supported on your computer.\n");
  else
  {
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

    if( wglSwapIntervalEXT )
      wglSwapIntervalEXT(interval);
  }
#else
   // MACNOTE: Don't do anything for now.  Investigate further some other time.
#endif
}

const static wstring application_name = L"Synthesia";
const static std::wstring friendly_app_name = WSTRING(L"Synthesia " << SynthesiaVersionString);

const static wstring error_header1 = L"Synthesia detected a";
const static wstring error_header2 = L" problem and must close:\n\n";
const static wstring error_footer = L"\n\nIf you don't think this should have happened, please\ncontact Nicholas (nicholas@halitestudios.com) and\ndescribe what you were doing when the problem\noccurred.  Thanks.";

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
   
#else

   // MACTODO: Set up the window
   
#endif


#ifndef _DEBUG
   try
#endif
   {
      wstring command_line;

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
      // MACTODO: concat the command line?
#endif

      // Strip any leading or trailing quotes from the filename
      // argument (to match the format returned by the open-file
      // dialog later).
      if (command_line.length() > 0 && command_line[0] == L'\"') command_line = command_line.substr(1, command_line.length() - 1);
      if (command_line.length() > 0 && command_line[command_line.length()-1] == L'\"') command_line = command_line.substr(0, command_line.length() - 1);

      UserSetting::Initialize(application_name);

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

#ifdef WIN32
      // This does what is necessary in construction and
      // resets what it does during its destruction
      ReasonableSynthVolume volume_correct;
#endif

#ifdef WIN32
      HWND hwnd = CreateWindow(application_name.c_str(), friendly_app_name.c_str(),
         WS_POPUP, 0, 0, WindowWidth, WindowHeight, HWND_DESKTOP, 0, instance, 0);
      g_hwnd = hwnd;

      HDC dc = GetDC(hwnd);
      if (!dc) throw std::exception("Couldn't get window device context.");
      
      // Grab the current pixel format and change a few fields
      int pixel_format_id = GetPixelFormat(dc);
      PIXELFORMATDESCRIPTOR pfd;
      DescribePixelFormat(dc, pixel_format_id, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.iLayerType = PFD_MAIN_PLANE;

      // After our changes, get the closest match the device has to offer
      pixel_format_id = ChoosePixelFormat(dc, &pfd);
      if (!pixel_format_id) throw std::exception("Unable to find a good pixel format.");
      if (!SetPixelFormat(dc, pixel_format_id, &pfd)) throw std::exception("Couldn't set pixel format.");

      HGLRC glrc = wglCreateContext(dc);
      if (!glrc) throw std::exception("Couldn't create OpenGL rendering context.");
      if (!wglMakeCurrent(dc, glrc)) throw std::exception("Couldn't make OpenGL rendering context current.");
#else
      // MACTODO: Set up the window and initialize OpenGL
#endif

      // Enable v-sync for release versions
      setVSync(1);

      // All of this OpenGL stuff only needs to be set once
      glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
      glClearDepth(1.0f);
      glDepthFunc(GL_LEQUAL);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);

      glShadeModel(GL_SMOOTH);

      glViewport(0, 0, WindowWidth, WindowHeight);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0, WindowWidth, 0, WindowHeight);

      int return_value = 1;

#ifdef WIN32
      ShowWindow (hwnd, iCmdShow);
      UpdateWindow (hwnd);

      SharedState state;
      state.song_title = FileSelector::TrimFilename(command_line);
      state.midi = midi;

      state_manager.SetInitialState(new TitleState(state));

      MSG msg;
      ZeroMemory(&msg, sizeof(MSG));

      bool was_inactive = true;
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
            if (WindowActive)
            {
               state_manager.Update(was_inactive);
               was_inactive = false;

               Renderer renderer(dc);
               state_manager.Draw(renderer);
            }
            else
            {
               was_inactive = true;
            }
         }
      }

      wglMakeCurrent(dc, 0);
      wglDeleteContext(glrc);
      ReleaseDC(hwnd, dc);
      DestroyWindow(hwnd);

      UnregisterClass(application_name.c_str(), instance);

      return_value = int(msg.wParam);
#else
      // MACTODO: Program loop
#endif

      return return_value;
   }
#ifndef _DEBUG
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
      wstring wrapped_description = WSTRING(error_header1 << error_header2 << e.what() << error_footer);
      Compatible::ShowError(wrapped_description);
   }
   catch (...)
   {
      wstring wrapped_description = WSTRING(L"Synthesia detected an unknown problem and must close!" << error_footer);
      Compatible::ShowError(wrapped_description);
   }

   return 1;

#endif
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
         WindowActive = (LOWORD(wParam) != WA_INACTIVE);
         return 0;
      }

   case WM_SYSCOMMAND:
      {
         // TODO: I'm not convinced this (NeHe code) is the appropriate behavior!

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

// MACTODO: Keyboard/Mouse events

#endif

