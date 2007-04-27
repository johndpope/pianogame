// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __KEYBOARDDISPLAY_H
#define __KEYBOARDDISPLAY_H

#include <map>
#include <vector>
#include <string>

#include "libmidi/Note.h"
#include "libmidi/MidiTypes.h"

enum KeyboardSize
{
   KeyboardSize37,
   KeyboardSize49,
   KeyboardSize61,
   KeyboardSize76,
   KeyboardSize88
};

enum TrackColor;

typedef std::map<std::string, TrackColor> KeyNames;
struct TrackProperties;

class Renderer;
class Tga;

class KeyboardDisplay
{
public:
   const static microseconds_t NoteWindowLength = 330000;

   KeyboardDisplay(KeyboardSize size, int pixelWidth, int pixelHeight);

   void Draw(Renderer &renderer, const Tga *key_tex[3], const Tga *note_tex[4], int x, int y,
      const TranslatedNoteSet &notes, microseconds_t show_duration, microseconds_t current_time,
      const std::vector<TrackProperties> &track_properties);

   void SetKeyActive(const std::string &key_name, bool active, TrackColor color);

   void ResetActiveKeys() { m_active_keys.clear(); }

private:

   struct NoteTexDimensions
   {
      int tex_width;
      int tex_height;

      int left;
      int right;

      int crown_start;
      int crown_end;

      int heel_start;
      int heel_end;
   };
   const static NoteTexDimensions WhiteNoteDimensions;
   const static NoteTexDimensions BlackNoteDimensions;

   struct KeyTexDimensions
   {
      int tex_width;
      int tex_height;

      int left;
      int right;

      int top;
      int bottom;
   };
   const static KeyTexDimensions BlackKeyDimensions;


   void DrawWhiteKeys(Renderer &renderer, bool active_only, int key_count, int key_width, int key_height, 
      int key_space, int x_offset, int y_offset) const;

   void DrawBlackKeys(Renderer &renderer, const Tga *tex, bool active_only,int white_key_count, int white_width,
      int black_width, int black_height, int key_space, int x_offset, int y_offset, int black_offset) const;

   void DrawRail(Renderer &renderer, const Tga *tex, int x, int y, int width) const;
   void DrawShadow(Renderer &renderer, const Tga *tex, int x, int y, int width) const;

   void DrawGuides(Renderer &renderer, int key_count, int key_width, int key_space,
      int x_offset, int y, int y_offset) const;

   void DrawNotePass(Renderer &renderer, const Tga *tex_white, const Tga *tex_black, int white_width,
      int key_space, int black_width, int black_offset, int x_offset, int y, int y_offset, int y_roll_under,
      const TranslatedNoteSet &notes, microseconds_t show_duration, microseconds_t current_time,
      const std::vector<TrackProperties> &track_properties) const;

   // This takes the rectangle where the actual note block should appear and transforms
   // it to the multi-quad (with relatively complicated texture coordinates) using the
   // passed-in texture descriptor, and then draws the result
   void DrawNote(Renderer &renderer, const Tga *tex, const NoteTexDimensions &tex_dimensions, int x, int y, int w, int h, int color_id) const;

   // This works very much like DrawNote
   void DrawBlackKey(Renderer &renderer, const Tga *tex, const KeyTexDimensions &tex_dimensions, int x, int y, int w, int h, TrackColor color) const;

   // Retrieves which white-key a piano with the given key count
   // will start with on the far left side
   char GetStartingNote() const;

   // Retrieves which octave a piano with the given key count
   // will start with on the far left side
   int GetStartingOctave() const;

   // Retrieves the number of white keys a piano with the given
   // key count will contain
   int GetWhiteKeyCount() const;

   KeyboardSize m_size;
   KeyNames m_active_keys;

   int m_width;
   int m_height;
};

#endif
