// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __MIDI_H
#define __MIDI_H

#include <iostream>
#include <vector>

#include "Note.h"
#include "MidiTrack.h"

class MidiError;
class MidiEvent;

typedef std::vector<MidiTrack> MidiTrackList;

typedef std::vector<MidiEvent> MidiEventList;
typedef std::vector<std::pair<size_t, MidiEvent> > MidiEventListWithTrackId;

// NOTE: KNOWN ISSUE: A Midi file that begins with a slow tempo, then changes
//       tempo again before the first note is played has some issues.

// NOTE: This library's MIDI loading and handling is destructive.  Perfect
//       1:1 serialization routines will not be possible without quite a
//       bit of additional work.
class Midi
{
public:
   static Midi ReadFromFile(const std::wstring &filename);
   static Midi ReadFromStream(std::istream &stream);

   const std::vector<MidiTrack> &Tracks() const { return m_tracks; }

   const TranslatedNoteSet &Notes() const { return m_translated_notes; }

   MidiEventListWithTrackId Update(unsigned long long delta_microseconds);

   void Reset(unsigned long long lead_in_microseconds, unsigned long long lead_out_microseconds);

   // This is O(n) where n is the number of tempo changes (across all tracks) in
   // the song up to the specified time.  Tempo changes are usually a small number.
   // (Almost always 0 or 1, going up to maybe 30-100 in rare cases.)
   //
   // This includes lead-in.  (It can also double for calculating base song length
   // only because just before the CalculateSongLength() call, the lead-in is reset
   // to 0.  In that way, this is kind of a double-duty function.
   unsigned long long GetEventPulseInMicroseconds(unsigned long event_pulses) const;

   unsigned long long GetFirstNoteMicroseconds() const { return GetEventPulseInMicroseconds(m_first_note_pulse); }
   unsigned long long GetSongPositionInMicroseconds() const { return m_microsecond_song_position; }
   unsigned long long GetSongLengthInMicroseconds() const;

   // Use this to find out when a song is over vs. AggregateEventsRemain() because
   // this takes into account song lead-in and lead-out time.  AggregateEventsRemain()
   // will return 0 throughout the entire lead-out period.
   double GetSongPercentageComplete() const;

   unsigned int AggregateEventsRemain() const;
   unsigned int AggregateEventCount() const;

   unsigned int AggregateNotesRemain() const;
   unsigned int AggregateNoteCount() const;

private:
   const static unsigned long DefaultBPM = 120;
   const static unsigned long long OneMinuteInMicroseconds = 60000000;
   const static unsigned long long DefaultUSTempo = OneMinuteInMicroseconds / DefaultBPM;

   Midi(): m_first_note_pulse(0), m_initialized(false), m_ever_translated(false) { Reset(0, 0); }

   bool m_initialized;
   bool m_ever_translated;

   void BuildTempoTrack();
   void CalculateSongLength();
   void CalculateFirstNotePulse();
   void TranslateNotes(const NoteSet &notes);

   unsigned long long ConvertPulsesToMicroseconds(unsigned long pulses, unsigned long long tempo) const;

   TranslatedNoteSet m_translated_notes;

   // Position includes lead-in/lead-out.
   unsigned long long m_microsecond_song_position;
   unsigned long long m_microsecond_base_song_length;

   unsigned long long m_microsecond_lead_in;
   unsigned long long m_microsecond_lead_out;

   unsigned long long m_us_tempo;
   double m_update_pulse_remainder;

   double m_playback_speed;

   unsigned long m_first_note_pulse;

   unsigned short m_pulses_per_quarter_note;
   MidiTrackList m_tracks;
};

#endif
