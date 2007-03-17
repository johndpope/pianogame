// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __TEXTWRITER_H
#define __TEXTWRITER_H

#include <vector>
#include <string>
#include <iomanip>
#include <windows.h>

#include "string_util.h"
#include "TrackProperties.h"

// A nice ostream-like class
class TextWriter
{
public:
   // Centering only works for single-write lines... in other words, centered
   // lines can only be 1 color.
   TextWriter(int in_x, int in_y, HDC in_hdc, bool in_centered = false, int in_size = 12, std::wstring fontname = L"Lucida Sans Unicode");
   ~TextWriter();

   // Skips at least 1 line, or the height of the last write... whichever is greater
   // (so that you can skip down past a multiline write)
   TextWriter& next_line();

   // Allow manipulators
   TextWriter& operator<<(TextWriter& (__cdecl *_Pfn)(TextWriter&))
   {   
      (*_Pfn)(*(TextWriter *)this);
      return (*this);
   }

   int TextWriter::get_point_size();

   int x, y, size, original_x;
   int last_line_height;
   bool centered;
   HFONT font;
   HDC hdc;

   friend class Text;

private:
   TextWriter operator=(const TextWriter&);
   TextWriter(const TextWriter&);
};

// Some colors to choose from, for convenience
enum TextColor { Black      = 0x00000000, Dk_Blue    = 0x00C40000, Dk_Green   = 0x0000C400, Dk_Cyan    = 0x00FF8000,
                 Dk_Red     = 0x000000C4, Dk_Purple  = 0x00800080, Brown      = 0x00004080, Gray       = 0x00BBBBBB,
                 Dk_Gray    = 0x00555555, Blue       = 0x00FF0000, Green      = 0x0000FF00, Cyan       = 0x00FFFF00,
                 Red        = 0x000000FF, Magenta    = 0x00FF00FF, Yellow     = 0x0000FFFF, White      = 0x00FFFFFF,
                 Orange     = 0x002080FF, Pink       = 0x00A080FF, CheatYellow= 0x0000CCFF };


// A class to use TextWriter, and write to the screen
class Text
{
public:
   Text(std::wstring in_txt, COLORREF in_col) : txt(in_txt), col(in_col) { }
   Text(int in_int, COLORREF in_col) : col(in_col), txt(WSTRING(in_int)) { }
   Text(double in_double, int prec, COLORREF in_col) : col(in_col), txt(WSTRING(std::setprecision(prec) << in_double)) { }

   Text(std::wstring in_txt, Color in_col) : txt(in_txt), col(ToRGB(in_col)) { }
   Text(int in_int, Color in_col) : col(ToRGB(in_col)), txt(WSTRING(in_int)) { }
   Text(double in_double, int prec, Color in_col) : col(ToRGB(in_col)), txt(WSTRING(std::setprecision(prec) << in_double)) { }

   TextWriter& operator<<(TextWriter& tw);

private:
   std::wstring txt;
   COLORREF col;
};

// newline manipulator
TextWriter& newline(TextWriter& tw);

TextWriter& operator<<(TextWriter& tw, Text& t);
TextWriter& operator<<(TextWriter& tw, const std::wstring& s);
TextWriter& operator<<(TextWriter& tw, const int& i);
TextWriter& operator<<(TextWriter& tw, const unsigned int& i);
TextWriter& operator<<(TextWriter& tw, const long& l);
TextWriter& operator<<(TextWriter& tw, const unsigned long& l);

#endif
