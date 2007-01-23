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
      notes_user_actually_played(0), longest_combo(0), score(0) { }

   int total_note_count;
   int notes_user_could_have_played;
   int notes_user_actually_played;

   int longest_combo;
   double score;

};

struct SharedState
{
   SharedState(const std::wstring &title)
      : midi(0), midi_out(0), midi_in(0), song_title(title)
   { }

   Midi *midi;
   MidiCommOut *midi_out;
   MidiCommIn *midi_in;

   SongStatistics stats;

   std::vector<TrackProperties> track_properties;
   const std::wstring song_title;
};

#endif