// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "KeyboardDisplay.h"
#include "TrackProperties.h"
#include "PianoHeroError.h"
#include "string_util.h"

using namespace std;

struct NoteBrush
{
   HBRUSH white;
   HBRUSH black;
   HBRUSH outline;
};


KeyboardDisplay::KeyboardDisplay(KeyboardSize size, int pixelWidth, int pixelHeight)
   : m_size(size), m_width(pixelWidth), m_height(pixelHeight),
   m_cached_background(pixelWidth, pixelHeight), m_background_initialized(false)
{
}



void KeyboardDisplay::Draw(HDC hdc, int x, int y, const TranslatedNoteSet &notes,
                           unsigned long long show_duration, unsigned long long current_time,
                           const std::vector<TrackProperties> &track_properties)
{
   // Source: Measured from Yamaha P-70
   const static double WhiteWidthHeightRatio = 6.8181818;
   const static double BlackWidthHeightRatio = 7.9166666;
   const static double WhiteBlackWidthRatio = 0.5454545;

   const char starting_note = GetStartingNote();
   const int white_key_count = GetWhiteKeyCount();

   // Calculate the largest white key size we can, and then
   // leave room for a single pixel space between each key
   int white_width = (m_width / white_key_count) - 1;
   int white_space = 1;

   int white_height = static_cast<int>(white_width * WhiteWidthHeightRatio);

   int black_width = static_cast<int>(white_width * WhiteBlackWidthRatio);
   int black_height = static_cast<int>(black_width * BlackWidthHeightRatio);
   int black_offset = white_width - (black_width / 2);

   // The dimensions given to the keyboard object are bounds.  Because of pixel
   // rounding, the keyboard will usually occupy less than the maximum in
   // either direction.
   //
   // So, we just try to center the keyboard inside the bounds.
   int final_width = (white_width + white_space) * white_key_count;
   int x_offset = (m_width - final_width) / 2;
   int y_offset = (m_height - white_height);

   // Generate our one-time background image that has the guidelines,
   // and empty keyboard.
   if (!m_background_initialized)
   {
      // Temporarily turn off any active keys
      KeyNames active_keys;
      m_active_keys = KeyNames();

      HDC bg = m_cached_background.beginDrawingOn();

      DrawWhiteKeys(bg, false, white_key_count, white_width, white_height, white_space, x_offset, y_offset);
      DrawBlackKeys(bg, false, white_key_count, white_width, white_height, black_width, black_height, white_space, x_offset, y_offset, black_offset);

      DrawGuides(bg, white_key_count, white_width, white_space, x_offset, 0, y_offset);

      m_cached_background.endDrawingOn();

      m_active_keys = active_keys;
      m_background_initialized = true;
   }

   m_cached_background.beginDrawing(hdc);
   m_cached_background.draw(x, y);
   m_cached_background.endDrawing();

   DrawWhiteKeys(hdc, true, white_key_count, white_width, white_height, white_space, x+x_offset, y+y_offset);
   DrawBlackKeys(hdc, false, white_key_count, white_width, white_height, black_width, black_height, white_space, x+x_offset, y+y_offset, black_offset);

   DrawNotes(hdc, white_width, white_space, black_width, black_offset, x + x_offset, y, y_offset, notes, show_duration, current_time, track_properties);
}

int KeyboardDisplay::GetStartingOctave() const
{
   // Source: Various "Specification" pages at Yamaha's website
   const static int StartingOctaveOn61 = 1;
   const static int StartingOctaveOn76 = 0;
   const static int StartingOctaveOn88 = 0;

   switch (m_size)
   {
   case KeyboardSize61: return StartingOctaveOn61;
   case KeyboardSize76: return StartingOctaveOn76;
   case KeyboardSize88: return StartingOctaveOn88;
   default: throw PianoHeroError(Error_BadPianoType);
   }
}

char KeyboardDisplay::GetStartingNote() const
{
   // Source: Various "Specification" pages at Yamaha's website
   const static char StartingKeyOn61 = 'C'; // C1-C6
   const static char StartingKeyOn76 = 'E'; // E0-G6
   const static char StartingKeyOn88 = 'A'; // A0-C6

   switch (m_size)
   {
   case KeyboardSize61: return StartingKeyOn61;
   case KeyboardSize76: return StartingKeyOn76;
   case KeyboardSize88: return StartingKeyOn88;
   default: throw PianoHeroError(Error_BadPianoType);
   }
}

int KeyboardDisplay::GetWhiteKeyCount() const
{
   // Source: Google Image Search
   const static int WhiteKeysOn61 = 36;
   const static int WhiteKeysOn76 = 45;
   const static int WhiteKeysOn88 = 52;

   switch (m_size)
   {
   case KeyboardSize61: return WhiteKeysOn61;
   case KeyboardSize76: return WhiteKeysOn76;
   case KeyboardSize88: return WhiteKeysOn88;
   default: throw PianoHeroError(Error_BadPianoType);
   }
}


void KeyboardDisplay::DrawWhiteKeys(HDC hdc, bool active_only, int key_count, int key_width, int key_height,
   int key_space, int x_offset, int y_offset) const
{
   HBRUSH white_brush = WHITE_BRUSH;

   HBRUSH color_brushes[TrackColorCount];
   for (int i = 0; i < TrackColorCount; ++i)
   {
      color_brushes[i] = CreateSolidBrush(TrackColorNoteWhite[i]);
   }

   char current_white = GetStartingNote();
   int current_octave = GetStartingOctave() + 1;
   for (int i = 0; i < key_count; ++i)
   {
      const int key_x = i * (key_width + key_space) + x_offset;

      RECT white_rect = { key_x, y_offset, key_x + key_width, y_offset + key_height };

      // Check to see if this is one of the active notes
      const string note_name = STRING(current_white << current_octave);

      KeyNames::const_iterator find_result = m_active_keys.find(note_name);
      bool active = (find_result != m_active_keys.end());

      HBRUSH brush = white_brush;
      if (active) brush = color_brushes[find_result->second];

      if ((active_only && active) || !active_only) FillRect(hdc, &white_rect, brush);

      current_white++;
      if (current_white == 'H') current_white = 'A';
      if (current_white == 'C') current_octave++;
   }

   for (int i = 0; i < TrackColorCount; ++i)
   {
      DeleteObject(color_brushes[i]);
   }

}

void KeyboardDisplay::DrawBlackKeys(HDC hdc, bool active_only, int white_key_count, int white_width, int white_height,
   int black_width, int black_height, int key_space, int x_offset, int y_offset, int black_offset) const
{
   HBRUSH black_brush = CreateSolidBrush(RGB(0x20, 0x20, 0x20));

   // Create brushes for each of our possible note colors
   HBRUSH color_brushes[TrackColorCount];
   for (int i = 0; i < TrackColorCount; ++i)
   {
      color_brushes[i] = CreateSolidBrush(TrackColorNoteBlack[i]);
   }

   char current_white = GetStartingNote();
   int current_octave = GetStartingOctave() + 1;
   for (int i = 0; i < white_key_count; ++i)
   {
      // Don't allow a very last black key
      if (i == white_key_count - 1) break;

      switch (current_white)
      {
      case 'A':
      case 'C':
      case 'D':
      case 'F':
      case 'G':
         {
            const int start_x = i * (white_width + key_space) + x_offset + black_offset;

            RECT black_rect = { start_x, y_offset, start_x + black_width, y_offset + black_height };

            // Check to see if this is one of the active notes
            const string note_name = STRING(current_white << '#' << current_octave);

            KeyNames::const_iterator find_result = m_active_keys.find(note_name);
            bool active = (find_result != m_active_keys.end());

            HBRUSH brush = black_brush;
            if (active) brush = color_brushes[find_result->second];

            if (!active_only || (active_only && active)) FillRect(hdc, &black_rect, brush);
         }
      }

      current_white++;
      if (current_white == 'H') current_white = 'A';
      if (current_white == 'C') current_octave++;
   }

   DeleteObject(black_brush);

   for (int i = 0; i < TrackColorCount; ++i)
   {
      DeleteObject(color_brushes[i]);
   }
}

void KeyboardDisplay::DrawGuides(HDC hdc, int key_count, int key_width, int key_space,
                                 int x_offset, int y, int y_offset) const
{
   const static int PixelsOffKeyboard = 2;
   int keyboard_width = key_width*key_count + key_space*(key_count-1);

   // Fill the background of the note-falling area
   HBRUSH fill_brush = CreateSolidBrush(RGB(0x60, 0x60, 0x60));

   RECT fill_rect = { x_offset, y, x_offset + keyboard_width, y + y_offset - PixelsOffKeyboard };
   FillRect(hdc, &fill_rect, fill_brush);


   HPEN guide_pen = CreatePen(PS_SOLID, 1, RGB(0x50, 0x50, 0x50));
   HPEN old_pen = static_cast<HPEN>(SelectObject(hdc, guide_pen));

   // Draw horizontal-lines
   MoveToEx(hdc, x_offset,                  y, 0);
   LineTo  (hdc, x_offset + keyboard_width, y);
   MoveToEx(hdc, x_offset,                  y + y_offset, 0);
   LineTo  (hdc, x_offset + keyboard_width, y + y_offset);
   MoveToEx(hdc, x_offset,                  y + y_offset - PixelsOffKeyboard - 1, 0);
   LineTo  (hdc, x_offset + keyboard_width, y + y_offset - PixelsOffKeyboard - 1);

   int octave_start_x = x_offset;
   char current_white = GetStartingNote() - 1;
   int current_octave = GetStartingOctave() + 1;
   for (int i = 0; i < key_count+1; ++i)
   {
      const int key_x = i * (key_width + key_space) + x_offset - 1;

      switch (current_white)
      {
      case 'A':
      case 'C':
      case 'D':
      case 'F':
      case 'G':
         {
            MoveToEx(hdc, key_x, y, 0);
            LineTo  (hdc, key_x, y + y_offset - PixelsOffKeyboard);
         }
      }

      current_white++;
      if (current_white == 'H') current_white = 'A';
      if (current_white == 'C') current_octave++;
   }

   DeleteObject(fill_brush);

   SelectObject(hdc, old_pen);
   DeleteObject(guide_pen);
}

void KeyboardDisplay::DrawNotes(HDC hdc, int white_width, int key_space, int black_width, int black_offset,
                                 int x_offset, int y, int y_offset, const TranslatedNoteSet &notes,
                                 unsigned long long show_duration, unsigned long long current_time,
                                 const std::vector<TrackProperties> &track_properties) const
{
   // Create brushes for each of our possible note colors
   NoteBrush brushes[TrackColorCount];
   for (int i = 0; i < TrackColorCount; ++i)
   {
      brushes[i].white   = CreateSolidBrush(TrackColorNoteWhite[i]);
      brushes[i].black   = CreateSolidBrush(TrackColorNoteBlack[i]);
      brushes[i].outline = CreateSolidBrush(TrackColorNoteBorder[i]);
   }

   for (TranslatedNoteSet::const_iterator i = notes.begin(); i != notes.end(); ++i)
   {
      TrackMode mode = track_properties[i->track_id].mode;
      if (mode == ModeNotPlayed) continue;
      if (mode == ModePlayedButHidden) continue;

      TrackColor color = track_properties[i->track_id].color;
      const NoteBrush &brush_set = brushes[color];

      // This list is sorted by note start time.  The moment we encounter
      // a note scrolled off the window, we're done drawing
      if (i->start > current_time + show_duration) break;

      unsigned long long adjusted_start = max(current_time, i->start) - current_time;
      unsigned long long adjusted_end = min(current_time + show_duration, i->end) - current_time;

      bool hitting_bottom = (adjusted_start + current_time != i->start);
      bool hitting_top    = (adjusted_end   + current_time != i->end);

      double scaling_factor = static_cast<double>(y_offset) / static_cast<double>(show_duration);

      // Convert our times to pixel coordinates
      int y_end = y - static_cast<int>(adjusted_start * scaling_factor) + y_offset;
      int y_start = y - static_cast<int>(adjusted_end * scaling_factor) + y_offset;

      // Shiny music domain knowledge
      const static unsigned int NotesPerOctave = 12;
      const static unsigned int WhiteNotesPerOctave = 7;
      const static bool IsBlackNote[12] = { false, true,  false, true,  false, false,
                                            true,  false, true,  false, true,  false };

      // This array describes how to "stack" notes in a single place.  The IsBlackNote array
      // then tells which one should be shifted slightly to the right
      const static int NoteToWhiteNoteOffset[12] = { 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -5, -5 };

      const int octave = (i->note_id / NotesPerOctave) - GetStartingOctave();
      const int octave_base = i->note_id % NotesPerOctave;
      const int stack_offset = NoteToWhiteNoteOffset[octave_base];
      const bool is_black = IsBlackNote[octave_base];

      // The constants used in the switch below refer to the number
      // of white keys off 'C' that type of piano starts on
      int keyboard_type_offset = 0;
      switch (m_size)
      {
      case KeyboardSize61: keyboard_type_offset = 7 - WhiteNotesPerOctave; break;
      case KeyboardSize76: keyboard_type_offset = 5 - WhiteNotesPerOctave; break;
      case KeyboardSize88: keyboard_type_offset = 2 - WhiteNotesPerOctave; break;
      default: throw PianoHeroError(Error_BadPianoType);
      }

      const int octave_offset = (max(octave - 1, 0) * WhiteNotesPerOctave);
      const int inner_octave_offset = (octave_base + stack_offset);
      const int generalized_black_offset = (is_black?black_offset:0);

      const int start_x = (octave_offset + inner_octave_offset + keyboard_type_offset) * (white_width + key_space)
         + generalized_black_offset + x_offset;

      RECT outline_rect = { start_x - 1, y_start, start_x + (is_black?black_width:white_width) + 1, y_end };

      // Force a note to be at least 1 pixel tall at all times
      // except when scrolling off underneath the keyboard and
      // coming in from the top of the screen.
      if (!hitting_bottom && !hitting_top)
      {
         while (outline_rect.bottom - outline_rect.top < 3) outline_rect.bottom++;
      }

      RECT note_rect = { outline_rect.left + 1, outline_rect.top + 1, outline_rect.right - 1, outline_rect.bottom - 1 };

      FillRect(hdc, &outline_rect, brush_set.outline);

      HBRUSH note_brush = brush_set.white;
      if (is_black) note_brush = brush_set.black;
      FillRect(hdc, &note_rect, note_brush);
   }

   for (int i = 0; i < TrackColorCount; ++i)
   {
      DeleteObject(brushes[i].white);
      DeleteObject(brushes[i].black);
      DeleteObject(brushes[i].outline);
   }
}

void KeyboardDisplay::SetKeyActive(const string &key_name, bool active, TrackColor color)
{
   if (active) m_active_keys[key_name] = color;
   else m_active_keys.erase(key_name);
}
