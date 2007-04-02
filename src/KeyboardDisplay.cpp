// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "KeyboardDisplay.h"
#include "TrackProperties.h"
#include "PianoHeroError.h"
#include "string_util.h"
#include "Renderer.h"

using namespace std;

KeyboardDisplay::KeyboardDisplay(KeyboardSize size, int pixelWidth, int pixelHeight)
   : m_size(size), m_width(pixelWidth), m_height(pixelHeight)
{
}



void KeyboardDisplay::Draw(Renderer &renderer, int x, int y, const TranslatedNoteSet &notes,
                           microseconds_t show_duration, microseconds_t current_time,
                           const std::vector<TrackProperties> &track_properties)
{
   // Source: Measured from Yamaha P-70
   const static double WhiteWidthHeightRatio = 6.8181818;
   const static double BlackWidthHeightRatio = 7.9166666;
   const static double WhiteBlackWidthRatio = 0.5454545;

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

   DrawGuides(renderer, white_key_count, white_width, white_space, x + x_offset, y, y_offset);
   DrawNotes(renderer, white_width, white_space, black_width, black_offset, x + x_offset, y, y_offset, notes, show_duration, current_time, track_properties);

   DrawWhiteKeys(renderer, false, white_key_count, white_width, white_height, white_space, x+x_offset, y+y_offset);
   DrawBlackKeys(renderer, false, white_key_count, white_width, black_width, black_height, white_space, x+x_offset, y+y_offset, black_offset);
}

int KeyboardDisplay::GetStartingOctave() const
{
   // Source: Various "Specification" pages at Yamaha's website
   const static int StartingOctaveOn37 = 2;
   const static int StartingOctaveOn49 = 2; // TODO!
   const static int StartingOctaveOn61 = 1; // TODO!
   const static int StartingOctaveOn76 = 0; // TODO!
   const static int StartingOctaveOn88 = 0;

   switch (m_size)
   {
   case KeyboardSize37: return StartingOctaveOn37;
   case KeyboardSize49: return StartingOctaveOn49;
   case KeyboardSize61: return StartingOctaveOn61;
   case KeyboardSize76: return StartingOctaveOn76;
   case KeyboardSize88: return StartingOctaveOn88;
   default: throw PianoHeroError(Error_BadPianoType);
   }
}

char KeyboardDisplay::GetStartingNote() const
{
   // Source: Various "Specification" pages at Yamaha's website
   const static char StartingKeyOn37 = 'F'; // F3-F6
   const static char StartingKeyOn49 = 'C'; // C3-C6 // TODO!
   const static char StartingKeyOn61 = 'C'; // C1-C6 // TODO!
   const static char StartingKeyOn76 = 'E'; // E0-G6 // TODO!
   const static char StartingKeyOn88 = 'A'; // A0-C6

   switch (m_size)
   {
   case KeyboardSize37: return StartingKeyOn37;
   case KeyboardSize49: return StartingKeyOn49;
   case KeyboardSize61: return StartingKeyOn61;
   case KeyboardSize76: return StartingKeyOn76;
   case KeyboardSize88: return StartingKeyOn88;
   default: throw PianoHeroError(Error_BadPianoType);
   }
}

int KeyboardDisplay::GetWhiteKeyCount() const
{
   // Source: Google Image Search
   const static int WhiteKeysOn37 = 22;
   const static int WhiteKeysOn49 = 29;
   const static int WhiteKeysOn61 = 36;
   const static int WhiteKeysOn76 = 45;
   const static int WhiteKeysOn88 = 52;

   switch (m_size)
   {
   case KeyboardSize37: return WhiteKeysOn37;
   case KeyboardSize49: return WhiteKeysOn49;
   case KeyboardSize61: return WhiteKeysOn61;
   case KeyboardSize76: return WhiteKeysOn76;
   case KeyboardSize88: return WhiteKeysOn88;
   default: throw PianoHeroError(Error_BadPianoType);
   }
}


void KeyboardDisplay::DrawWhiteKeys(Renderer &renderer, bool active_only, int key_count, int key_width, int key_height,
   int key_space, int x_offset, int y_offset) const
{
   Color white = ToColor(255, 255, 255);

   char current_white = GetStartingNote();
   int current_octave = GetStartingOctave() + 1;
   for (int i = 0; i < key_count; ++i)
   {
      // Check to see if this is one of the active notes
      const string note_name = STRING(current_white << current_octave);

      KeyNames::const_iterator find_result = m_active_keys.find(note_name);
      bool active = (find_result != m_active_keys.end());

      Color c = white;
      if (active) c = TrackColorNoteWhite[find_result->second];

      if ((active_only && active) || !active_only)
      {
         renderer.SetColor(c);

         const int key_x = i * (key_width + key_space) + x_offset;
         renderer.DrawQuad(key_x, y_offset, key_width, key_height);
      }

      current_white++;
      if (current_white == 'H') current_white = 'A';
      if (current_white == 'C') current_octave++;
   }
}

void KeyboardDisplay::DrawBlackKeys(Renderer &renderer, bool active_only, int white_key_count, int white_width,
   int black_width, int black_height, int key_space, int x_offset, int y_offset, int black_offset) const
{
   Color black = ToColor(0x20, 0x20, 0x20);

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
            // Check to see if this is one of the active notes
            const string note_name = STRING(current_white << '#' << current_octave);

            KeyNames::const_iterator find_result = m_active_keys.find(note_name);
            bool active = (find_result != m_active_keys.end());

            Color c = black;
            if (active) c = TrackColorNoteBlack[find_result->second];

            if (!active_only || (active_only && active))
            {
               renderer.SetColor(c);

               const int start_x = i * (white_width + key_space) + x_offset + black_offset;
               renderer.DrawQuad(start_x, y_offset, black_width, black_height);
            }
         }
      }

      current_white++;
      if (current_white == 'H') current_white = 'A';
      if (current_white == 'C') current_octave++;
   }
}

void KeyboardDisplay::DrawGuides(Renderer &renderer, int key_count, int key_width, int key_space,
                                 int x_offset, int y, int y_offset) const
{
   const static int PixelsOffKeyboard = 2;
   int keyboard_width = key_width*key_count + key_space*(key_count-1);

   // Fill the background of the note-falling area
   renderer.SetColor(0x60, 0x60, 0x60);
   renderer.DrawQuad(x_offset, y, keyboard_width, y_offset - PixelsOffKeyboard);

   const static Color thick(ToColor(0x48,0x48,0x48));
   const static Color thin(ToColor(0x50,0x50,0x50));
   const static Color center(ToColor(0x70,0x70,0x70));

   char current_white = GetStartingNote() - 1;
   int current_octave = GetStartingOctave() + 1;
   for (int i = 0; i < key_count + 1; ++i)
   {
      const int key_x = i * (key_width + key_space) + x_offset - 1;

      int guide_thickness = 2;
      Color guide_color = thin;

      bool draw_guide = true;
      switch (current_white)
      {
      case 'C':
      case 'D':
         guide_color = thin;
         if (current_octave == 5) guide_color = center;
         break;

      case 'F':
      case 'G':
      case 'A':
         guide_color = thick;
         guide_thickness = 3;
         break;

      default:
         draw_guide = false;
         break;
      }

      if (draw_guide)
      {
         renderer.SetColor(guide_color);
         renderer.DrawQuad(key_x - guide_thickness/2, y, guide_thickness, y_offset - PixelsOffKeyboard);
      }

      current_white++;
      if (current_white == 'H') current_white = 'A';
      if (current_white == 'C') current_octave++;
   }

   // Draw horizontal-lines
   const static int hr_thickness = 2;
   renderer.SetColor(thick);
   renderer.DrawQuad(x_offset, y, keyboard_width, hr_thickness);
   renderer.DrawQuad(x_offset, y+y_offset, keyboard_width, hr_thickness);
   renderer.DrawQuad(x_offset, y+y_offset-PixelsOffKeyboard, keyboard_width, hr_thickness);
}

void KeyboardDisplay::DrawNotes(Renderer &renderer, int white_width, int key_space, int black_width, int black_offset,
                                 int x_offset, int y, int y_offset, const TranslatedNoteSet &notes,
                                 microseconds_t show_duration, microseconds_t current_time,
                                 const std::vector<TrackProperties> &track_properties) const
{
   for (TranslatedNoteSet::const_iterator i = notes.begin(); i != notes.end(); ++i)
   {
      const TrackMode mode = track_properties[i->track_id].mode;
      if (mode == ModeNotPlayed) continue;
      if (mode == ModePlayedButHidden) continue;

      const TrackColor color = track_properties[i->track_id].color;
      const int &brush_id = (i->state == UserMissed ? MissedNote : color);

      // This list is sorted by note start time.  The moment we encounter
      // a note scrolled off the window, we're done drawing
      if (i->start > current_time + show_duration) break;

      const long long adjusted_start = max(max(current_time,                 i->start) - current_time, 0);
      const long long adjusted_end   = max(min(current_time + show_duration, i->end)   - current_time, 0);

      if (adjusted_end < adjusted_start) continue;

      const double scaling_factor = static_cast<double>(y_offset) / static_cast<double>(show_duration);

      // Convert our times to pixel coordinates
      const int y_end   = y - static_cast<int>(adjusted_start * scaling_factor) + y_offset;
      const int y_start = y - static_cast<int>(adjusted_end   * scaling_factor) + y_offset;

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
      case KeyboardSize37: keyboard_type_offset = 4 - WhiteNotesPerOctave; break;
      case KeyboardSize49: keyboard_type_offset = 0 - WhiteNotesPerOctave; break; // TODO!
      case KeyboardSize61: keyboard_type_offset = 7 - WhiteNotesPerOctave; break; // TODO!
      case KeyboardSize76: keyboard_type_offset = 5 - WhiteNotesPerOctave; break; // TODO!
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
      const bool hitting_bottom = (adjusted_start + current_time != i->start);
      const bool hitting_top    = (adjusted_end   + current_time != i->end);
      if (!hitting_bottom && !hitting_top)
      {
         while ( (outline_rect.bottom - outline_rect.top) < 3) outline_rect.bottom++;
      }

      renderer.SetColor(TrackColorNoteBorder[brush_id]);
      renderer.DrawQuad(outline_rect.left, outline_rect.top, outline_rect.right - outline_rect.left, outline_rect.bottom - outline_rect.top);

      Color c = (i->state == UserHit ? TrackColorNoteHit[brush_id] : TrackColorNoteWhite[brush_id]);
      if (is_black) c = (i->state == UserHit ? TrackColorNoteHit[brush_id] : TrackColorNoteBlack[brush_id]);
      renderer.SetColor(c);

      const RECT note_rect = { outline_rect.left + 1, outline_rect.top + 1, outline_rect.right - 1, outline_rect.bottom - 1 };
      renderer.DrawQuad(note_rect.left, note_rect.top, note_rect.right - note_rect.left, note_rect.bottom - note_rect.top);
   }
}

void KeyboardDisplay::SetKeyActive(const string &key_name, bool active, TrackColor color)
{
   if (active) m_active_keys[key_name] = color;
   else m_active_keys.erase(key_name);
}
