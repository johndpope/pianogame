// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "TextWriter.h"
#include "Renderer.h"
#include "SynthesiaError.h"
#include "os_graphics.h"

#include <map>

#ifdef WIN32
// TODO: This should be deleted at shutdown
static std::map<int, HFONT> font_handle_lookup;
static int next_call_list_start = 1;
#endif

// TODO: This should be deleted at shutdown
static std::map<int, int> font_size_lookup;

TextWriter::TextWriter(int in_x, int in_y, Renderer &in_renderer, bool in_centered, int in_size, std::wstring fontname) :
x(in_x), y(in_y), size(in_size), original_x(0), last_line_height(0), centered(in_centered), renderer(in_renderer)
{
   x += renderer.GetXoffset();
   original_x = x;

   y += renderer.GetYoffset();

#ifdef WIN32

   Context c = renderer.GetContext();
   point_size = MulDiv(size, GetDeviceCaps(c, LOGPIXELSY), 72);

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

      HFONT previous_font = (HFONT)SelectObject(c, font);

      wglUseFontBitmaps(c, 0, 128, next_call_list_start);
      font_size_lookup[in_size] = next_call_list_start;
      font_handle_lookup[in_size] = font;
      next_call_list_start += 130;

      SelectObject(c, previous_font);
   }

#else

   // TODO: is this sufficient?
   point_size = size;

   if (font_size_lookup[size] == 0)
   {
      int list_start = glGenLists(128);

      // MACNOTE: We ignore the passed-in font on the Mac for now
      // MACTODO: Apparently this font isn't a built-in system font
      const CFStringRef font_name = CFSTR("Palatino");
      
      ATSFontFamilyRef font = ATSFontFamilyFindFromName(font_name, kATSOptionFlagsDefault);
      if (!font) throw SynthesiaError(WSTRING(L"Couldn't get ATSFontFamilyRef for font '" << WideFromMacString(font_name) << L"'."));         
      
      AGLContext context = aglGetCurrentContext();
      if (!context) throw SynthesiaError(L"Couldn't retrieve OpenGL context while creating font.");         
      
      GLboolean ret = aglUseFont(context, font, normal, size, 0, 128, list_start);
      if (ret == GL_FALSE) throw SynthesiaError(WSTRING(L"aglUseFont() call failed with error code: " << aglGetError()));
      
      font_size_lookup[size] = list_start;
   }

#endif

}

TextWriter::~TextWriter()
{
}

int TextWriter::get_point_size() 
{
   return point_size;
}

TextWriter& TextWriter::next_line()
{
   y += std::max(last_line_height, get_point_size());
   x = original_x;

   last_line_height = 0;
   return *this;
}

TextWriter& Text::operator<<(TextWriter& tw) const
{
   int draw_x;
   int draw_y;
   calculate_position_and_advance_cursor(tw, &draw_x, &draw_y);

   // TODO: This isn't Unicode!
   std::string narrow(m_text.begin(), m_text.end());

   glBindTexture(GL_TEXTURE_2D, 0);
   
   glPushMatrix();
   tw.renderer.SetColor(m_color);
   glListBase(font_size_lookup[tw.size]); 
   glRasterPos2i(draw_x, draw_y + tw.size);
   glCallLists(static_cast<int>(narrow.length()), GL_UNSIGNED_BYTE, narrow.c_str());
   glPopMatrix();

   // TODO: Should probably delete these on shutdown.
   //glDeleteLists(1000, 128);

   return tw;
}

void Text::calculate_position_and_advance_cursor(TextWriter &tw, int *out_x, int *out_y) const
{
#ifdef WIN32

   const long options = DT_LEFT | DT_NOPREFIX;

   Context c = tw.renderer.GetContext();
   int previous_map_mode = SetMapMode(c, MM_TEXT);

   HFONT font = font_handle_lookup[tw.size];

   // Create the font we want to use, and swap it out with
   // whatever is currently in there, along with our color
   HFONT previous_font = (HFONT)SelectObject(c, font);

   // Call DrawText to find out how large our text is
   RECT drawing_rect = { tw.x, tw.y, 0, 0 };
   tw.last_line_height = DrawText(c, m_text.c_str(), int(m_text.length()), &drawing_rect, options | DT_CALCRECT);

   // Call it again to do the drawing, and get the line height
   if (tw.centered) drawing_rect.left -= (drawing_rect.right - drawing_rect.left) / 2;

   // Update the TextWriter, with however far we just wrote
   if (!tw.centered) tw.x += drawing_rect.right - drawing_rect.left;

   // Return the hdc settings to their previous setting
   SelectObject(c, previous_font);
   SetMapMode(c, previous_map_mode);

#else

   Rect drawing_rect;
   drawing_rect.left = tw.x;
   drawing_rect.top = tw.y;

   // TODO: Get the actual size of the text extents here
   drawing_rect.right = drawing_rect.left + int(m_text.length() * tw.get_point_size() * 0.475);
   drawing_rect.bottom = drawing_rect.top + tw.get_point_size();

   if (tw.centered) drawing_rect.left -= (drawing_rect.right - drawing_rect.left) / 2;
   if (!tw.centered) tw.x += drawing_rect.right - drawing_rect.left;

#endif

   // Tell the draw function where to put the text
   *out_x = drawing_rect.left;
   *out_y = drawing_rect.top;
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
