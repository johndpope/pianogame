// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __KEYBOARDDISPLAY_H
#define __KEYBOARDDISPLAY_H

#include <map>
#include <vector>
#include <string>
#include <windows.h>

#include "Image.h"
#include "libmidi\Note.h"

enum KeyboardSize
{
   KeyboardSize61,
   KeyboardSize76,
   KeyboardSize88
};

enum TrackColor;

typedef std::map<std::string, TrackColor> KeyNames;
struct TrackProperties;

class KeyboardDisplay
{
public:
   const static unsigned long long NoteWindowLength = 300000;

   KeyboardDisplay(KeyboardSize size, int pixelWidth, int pixelHeight);

   void Draw(HDC hdc, int x, int y, const TranslatedNoteSet &notes,
      unsigned long long show_duration, unsigned long long current_time,
      const std::vector<TrackProperties> &track_properties);

   void SetKeyActive(const std::string &key_name, bool active, TrackColor color);

   void ResetActiveKeys() { m_active_keys.clear(); }

private:

   bool m_background_initialized;
   Image m_cached_background;

   void DrawWhiteKeys(HDC hdc, bool active_only, int key_count, int key_width, int key_height, 
      int key_space, int x_offset, int y_offset) const;

   void DrawBlackKeys(HDC hdc, bool active_only,int white_key_count, int white_width, int white_height,
      int black_width, int black_height, int key_space, int x_offset, int y_offset, int black_offset) const;

   void DrawGuides(HDC hdc, int key_count, int key_width, int key_space,
      int x_offset, int y, int y_offset) const;

   void DrawNotes(HDC hdc, int white_width, int key_space, int black_width, int black_offset,
      int x_offset, int y, int y_offset, const TranslatedNoteSet &notes,
      unsigned long long show_duration, unsigned long long current_time,
      const std::vector<TrackProperties> &track_properties) const;

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
