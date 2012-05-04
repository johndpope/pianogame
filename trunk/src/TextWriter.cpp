
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include <map>

#include "TextWriter.h"
#include "Renderer.h"
#include "SynthesiaError.h"
#include "os_graphics.h"

#ifdef WIN32
// TODO: This should be deleted at shutdown
static std::map<int, HFONT> font_handle_lookup;
static int next_call_list_start = 1;
#else
// TODO: This should be deleted at shutdown
static std::map<int, ATSUStyle> atsu_style_lookup;
#endif

// TODO: This should be deleted at shutdown
static std::map<int, int> font_size_lookup;

TextWriter::TextWriter(int in_x, int in_y, Renderer &in_renderer, bool in_centered, int in_size, std::wstring fontname) :
x(in_x), y(in_y), size(in_size), original_x(0), last_line_height(0), centered(in_centered), renderer(in_renderer)
{
   x += renderer.m_xoffset;
   original_x = x;

   y += renderer.m_yoffset;

#ifdef WIN32

   Context c = renderer.m_context;
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

      // MACNOTE: Force Trebuchet MS.  It's what we mostly use anyway, but
      // I want to be sure they have it.
      const CFStringRef font_name = CFSTR("Trebuchet MS");
      
      ATSFontFamilyRef font = ATSFontFamilyFindFromName(font_name, kATSOptionFlagsDefault);
      if (!font) throw SynthesiaError(WSTRING(L"Couldn't get ATSFontFamilyRef for font '" << WideFromMacString(font_name) << L"'."));         
      
      AGLContext context = aglGetCurrentContext();
      if (!context) throw SynthesiaError(L"Couldn't retrieve OpenGL context while creating font.");         
      
      GLboolean ret = aglUseFont(context, font, normal, size, 0, 128, list_start);
      if (ret == GL_FALSE) throw SynthesiaError(WSTRING(L"aglUseFont() call failed with error code: " << aglGetError()));
      
      font_size_lookup[size] = list_start;


      // Create the ATSU style object that we'll use for calculating text extents and store it for later.
      ATSUStyle style;

      OSStatus status = ATSUCreateStyle(&style);
      if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't create ATSU style.  Error code: " << static_cast<int>(status)));

      Fixed fixed_size = Long2Fix(size);
      
      ATSUAttributeTag tags[] = { kATSUSizeTag };
      ByteCount sizes[] = { sizeof(Fixed) };
      ATSUAttributeValuePtr values[] = { &fixed_size };
      status = ATSUSetAttributes(style, sizeof(sizes) / sizeof(ByteCount), tags, sizes, values);
      if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't set ATSU style attributes.  Error code: " << static_cast<int>(status)));
      
      atsu_style_lookup[size] = style;      
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

   Context c = tw.renderer.m_context;
   int previous_map_mode = SetMapMode(c, MM_TEXT);

   HFONT font = font_handle_lookup[tw.size];

   // Create the font we want to use, and swap it out with
   // whatever is currently in there, along with our color
   HFONT previous_font = (HFONT)SelectObject(c, font);

   // Call DrawText to find out how large our text is
   RECT drawing_rect = { tw.x, tw.y, 0, 0 };
   tw.last_line_height = DrawText(c, m_text.c_str(), int(m_text.length()), &drawing_rect, options | DT_CALCRECT);

   // Return the hdc settings to their previous setting
   SelectObject(c, previous_font);
   SetMapMode(c, previous_map_mode);

#else

   // Convert passed-in text to Unicode
   CFStringRef cftext = MacStringFromWide(m_text, true).get();
   CFDataRef unitext = CFStringCreateExternalRepresentation(kCFAllocatorDefault, cftext, kCFStringEncodingUnicode, 0);
   if (!unitext) throw SynthesiaError(WSTRING(L"Couldn't convert string to unicode: '" << m_text << L"'"));
   CFRelease(cftext);

   // Create an ATSU layout
   ATSUTextLayout layout;
   const UniCharCount run_length = kATSUToTextEnd;
   OSStatus status = ATSUCreateTextLayoutWithTextPtr((ConstUniCharArrayPtr)CFDataGetBytePtr(unitext), kATSUFromTextBeginning, kATSUToTextEnd, CFDataGetLength(unitext) / 2, 1, &run_length, &atsu_style_lookup[tw.size], &layout);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't create ATSU text layout for string: '" << m_text << L"', Error code: " << static_cast<int>(status)));

   // Measure the size of the resulting text
   Rect drawing_rect = { 0, 0, 0, 0 };
   
   ATSUTextMeasurement before = 0;
   ATSUTextMeasurement after = 0;
   ATSUTextMeasurement ascent = 0;
   ATSUTextMeasurement descent = 0;
   
   status = ATSUGetUnjustifiedBounds(layout, 0, kATSUToTextEnd, &before, &after, &ascent, &descent);
   if (status != noErr) throw SynthesiaError(WSTRING(L"Couldn't get unjustified bounds for text layout for string: '" << m_text << L"', Error code: " << static_cast<int>(status)));

   // NOTE: the +1 here is completely arbitrary and seemed to place the text better.
   // It may just be a difference between the Windows and Mac text placement systems.
   drawing_rect.top += tw.y + 1;
   drawing_rect.left += tw.x + FixRound(before);
   drawing_rect.right += tw.x + FixRound(after);

   // Not used.
   drawing_rect.bottom = 0;

   // Clean-up
	ATSUDisposeTextLayout(layout);
   CFRelease(unitext);

#endif

   // Update the text-writer with post-draw coordinates
   if (tw.centered) drawing_rect.left -= (drawing_rect.right - drawing_rect.left) / 2;
   if (!tw.centered) tw.x += drawing_rect.right - drawing_rect.left;

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
