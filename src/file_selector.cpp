// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "SynthesiaError.h"
#include "file_selector.h"
#include "UserSettings.h"
#include "string_util.h"
#include "os.h"

#include <set>
using namespace std;

#ifdef WIN32
#include <strsafe.h>
const static wchar_t PathDelimiter = L'\\';
#else
#include "ApplicationServices/ApplicationServices.h"
const static wchar_t PathDelimiter = L'/';
#endif

namespace FileSelector
{

#ifndef WIN32

static pascal Boolean NavOpenFilterProc(AEDesc *item, void *info, NavCallBackUserData callBackUD, NavFilterModes filterMode)
{
   OSStatus status;
   Boolean outCanOpenAsMovie;
   Boolean canViewItem = false;

   if (!item->descriptorType == typeFSRef) return false;
   if (((NavFileOrFolderInfo*)info)->isFolder == true) return true;

   FSRef fsRef;
   status = AEGetDescData(item, &fsRef, sizeof(fsRef));
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get item description.  Error code: " << static_cast<int>(status)));

   const static int BufferSize(1024);
   char path_buffer[BufferSize];
   status = FSRefMakePath(&fsRef, (UInt8*)path_buffer, BufferSize);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get file path.  Error code: " << static_cast<int>(status)));

   std::string path(path_buffer);
   if (path.length() < 5) return false;
   
   std::string path_lower(StringLower<std::string>(path));
   
   bool allowed = false;
   const static std::string allowed1(".mid");
   const static std::string allowed2(".midi");
   allowed = allowed || (path_lower.substr(path_lower.length() - allowed1.length()) == allowed1);
   allowed = allowed || (path_lower.substr(path_lower.length() - allowed2.length()) == allowed2);

   return allowed;
}

#endif


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
         // LOGTODO!
         // This is non-critical.  No action required.
      }
   }

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize =     sizeof(OPENFILENAME);
   ofn.hwndOwner =       0;
   ofn.lpstrTitle =      L"Synthesia: Choose a MIDI song to play";
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
   
   OSStatus status;
   
   NavDialogCreationOptions options;
   status  = NavGetDefaultDialogCreationOptions(&options);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't create dialog options.  Error code: " << static_cast<int>(status)));
   
   options.windowTitle = CFSTR("Synthesia: Choose a MIDI song to play");
   
   // TODO: Should clean this up at shut-down
   static NavObjectFilterUPP navFilterUPP(0);
   if (navFilterUPP == 0) navFilterUPP = NewNavObjectFilterUPP(NavOpenFilterProc);
   
   NavDialogRef navDialog(0);
   status = NavCreateChooseFileDialog(&options, 0, 0, 0, navFilterUPP, 0, &navDialog);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't create open dialog.  Error code: " << static_cast<int>(status)));
   
   status = NavDialogRun(navDialog);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't run open dialog.  Error code: " << static_cast<int>(status)));
   
   NavReplyRecord navReply;
   status = NavDialogGetReply(navDialog, &navReply);

   if (status == userCanceledErr || !navReply.validRecord)
   {
      NavDisposeReply(&navReply);

      if (returned_file_title) *returned_file_title = L"";
      if (returned_filename) *returned_filename = L"";
      return;
   }
   
   long item_count = 0;
   status = AECountItems(&navReply.selection, &item_count);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't count resulting items from open dialog.  Error code: " << static_cast<int>(status)));
      
   for (long i = 1; i <= item_count; i++)
   {
      FSRef fsRef;
      if (AEGetNthPtr(&navReply.selection, i, typeFSRef, 0, 0, &fsRef, sizeof(FSRef), 0) == noErr)
      {         
         CFStringRef file_title;
         status = LSCopyDisplayNameForRef( &fsRef, &file_title );
         if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get file title.  Error code: " << static_cast<int>(status)));

         const static int BufferSize(1024);
         char path_buffer[BufferSize];
         status = FSRefMakePath(&fsRef, (UInt8*)path_buffer, BufferSize);
         if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get file path.  Error code: " << static_cast<int>(status)));

         std::string narrow_path(path_buffer);
         std::wstring filepath(narrow_path.begin(), narrow_path.end());
         
         if (returned_file_title) *returned_file_title = WideFromMacString(file_title);
         if (returned_filename) *returned_filename = filepath;
         
         CFRelease(file_title);
      }
   }
   
   NavDisposeReply(&navReply);
   
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
      if (song_title[i-1] == PathDelimiter)
      {
         song_title = song_title.substr(i, song_title.length());
         break;
      }
   }

   return song_title;
}

}; // End namespace
