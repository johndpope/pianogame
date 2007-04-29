// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "TextWriter.h"
#include "Renderer.h"

#ifdef WIN32
#include <gl/gl.h>
#else
#include <OpenGL/OpenGL.h>
#include <AGL/gl.h>
#endif

#include <map>

#ifdef WIN32

// TODO: These should be deleted at shutdown
static std::map<int, int> font_size_lookup;
static std::map<int, HFONT> font_handle_lookup;
static int next_call_list_start = 1;

TextWriter::TextWriter(int in_x, int in_y, Renderer &in_renderer, bool in_centered, int in_size, std::wstring fontname) :
x(in_x), y(in_y), size(in_size), original_x(in_x), last_line_height(0), centered(in_centered), renderer(in_renderer)
{
   x += renderer.GetXoffset();
   original_x = x;

   y += renderer.GetYoffset();

   HFONT font = 0;
   if (font_size_lookup[in_size] == 0)
   {
      // Set up the LOGFONT structure
      LOGFONT logical_font;
      logical_font.lfHeight = get_point_size();
      logical_font.lfWidth = 0;
      logical_font.lfEscapement = 0;
      logical_font.lfOrientation = 0;
      logical_font.lfWeight = FW_NORMAL;
      logical_font.lfItalic = false;
      logical_font.lfUnderline = false;
      logical_font.lfStrikeOut = false;
      logical_font.lfCharSet = ANSI_CHARSET;
      logical_font.lfOutPrecision = OUT_DEFAULT_PRECIS;
      logical_font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      logical_font.lfQuality = PROOF_QUALITY;
      logical_font.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
      lstrcpy(logical_font.lfFaceName, fontname.c_str()); 

      font = CreateFontIndirect(&logical_font);

      HFONT previous_font = (HFONT)SelectObject(renderer.GetHdc(), font);

      wglUseFontBitmaps(renderer.GetHdc(), 0, 255, next_call_list_start);
      font_size_lookup[in_size] = next_call_list_start;
      font_handle_lookup[in_size] = font;
      next_call_list_start += 260;

      SelectObject(renderer.GetHdc(), previous_font);
   }
}

TextWriter::~TextWriter()
{
}

int TextWriter::get_point_size() 
{
   return MulDiv(size, GetDeviceCaps(renderer.GetHdc(), LOGPIXELSY), 72);
}

TextWriter& Text::operator<<(TextWriter& tw) const
{
   const long options = DT_LEFT | DT_NOPREFIX;

   int previous_map_mode = SetMapMode(tw.renderer.GetHdc(), MM_TEXT);

   HFONT font = font_handle_lookup[tw.size];

   // Create the font we want to use, and swap it out with
   // whatever is currently in there, along with our color
   HFONT previous_font = (HFONT)SelectObject(tw.renderer.GetHdc(), font);

   // Call DrawText to find out how large our text is
   RECT drawing_rect = { tw.x, tw.y, 0, 0 };
   tw.last_line_height = DrawText(tw.renderer.GetHdc(), m_text.c_str(), int(m_text.length()), &drawing_rect, options | DT_CALCRECT);

   // Call it again to do the drawing, and get the line height
   if (tw.centered) drawing_rect.left -= (drawing_rect.right - drawing_rect.left) / 2;

   // Update the TextWriter, with however far we just wrote
   if (!tw.centered) tw.x += drawing_rect.right - drawing_rect.left;

   // TODO: This isn't Unicode!
   std::string narrow(m_text.begin(), m_text.end());

   glBindTexture(GL_TEXTURE_2D, 0);
   
   glPushMatrix();
   tw.renderer.SetColor(m_color);
   glListBase(font_size_lookup[tw.size]); 
   glRasterPos2i(drawing_rect.left, drawing_rect.top + tw.size);
   glCallLists(static_cast<int>(narrow.length()), GL_UNSIGNED_BYTE, narrow.c_str());
   glPopMatrix();

   // TODO: Should probably delete these on shutdown.
   //glDeleteLists(1000, 255);


   // Return the hdc settings to how they previously were
   SelectObject(tw.renderer.GetHdc(), previous_font);
   SetMapMode(tw.renderer.GetHdc(), previous_map_mode);

   return tw;
}

#else

// MACTODO: Platform dependent TextWriter code

TextWriter::TextWriter(int in_x, int in_y, Renderer &in_renderer, bool in_centered, int in_size, std::wstring fontname) :
x(in_x), y(in_y), size(in_size), original_x(in_x), last_line_height(0), centered(in_centered), renderer(in_renderer)
{
}

TextWriter::~TextWriter()
{
}

int TextWriter::get_point_size() 
{
   return 0;
}

TextWriter& Text::operator<<(TextWriter& tw) const
{
   return tw;
}

#endif

TextWriter& TextWriter::next_line()
{
   y += std::max(last_line_height, get_point_size());
   x = original_x;

   last_line_height = 0;
   return *this;
}

TextWriter& operator<<(TextWriter& tw, const Text& t)
{
   return t.operator <<(tw);
}

TextWriter& newline(TextWriter& tw)
{
   return tw.next_line();
}

TextWriter& operator<<(TextWriter& tw, const std::wstring& s)  { return tw << Text(s, White); }
TextWriter& operator<<(TextWriter& tw, const int& i)           { return tw << Text(i, White); }
TextWriter& operator<<(TextWriter& tw, const unsigned int& i)  { return tw << Text(i, White); }
TextWriter& operator<<(TextWriter& tw, const long& l)          { return tw << Text(l, White); }
TextWriter& operator<<(TextWriter& tw, const unsigned long& l) { return tw << Text(l, White); }
