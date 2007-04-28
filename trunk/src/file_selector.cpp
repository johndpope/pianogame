#include "file_selector.h"
#include "UserSettings.h"
#include "string_util.h"

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <strsafe.h>
#endif

#include <set>

using namespace std;

#ifdef WIN32
extern HWND g_hwnd;
#endif

namespace FileSelector
{

void RequestMidiFilename(std::wstring *returned_filename, std::wstring *returned_file_title)
{
   // Grab the filename of the last song we played
   // and pre-load it into the open dialog
   wstring last_filename = UserSetting::Get(L"Last File", L"");

   const static int BufferSize = 512;
   wchar_t filename[BufferSize] = L"";
   wchar_t filetitle[BufferSize] = L"";

#ifdef WIN32
   // Try to populate our "File Open" box with the last file selected
   if (StringCbCopyW(filename, BufferSize, last_filename.c_str()) == STRSAFE_E_INSUFFICIENT_BUFFER)
   {
      // If there wasn't a last file, default to the built-in Music directory
      filename[0] = L'\0';
   }

   wstring default_dir;
   bool default_directory = false;
   if (last_filename.length() == 0)
   {
      default_directory = true;
      default_dir = UserSetting::Get(L"Default Music Directory", L"");

      if (!SetCurrentDirectory(default_dir.c_str()))
      {
         // TODO: Log something some day, but take no other action.
         //
         // This is alright.
      }
   }

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize =     sizeof(OPENFILENAME);
   ofn.hwndOwner =       g_hwnd;
   ofn.lpstrTitle =      L"Synthesia: Choose a song to play";
   ofn.lpstrFilter =     L"MIDI Files (*.mid)\0*.mid;*.midi\0All Files (*.*)\0*.*\0";
   ofn.lpstrFile =       filename;
   ofn.nMaxFile =        BufferSize;
   ofn.lpstrFileTitle =  filetitle;
   ofn.lpstrInitialDir = default_dir.c_str();
   ofn.nMaxFileTitle =   BufferSize;
   ofn.lpstrDefExt =     L"mid";
   ofn.Flags =           OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

   if (GetOpenFileName(&ofn))
   {
      std::wstring filename = WSTRING(ofn.lpstrFile);

      SetLastMidiFilename(filename);

      if (returned_file_title) *returned_file_title = WSTRING(filetitle);
      if (returned_filename) *returned_filename = filename;
      return;
   }

   if (returned_file_title) *returned_file_title = L"";
   if (returned_filename) *returned_filename = L"";
#else
   // MACTODO: OpenPanel
#endif
}

void SetLastMidiFilename(const std::wstring &filename)
{
   UserSetting::Set(L"Last File", filename);
}

std::wstring TrimFilename(const std::wstring &filename)
{
   wstring song_title = filename;
   wstring song_lower = StringLower(song_title);

   // Strip off known file extensions
   set<wstring> extensions;
   extensions.insert(L".mid");
   extensions.insert(L".midi");
   for (set<wstring>::const_iterator i = extensions.begin(); i != extensions.end(); ++i)
   {
      wstring extension = StringLower(*i);
      wstring::size_type len = extension.length();

      wstring song_end = song_lower.substr(std::max((unsigned long)0, (unsigned long)(song_lower.length() - len)), song_lower.length());
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

   return song_title;
}

}; // End namespace