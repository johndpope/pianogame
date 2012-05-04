
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __FILE_SELECTOR_H
#define __FILE_SELECTOR_H

#include <string>

namespace FileSelector
{
   // Presents a standard "File Open" dialog box.  Returns empty string in [filename]
   // if user presses cancel.  Also, remembers last filename
   void RequestMidiFilename(std::wstring *filename, std::wstring *file_title);

   // If a filename was passed in on the command line, we
   // can remember it for future file-open dialogs
   void SetLastMidiFilename(const std::wstring &filename);

   // Returns a filename with no path or .mid/.midi extension
   std::wstring TrimFilename(const std::wstring &filename);
};

#endif
