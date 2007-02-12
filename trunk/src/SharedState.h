// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __SHARED_STATE_H
#define __SHARED_STATE_H

#include <string>
#include <vector>
#include "TrackProperties.h"

class Midi;
class MidiCommOut;
class MidiCommIn;

struct SongStatistics
{
   SongStatistics() : total_note_count(0), notes_user_could_have_played(0),
      notes_user_actually_played(0), stray_notes(0), total_notes_user_pressed(0),
      longest_combo(0), score(0) { }

   int total_note_count;
   int notes_user_could_have_played;
   int notes_user_actually_played;

   int stray_notes;
   int total_notes_user_pressed;

   int longest_combo;
   double score;

};

struct SharedState
{
   SharedState()
      : midi(0), midi_out(0), midi_in(0)
   { }

   Midi *midi;
   MidiCommOut *midi_out;
   MidiCommIn *midi_in;

   SongStatistics stats;

   std::vector<TrackProperties> track_properties;
   std::wstring song_title;
};

#endif