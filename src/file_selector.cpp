#include "file_selector.h"
#include "registry.h"
#include "string_util.h"

#include <windows.h>
#include <strsafe.h>

#include <set>

using namespace std;

extern HWND g_hwnd;

void RequestMidiFilename(std::wstring *returned_filename, std::wstring *returned_file_title)
{
   // Grab the filename of the last song we played from the
   // registry and pre-load that filename in the open dialog
   wstring last_filename;
   Registry reg(Registry::CurrentUser, L"Piano Hero");
   reg.Read(L"Last File", &last_filename, L"");

   const static int BufferSize = 512;
   wchar_t filename[BufferSize] = L"";
   wchar_t filetitle[BufferSize] = L"";

   if (StringCbCopyW(filename, BufferSize, last_filename.c_str()) == STRSAFE_E_INSUFFICIENT_BUFFER)
   {
      filename[0] = L'\0';
   }

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize =    sizeof(OPENFILENAME);
   ofn.hwndOwner =      g_hwnd;
   ofn.lpstrTitle =     L"Piano Hero: Choose a song to play";
   ofn.lpstrFilter =    L"MIDI Files (*.mid)\0*.mid;*.midi\0All Files (*.*)\0*.*\0";
   ofn.lpstrFile =      filename;
   ofn.nMaxFile =       BufferSize;
   ofn.lpstrFileTitle = filetitle;
   ofn.nMaxFileTitle =  BufferSize;
   ofn.lpstrDefExt =    L"mid";
   ofn.Flags =          OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

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
}

void SetLastMidiFilename(const std::wstring &filename)
{
   Registry reg(Registry::CurrentUser, L"Piano Hero");
   reg.Write(L"Last File", filename);
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

   return song_title;
}
