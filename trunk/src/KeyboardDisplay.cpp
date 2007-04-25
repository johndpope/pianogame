// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "KeyboardDisplay.h"
#include "TrackProperties.h"
#include "PianoHeroError.h"
#include "string_util.h"

#include "Renderer.h"
#include "Textures.h"
#include "Tga.h"

using namespace std;

const KeyboardDisplay::NoteTexDimensions KeyboardDisplay::WhiteNoteDimensions = { 32, 128, 4, 25, 22, 28, 93, 100 };
const KeyboardDisplay::NoteTexDimensions KeyboardDisplay::BlackNoteDimensions = { 32,  64, 8, 20,  3,  8, 49,  55 };
   struct KeyTexDimensions
   {
      int tex_width;
      int tex_height;

      int left;
      int right;

      int top;
      int bottom;
   };
   const KeyboardDisplay::KeyTexDimensions KeyboardDisplay::BlackKeyDimensions = { 32, 128, 8, 20, 15, 109 };


KeyboardDisplay::KeyboardDisplay(KeyboardSize size, int pixelWidth, int pixelHeight)
   : m_size(size), m_width(pixelWidth), m_height(pixelHeight)
{
}



void KeyboardDisplay::Draw(Renderer &renderer, const Tga *key_tex[3], const Tga *note_tex[4], int x, int y,
                           const TranslatedNoteSet &notes, microseconds_t show_duration, microseconds_t current_time,
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

   const int black_width = static_cast<int>(white_width * WhiteBlackWidthRatio);
   const int black_height = static_cast<int>(black_width * BlackWidthHeightRatio);
   const int black_offset = white_width - (black_width / 2);

   // The dimensions given to the keyboard object are bounds.  Because of pixel
   // rounding, the keyboard will usually occupy less than the maximum in
   // either direction.
   //
   // So, we just try to center the keyboard inside the bounds.
   const int final_width = (white_width + white_space) * white_key_count;
   const int x_offset = (m_width - final_width) / 2;
   const int y_offset = (m_height - white_height);

   // Give the notes a little more room to work with so they can roll under
   // the keys without distortion
   const int y_roll_under = white_height*3/4;

   // Symbolic names for the arbitrary array passed in here
   enum { Rail, Shadow, BlackKey };

   DrawGuides(renderer, white_key_count, white_width, white_space, x + x_offset, y, y_offset);

   // Do two passes on the notes, the first for note shadows and the second
   // for the note blocks themselves.  This is to avoid shadows being drawn
   // on top of notes.
   renderer.SetColor(ToColor(255, 255, 255));
   DrawNotePass(renderer, note_tex[0], note_tex[1], white_width, white_space, black_width, black_offset, x + x_offset, y, y_offset, y_roll_under, notes, show_duration, current_time, track_properties);
   DrawNotePass(renderer, note_tex[2], note_tex[3], white_width, white_space, black_width, black_offset, x + x_offset, y, y_offset, y_roll_under, notes, show_duration, current_time, track_properties);

   // Black out the background of where the keys are about to appear
   renderer.SetColor(ToColor(0, 0, 0));
   renderer.DrawQuad(x + x_offset, y+y_offset, m_width, white_height);

   DrawWhiteKeys(renderer, false, white_key_count, white_width, white_height, white_space, x+x_offset, y+y_offset);
   DrawBlackKeys(renderer, key_tex[BlackKey], false, white_key_count, white_width, black_width, black_height, white_space, x+x_offset, y+y_offset, black_offset);
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

void KeyboardDisplay::DrawBlackKey(Renderer &renderer, const Tga *tex, const KeyTexDimensions &tex_dimensions,
                                   int x, int y, int w, int h, TrackColor color) const
{
   const KeyTexDimensions &d = tex_dimensions;

   const int tex_w = d.right - d.left;
   const double width_scale = double(w) / double(tex_w);
   const double full_tex_width = d.tex_width * width_scale;
   const double left_offset = d.left * width_scale;

   const int src_x = (int(color) * d.tex_width);
   const int dest_x = int(x - left_offset) - 1;
   const int dest_w = int(full_tex_width);

   const int tex_h = d.bottom - d.top;
   const double height_scale = double(h) / double(tex_h);
   const double full_tex_height = d.tex_height * height_scale;
   const double top_offset = d.top * height_scale;

   const int dest_y = int(y - top_offset) - 1;
   const int dest_h = int(full_tex_height);

   renderer.DrawStretchedTga(tex, dest_x, dest_y, dest_w, dest_h, src_x, 0, d.tex_width, d.tex_height);
}

void KeyboardDisplay::DrawBlackKeys(Renderer &renderer, const Tga *tex, bool active_only, int white_key_count, int white_width,
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

            // In this case, MissedNote isn't actually MissedNote.  In the black key
            // texture we use this value (which doesn't make any sense in this context)
            // as the default "Black" color.
            TrackColor c = MissedNote;
            if (active) c = find_result->second;

            if (!active_only || (active_only && active))
            {
               const int start_x = i * (white_width + key_space) + x_offset + black_offset;
               DrawBlackKey(renderer, tex, BlackKeyDimensions, start_x, y_offset, black_width, black_height, c);
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

   char current_white = GetStartingNote() - 1;
   int current_octave = GetStartingOctave() + 1;
   for (int i = 0; i < key_count + 1; ++i)
   {
      const int key_x = i * (key_width + key_space) + x_offset - 1;

      int guide_thickness = 1;
      Color guide_color = thin;

      bool draw_guide = true;
      switch (current_white)
      {
      case 'C':
      case 'D':
         guide_color = thin;
         break;

      case 'F':
      case 'G':
      case 'A':
         guide_color = thick;
         guide_thickness = 2;
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

void KeyboardDisplay::DrawNote(Renderer &renderer, const Tga *tex, const NoteTexDimensions &tex_dimensions, int x, int y, int w, int h, int color_id) const
{
   const NoteTexDimensions &d = tex_dimensions;

   // Width is super-easy
   const int tex_note_w = d.right - d.left;

   const double width_scale = double(w) / double(tex_note_w);
   const double full_tex_width = d.tex_width * width_scale;
   const double left_offset = d.left * width_scale;

   const int src_x = (color_id * d.tex_width);
   const int dest_x = int(x - left_offset);
   const int dest_w = int(full_tex_width);

   // Now we draw the note in three sections:
   // - Crown (fixed (relative) height)
   // - Middle (variable height)
   // - Heel (fixed (relative) height)

   // Force the note to be at least as large as the crown + heel height
   const int crown_h = d.crown_end - d.crown_start;
   const int heel_h = d.heel_end - d.heel_start;
   if (h < crown_h + heel_h)
   {
      const int diff = (crown_h + heel_h) - h;
      h += diff;
      y -= diff;
   }

   // We actually use the width scale in height calculations
   // to keep the proportions fixed.
   const double crown_start_offset = d.crown_start * width_scale;
   const double crown_end_offset = d.crown_end * width_scale;
   const double heel_height = double(d.heel_end - d.heel_start) * width_scale;
   const double bottom_height = double(d.tex_height - d.heel_end) * width_scale;

   const int dest_y1 = int(y - crown_start_offset);
   const int dest_y2 = int(dest_y1 + crown_end_offset);
   const int dest_y3 = int((y+h) - heel_height);
   const int dest_y4 = int(dest_y3 + bottom_height);

   renderer.DrawStretchedTga(tex, dest_x, dest_y1, dest_w, dest_y2 - dest_y1, src_x, 0, d.tex_width, d.crown_end);
   renderer.DrawStretchedTga(tex, dest_x, dest_y2, dest_w, dest_y3 - dest_y2, src_x, d.crown_end, d.tex_width, d.heel_start - d.crown_end);
   renderer.DrawStretchedTga(tex, dest_x, dest_y3, dest_w, dest_y4 - dest_y3, src_x, d.heel_start, d.tex_width, d.tex_height - d.heel_start);
}


void KeyboardDisplay::DrawNotePass(Renderer &renderer, const Tga *tex_white, const Tga *tex_black, int white_width,
   int key_space, int black_width, int black_offset, int x_offset, int y, int y_offset, int y_roll_under, 
   const TranslatedNoteSet &notes, microseconds_t show_duration, microseconds_t current_time,
   const std::vector<TrackProperties> &track_properties) const
{
   // Shiny music domain knowledge
   const static unsigned int NotesPerOctave = 12;
   const static unsigned int WhiteNotesPerOctave = 7;
   const static bool IsBlackNote[12] = { false, true,  false, true,  false, false,
                                         true,  false, true,  false, true,  false };

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

   // This array describes how to "stack" notes in a single place.  The IsBlackNote array
   // then tells which one should be shifted slightly to the right
   const static int NoteToWhiteNoteOffset[12] = { 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -5, -5 };

   const static int MinNoteHeight = 3;

   bool drawing_black = false;
   for (int toggle = 0; toggle < 2; ++toggle)
   {
      for (TranslatedNoteSet::const_iterator i = notes.begin(); i != notes.end(); ++i)
      {
         // This list is sorted by note start time.  The moment we encounter
         // a note scrolled off the window, we're done drawing
         if (i->start > current_time + show_duration) break;

         const TrackMode mode = track_properties[i->track_id].mode;
         if (mode == ModeNotPlayed) continue;
         if (mode == ModePlayedButHidden) continue;

         const int octave = (i->note_id / NotesPerOctave) - GetStartingOctave();
         const int octave_base = i->note_id % NotesPerOctave;
         const int stack_offset = NoteToWhiteNoteOffset[octave_base];
         const bool is_black = IsBlackNote[octave_base];

         if (drawing_black != is_black) continue;

         const int octave_offset = (max(octave - 1, 0) * WhiteNotesPerOctave);
         const int inner_octave_offset = (octave_base + stack_offset);
         const int generalized_black_offset = (is_black?black_offset:0);

         const double scaling_factor = static_cast<double>(y_offset) / static_cast<double>(show_duration);

         const long long roll_under = static_cast<int>(y_roll_under / scaling_factor);
         const long long adjusted_start = max(i->start - current_time, -roll_under);
         const long long adjusted_end   = max(i->end   - current_time, 0);
         if (adjusted_end < adjusted_start) continue;

         // Convert our times to pixel coordinates
         const int y_end   = y - static_cast<int>(adjusted_start * scaling_factor) + y_offset;
         const int y_start = y - static_cast<int>(adjusted_end   * scaling_factor) + y_offset;

         const int start_x = (octave_offset + inner_octave_offset + keyboard_type_offset) * (white_width + key_space)
            + generalized_black_offset + x_offset;

         RECT outline_rect = { start_x - 1, y_start, start_x + (is_black?black_width:white_width) + 1, y_end };

         // Force a note to be a minimum height at all times
         // except when scrolling off underneath the keyboard and
         // coming in from the top of the screen.
         const bool hitting_bottom = (adjusted_start + current_time != i->start);
         const bool hitting_top    = (adjusted_end   + current_time != i->end);
         if (!hitting_bottom && !hitting_top)
         {
            while ( (outline_rect.bottom - outline_rect.top) < MinNoteHeight) outline_rect.bottom++;
         }

         const TrackColor color = track_properties[i->track_id].color;
         const int &brush_id = (i->state == UserMissed ? MissedNote : color);

         if (drawing_black)
         {
            DrawNote(renderer, tex_black, BlackNoteDimensions, outline_rect.left, outline_rect.top, outline_rect.right - outline_rect.left,
               outline_rect.bottom - outline_rect.top, brush_id);
         }
         else
         {
            DrawNote(renderer, tex_white, WhiteNoteDimensions, outline_rect.left, outline_rect.top, outline_rect.right - outline_rect.left,
               outline_rect.bottom - outline_rect.top, brush_id);
         }
      }

      drawing_black = !drawing_black;
   }
}

void KeyboardDisplay::SetKeyActive(const string &key_name, bool active, TrackColor color)
{
   if (active) m_active_keys[key_name] = color;
   else m_active_keys.erase(key_name);
}
