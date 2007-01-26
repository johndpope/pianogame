// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include <windows.h>
#include "registry.h"

#include <set>
#include <string>
#include <strsafe.h>
#include "string_util.h"

#include "PianoHeroError.h"
#include "KeyboardDisplay.h"
#include "libmidi/Midi.h"
#include "libmidi/MidiComm.h"

#include "SharedState.h"
#include "GameState.h"
#include "State_Title.h"

#include "resource.h"
#include "version.h"

using namespace std;

static const int WindowWidth  = GetSystemMetrics(SM_CXSCREEN);
static const int WindowHeight = GetSystemMetrics(SM_CYSCREEN);

GameStateManager state_manager(WindowWidth, WindowHeight);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain (HINSTANCE instance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
   const static wstring application_name = L"PianoHero";
   const static wstring friendly_app_name = WSTRING(L"Piano Hero " << PianoHeroVersionString);

#ifndef _DEBUG
   try
#endif
   {
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
      
      
      wstring command_line;

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

      // Strip any leading or trailing quotes from the filename
      // argument (to match the format returned by the open-file
      // dialog later).
      if (command_line.length() > 0 && command_line[0] == L'\"') command_line = command_line.substr(1, command_line.length() - 1);
      if (command_line.length() > 0 && command_line[command_line.length()-1] == L'\"') command_line = command_line.substr(0, command_line.length() - 1);

      HWND hwnd = CreateWindow(application_name.c_str(), friendly_app_name.c_str(),
         WS_POPUP, 0, 0, WindowWidth, WindowHeight, 0, 0, instance, 0);

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
            MessageBox(0, wrapped_description.c_str(), (friendly_app_name + WSTRING(L" Error")).c_str(), MB_ICONERROR);

            command_line = L"";
            midi = 0;
         }
      }

      // If midi couldn't be opened from command line filename or there
      // simply was no command line filename, use a "file open" dialog.
      if (command_line == L"")
      {
         // Grab the filename of the last song we played from the
         // registry and pre-load that filename in the open dialog
         const static int BufferSize = 512;
         wchar_t filename[BufferSize] = L"";
         wchar_t filetitle[BufferSize] = L"";

         wstring last_filename;
         Registry reg(Registry::CurrentUser, L"Piano Hero");
         reg.Read(L"Last File", &last_filename, L"");

         if (StringCbCopyW(filename, BufferSize, last_filename.c_str()) == STRSAFE_E_INSUFFICIENT_BUFFER)
         {
            filename[0] = L'\0';
         }

         OPENFILENAME ofn;
         ZeroMemory(&ofn, sizeof(OPENFILENAME));
         ofn.lStructSize =    sizeof(OPENFILENAME);
         ofn.hwndOwner =      hwnd;
         ofn.lpstrTitle =     L"Piano Hero: Choose a song to play";
         ofn.lpstrFilter =    L"MIDI Files (*.mid)\0*.mid;*.midi\0All Files (*.*)\0*.*\0";
         ofn.lpstrFile =      filename;
         ofn.nMaxFile =       BufferSize;
         ofn.lpstrFileTitle = filetitle;
         ofn.nMaxFileTitle =  BufferSize;
         ofn.lpstrDefExt =    L"mid";
         ofn.Flags =          OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

         while (!midi)
         {
            if (GetOpenFileName(&ofn))
            {
               command_line = WSTRING(ofn.lpstrFile);

               try
               {
                  midi = new Midi(Midi::ReadFromFile(command_line));
               }
               catch (const MidiError &e)
               {
                  wstring wrapped_description = WSTRING(L"Problem while loading file: " << filetitle << L"\n") + e.GetErrorDescription();
                  MessageBox(0, wrapped_description.c_str(), (friendly_app_name + WSTRING(L" Error")).c_str(), MB_ICONERROR);

                  midi = 0;
               }
            }
            else
            {
               exit(0);
            }
         }
      }

      // Save this filename for next time so we can
      // seek the "Open" dialog to the right folder.
      Registry reg(Registry::CurrentUser, L"Piano Hero");
      reg.Write(L"Last File", command_line);

      MidiCommOut::SetReasonableSynthesizerVolume();

      ShowWindow (hwnd, iCmdShow);
      UpdateWindow (hwnd);
   
      Image::SetGlobalModuleInstance(instance);

      // Trim down the filename for display purposes
      wstring song_title = command_line;
      wstring song_lower = StringLower(song_title);

      // Strip off known file extensions
      set<wstring> extensions;
      extensions.insert(L".mid");
      extensions.insert(L".midi");
      for (set<wstring>::const_iterator i = extensions.begin(); i != extensions.end(); ++i)
      {
         wstring extension = StringLower(*i);
         wstring::size_type len = extension.length();

         wstring song_end = song_lower.substr(max(0, song_lower.length() - len), song_lower.length());
         if (song_end == extension) song_title = song_title.substr(0, song_title.length() - len);
         song_lower = StringLower(song_title);
      }

      // Strip off path
      for (wstring::size_type i = song_title.length(); i != 0; --i)
      {
         if (song_title[i-1] == L'\\')
         {
            song_title = song_title.substr(i, song_title.length());
            break;
         }
      }

      SharedState state(song_title);
      state.midi = midi;

      state_manager.SetInitialState(new TitleState(state));

      MSG msg;
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
            // Redraw the window as often as possible
            InvalidateRect(hwnd, 0, false);

            state_manager.Update();
         }
      }

      delete midi;

      MidiCommOut::RestoreSynthesizerVolume();

      return int(msg.wParam);
   }
#ifndef _DEBUG
   catch (const PianoHeroError &e)
   {
      wstring wrapped_description = WSTRING(L"Detected a problem:\n") + e.GetErrorDescription();
      MessageBox(0, wrapped_description.c_str(), (friendly_app_name + WSTRING(L" Error")).c_str(), MB_ICONERROR);

      return 1;
   }
   catch (const MidiError &e)
   {
      wstring wrapped_description = WSTRING(L"Detected a MIDI problem:\n") + e.GetErrorDescription();
      MessageBox(0, wrapped_description.c_str(), (friendly_app_name + WSTRING(L" Error")).c_str(), MB_ICONERROR);

      return 1;
   }
   catch (const ImageError &e)
   {
      wstring wrapped_description = WSTRING(L"Detected an image problem:\n") + e.GetErrorDescription();
      MessageBox(0, wrapped_description.c_str(), (friendly_app_name + WSTRING(L" Error")).c_str(), MB_ICONERROR);

      return 1;
   }
#endif
}

void UpdatePlaying()
{
   // NOTE: This is what midi input looks like...

   /*
   // Read in MIDI
   while (midi_in.KeepReading())
   {
   MidiEvent ev = midi_in.Read();
   if (ev.Type() == MidiEventType_SysEx) continue;

   if (ev.Type() == MidiEventType_NoteOn || ev.Type() == MidiEventType_NoteOff)
   {
   int vel = ev.NoteVelocity();
   wstring name = MidiEvent::NoteName(ev.NoteNumber());
   last_note = WSTRING(name << L", " << vel);

   keyboard.SetKeyActive(name, (vel > 0) );

   midi_out.Write(ev);
   }
   }
   */
}



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

   case WM_PAINT:
      {
         PAINTSTRUCT ps;

         HDC hdc = BeginPaint(hwnd, &ps);
         state_manager.Draw(hdc);
         EndPaint (hwnd, &ps);

         return 0;
      }
   }
   return DefWindowProc (hwnd, message, wParam, lParam);
}

