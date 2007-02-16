// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "Midi.h"
#include "MidiEvent.h"
#include "MidiTrack.h"
#include "MidiUtil.h"

#include <fstream>
#include <map>

using namespace std;

Midi Midi::ReadFromFile(const wstring &filename)
{
   fstream file(reinterpret_cast<const wchar_t*>((filename).c_str()), ios::in|ios::binary);
   if (!file.good()) throw MidiError(MidiError_BadFilename);

   Midi m;

   try
   {
      m = ReadFromStream(file);
   }
   catch (const MidiError &e)
   {
      // Close our file resource before handing the error up
      file.close();
      throw e;
   }

   return m;
}

Midi Midi::ReadFromStream(istream &stream)
{
   Midi m;

   // header_id is always "MThd" by definition
   const static string MidiFileHeader = "MThd";
   const static string RiffFileHeader = "RIFF";

   // I could use (MidiFileHeader.length() + 1), but then this has to be
   // dynamically allocated.  More hassle than it's worth.  MIDI is well
   // defined and will always have a 4-byte header.  We use 5 so we get
   // free null termination.
   char           header_id[5] = { 0, 0, 0, 0, 0 };
   unsigned long  header_length;
   unsigned short format;
   unsigned short track_count;
   unsigned short time_division;

   stream.read(header_id, static_cast<streamsize>(MidiFileHeader.length()));
   string header(header_id);
   if (header != MidiFileHeader)
   {
      if (header != RiffFileHeader) throw MidiError(MidiError_UnknownHeaderType);
      else
      {
         // We know how to support RIFF files
         unsigned long throw_away;
         stream.read(reinterpret_cast<char*>(&throw_away), sizeof(unsigned long)); // RIFF length
         stream.read(reinterpret_cast<char*>(&throw_away), sizeof(unsigned long)); // "RMID"
         stream.read(reinterpret_cast<char*>(&throw_away), sizeof(unsigned long)); // "data"
         stream.read(reinterpret_cast<char*>(&throw_away), sizeof(unsigned long)); // data size

         // Call this recursively, without the RIFF header this time
         return ReadFromStream(stream);
      }
   }

   stream.read(reinterpret_cast<char*>(&header_length), sizeof(unsigned long));
   stream.read(reinterpret_cast<char*>(&format),        sizeof(unsigned short));
   stream.read(reinterpret_cast<char*>(&track_count),   sizeof(unsigned short));
   stream.read(reinterpret_cast<char*>(&time_division), sizeof(unsigned short));

   if (stream.fail()) throw MidiError(MidiError_NoHeader);

   // Chunk Size is always 6 by definition
   const static unsigned int MidiFileHeaderChunkLength = 6;

   header_length = swap32(header_length);
   if (header_length != MidiFileHeaderChunkLength)
   {
      throw MidiError(MidiError_BadHeaderSize);
   }

   const static int MidiFormat0 = 0;
   const static int MidiFormat1 = 1;
   const static int MidiFormat2 = 2;

   format = swap16(format);
   if (format == MidiFormat2)
   {
      // MIDI 0: All information in 1 track
      // MIDI 1: Multiple tracks intended to be played simultaneously
      // MIDI 2: Multiple tracks intended to be played separately
      //
      // We do not support MIDI 2 at this time
      throw MidiError(MidiError_Type2MidiNotSupported);
   }

   track_count = swap16(track_count);
   if (format == 0 && track_count != 1)
   {
      // MIDI 0 has only 1 track by definition
      throw MidiError(MidiError_BadType0Midi);
   }

   // Time division can be encoded two ways based on a bit-flag:
   // - pulses per quarter note (15-bits)
   // - SMTPE frames per second (7-bits for SMPTE frame count and 8-bits for clock ticks per frame)
   time_division = swap16(time_division);
   bool in_smpte = ((time_division & 0x8000) != 0);

   if (in_smpte)
   {
      throw MidiError(MidiError_SMTPETimingNotImplemented);
   }

   // We ignore the possibility of SMPTE timing, so we can
   // use the time division value directly as PPQN.
   m.m_pulses_per_quarter_note = time_division;

   // Read in our tracks
   for (int i = 0; i < track_count; ++i)
   {
      m.m_tracks.push_back(MidiTrack::ReadFromStream(stream));
   }

   m.Reset(0, 0);
   m.BuildTempoTrack();

   m.m_initialized = true;

   m.CalculateSongLength();
   m.CalculateFirstNotePulse();

   // Tell our tracks their IDs
   for (int i = 0; i < track_count; ++i)
   {
      m.m_tracks[i].SetTrackId(i);
   }

   return m;
}

// NOTE: This is required for much of the other functionality provided
// by this class, however, this causes a destructive change in the way
// the MIDI is represented internally which means we can never save the
// file back out to disk exactly as we loaded it.
//
// This adds an extra track dedicated to tempo change events.  Tempo events
// are extracted from every other track and placed in the new one.
//
// This allows quick(er) calculation of wall-clock event times
void Midi::BuildTempoTrack()
{
   // This map will help us get rid of duplicate events if
   // the tempo is specified in every track (as is common).
   //
   // It also does sorting for us so we can just copy the
   // events right over to the new track.
   std::map<unsigned long, MidiEvent> tempo_events;

   // Run through each track looking for tempo events
   for (MidiTrackList::iterator t = m_tracks.begin(); t != m_tracks.end(); ++t)
   {
      for (size_t i = 0; i < t->Events().size(); ++i)
      {
         MidiEvent ev = t->Events()[i];
         unsigned long ev_pulses = t->EventPulses()[i];

         if (ev.Type() == MidiEventType_Meta && ev.MetaType() == MidiMetaEvent_TempoChange)
         {
            // Pull tempo event out of both lists
            //
            // Vector is kind of a hassle this way -- we have to
            // walk an iterator to that point in the list because
            // erase MUST take an iterator... but erasing from a
            // list invalidates iterators.  bleah.
            MidiEventList::iterator event_to_erase = t->Events().begin();
            MidiEventPulsesList::iterator event_pulse_to_erase = t->EventPulses().begin();
            for (size_t j = 0; j < i; ++j) { ++event_to_erase; ++event_pulse_to_erase; }

            t->Events().erase(event_to_erase);
            t->EventPulses().erase(event_pulse_to_erase);

            // Adjust next event's delta time
            if (t->Events().size() > i)
            {
               // (We just erased the element at i, so
               // now i is pointing to the next element)
               unsigned long next_dt = t->Events()[i].GetDeltaPulses();

               t->Events()[i].SetDeltaPulses(ev.GetDeltaPulses() + next_dt);
            }

            // We have to roll i back for the next loop around
            --i;

            // Insert our newly stolen event into the auto-sorting map
            tempo_events[ev_pulses] = ev;
         }
      }
   }

   // Create a new track (always the last track in the track list)
   m_tracks.push_back(MidiTrack::CreateBlankTrack());

   MidiEventList &tempo_track_events = m_tracks[m_tracks.size()-1].Events();
   MidiEventPulsesList &tempo_track_event_pulses = m_tracks[m_tracks.size()-1].EventPulses();

   // Copy over all our tempo events
   unsigned long previous_absolute_pulses = 0;
   for (std::map<unsigned long, MidiEvent>::const_iterator i = tempo_events.begin(); i != tempo_events.end(); ++i)
   {
      unsigned long absolute_pulses = i->first;
      MidiEvent ev = i->second;

      // Reset each of their delta times while we go
      ev.SetDeltaPulses(absolute_pulses - previous_absolute_pulses);
      previous_absolute_pulses = absolute_pulses;

      // Add them to the track
      tempo_track_event_pulses.push_back(absolute_pulses);
      tempo_track_events.push_back(ev);
   }
}

void Midi::CalculateSongLength()
{
   m_microsecond_base_song_length = 0;

   // Check the last event in each track to see which one happens last
   for (MidiTrackList::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
   {
      size_t event_count = i->EventPulses().size();
      if (event_count == 0) continue;

      unsigned long event_pulses = i->EventPulses().back();
      microseconds_t event_microseconds = GetEventPulseInMicroseconds(event_pulses);

      if (event_microseconds > m_microsecond_base_song_length) m_microsecond_base_song_length = event_microseconds;
   }
}

void Midi::CalculateFirstNotePulse()
{
   // Find the very last value it could ever possibly be, to start with
   for (MidiTrackList::const_iterator t = m_tracks.begin(); t != m_tracks.end(); ++t)
   {
      if (t->EventPulses().size() == 0) continue;
      unsigned long pulses = t->EventPulses().back();

      if (pulses > m_first_note_pulse) m_first_note_pulse = pulses;
   }

   // Now run through each event in each track looking for the very
   // first note_on event
   for (MidiTrackList::const_iterator t = m_tracks.begin(); t != m_tracks.end(); ++t)
   {
      for (size_t ev_id = 0; ev_id < t->Events().size(); ++ev_id)
      {
         if (t->Events()[ev_id].Type() == MidiEventType_NoteOn)
         {
            unsigned long note_pulse = t->EventPulses()[ev_id];

            if (note_pulse < m_first_note_pulse) m_first_note_pulse = note_pulse;

            // We found the first note event in this
            // track.  No need to keep searching.
            break;
         }
      }
   }

}

microseconds_t Midi::ConvertPulsesToMicroseconds(unsigned long pulses, microseconds_t tempo) const
{
   // Here's what we have to work with:
   //   pulses is given
   //   tempo is given (units of microseconds/quarter_note)
   //   (pulses/quarter_note) is given as a constant in this object file
   const double quarter_notes = static_cast<double>(pulses) / static_cast<double>(m_pulses_per_quarter_note);
   const double microseconds = quarter_notes * static_cast<double>(tempo);

   return static_cast<microseconds_t>(microseconds);
}

microseconds_t Midi::GetEventPulseInMicroseconds(unsigned long event_pulses) const
{
   if (!m_initialized) return 0;

   if (m_tracks.size() == 0) return 0;
   const MidiTrack &tempo_track = m_tracks.back();

   microseconds_t running_result = m_microsecond_lead_in;

   bool hit = false;
   unsigned long last_tempo_event_pulses = 0;
   microseconds_t running_tempo = DefaultUSTempo;
   for (size_t i = 0; i < tempo_track.Events().size(); ++i)
   {
      unsigned long tempo_event_pulses = tempo_track.EventPulses()[i];

      // If the time we're asking to convert is still beyond
      // this tempo event, just add the last time slice (at
      // the previous tempo) to the running wall-clock time.
      unsigned long delta_pulses = 0;
      if (event_pulses > tempo_event_pulses)
      {
         delta_pulses = tempo_event_pulses - last_tempo_event_pulses;
      }
      else
      {
         hit = true;
         delta_pulses = event_pulses - last_tempo_event_pulses;
      }

      running_result += ConvertPulsesToMicroseconds(delta_pulses, running_tempo);

      // If the time we're calculating is before the tempo event we're
      // looking at, we're done.
      if (hit) break;

      running_tempo = tempo_track.Events()[i].GetTempoInUsPerQn();
      last_tempo_event_pulses = tempo_event_pulses;
   }

   // The requested time may be after the very last tempo event
   if (!hit)
   {
      unsigned long remaining_pulses = event_pulses - last_tempo_event_pulses;
      running_result += ConvertPulsesToMicroseconds(remaining_pulses, running_tempo);
   }

   return running_result;
}

void Midi::Reset(microseconds_t lead_in_microseconds, microseconds_t lead_out_microseconds)
{
   bool should_translate = !m_ever_translated ||
      (lead_in_microseconds != m_microsecond_lead_in || lead_out_microseconds != m_microsecond_lead_out);

   // Lead-in and lead-out are always handled absolutely in Update(),
   // so we don't need to adjust them with the playback_speed.
   m_microsecond_lead_in = lead_in_microseconds;
   m_microsecond_lead_out = lead_out_microseconds;

   m_us_tempo = DefaultUSTempo;
   m_microsecond_song_position = 0;
   m_update_pulse_remainder = 0;

   Update(GetEventPulseInMicroseconds(m_first_note_pulse) - m_microsecond_lead_in);


   if (should_translate) m_translated_notes.clear();
   for (MidiTrackList::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
   {
      i->Reset();

      // Translate each track's list of notes into the microsecond-based
      // master list.  We only need to do this if it's the first time
      // we've ever reset or if the lead-in or lead-out changed.
      if (should_translate) TranslateNotes(i->Notes());
   }

   m_ever_translated = true;
}

void Midi::TranslateNotes(const NoteSet &notes)
{
   if (!m_initialized) return;

   for (NoteSet::const_iterator i = notes.begin(); i != notes.end(); ++i)
   {
      TranslatedNote trans;
      
      trans.note_id = i->note_id;
      trans.track_id = i->track_id;
      trans.start = GetEventPulseInMicroseconds(i->start);
      trans.end = GetEventPulseInMicroseconds(i->end);

      m_translated_notes.insert(trans);
   }
}

MidiEventListWithTrackId Midi::Update(microseconds_t delta_microseconds)
{
   MidiEventListWithTrackId aggregated_events;
   if (!m_initialized) return aggregated_events;

   // Tempo changes in the middle of long updates can cause
   // skew.  Recursively call Update in tiny amounts to keep
   // the tempo more accurate throughout.
   const static microseconds_t LongestUpdate = 500000;
   if (delta_microseconds > LongestUpdate)
   {
      aggregated_events = Update(delta_microseconds - LongestUpdate);
      delta_microseconds = LongestUpdate;
   }

   // If we're still in the lead-in phase, don't pass any time to
   // the MIDI tracks
   if (m_microsecond_song_position + delta_microseconds < m_microsecond_lead_in)
   {
      m_microsecond_song_position += delta_microseconds;
      return aggregated_events;
   }
   else
   {
      // We may just be exiting the lead-in phase
      if (m_microsecond_song_position < m_microsecond_lead_in)
      {
         microseconds_t original_delta = delta_microseconds;

         // We only want to update the tracks with what's left after lead-in
         delta_microseconds = (m_microsecond_song_position + original_delta) - m_microsecond_lead_in;

         m_microsecond_song_position += original_delta;
      }
      else
      {
         m_microsecond_song_position += delta_microseconds;
      }
   }

   const double delta_quarter_notes = static_cast<double>(delta_microseconds) / static_cast<double>(m_us_tempo);

   // Roll any partial pulses from our last update into this one
   double delta_pulses = delta_quarter_notes * m_pulses_per_quarter_note;
   delta_pulses += m_update_pulse_remainder;

   // Keep the integer portion and record the remainder for next time
   unsigned long whole_pulses = static_cast<unsigned long>(delta_pulses);
   m_update_pulse_remainder = delta_pulses - whole_pulses;

   const size_t track_count = m_tracks.size();
   for (size_t i = 0; i < track_count; ++i)
   {
      MidiEventList &track_events = m_tracks[i].Update(whole_pulses);

      const size_t event_count = track_events.size();
      for (size_t j = 0; j < event_count; ++j)
      {
         aggregated_events.insert(aggregated_events.end(), make_pair<size_t, MidiEvent>(i, track_events[j]));
      }
   }

   // We have to search this list ourselves for tempo changes
   const size_t aggregated_count = aggregated_events.size();
   for (size_t i = 0; i < aggregated_count; ++i)
   {
      const MidiEvent &ev = aggregated_events[i].second;

      if (ev.Type() == MidiEventType_Meta && ev.MetaType() == MidiMetaEvent_TempoChange)
      {
         m_us_tempo = ev.GetTempoInUsPerQn();
      }
   }

   return aggregated_events;
}

microseconds_t Midi::GetSongLengthInMicroseconds() const
{
   if (!m_initialized) return 0;

   return m_microsecond_lead_in + m_microsecond_lead_out + m_microsecond_base_song_length;
}

unsigned int Midi::AggregateEventsRemain() const
{
   if (!m_initialized) return 0;

   unsigned int aggregate = 0;
   for (MidiTrackList::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
   {
      aggregate += i->AggregateEventsRemain();
   }
   return aggregate;
}

unsigned int Midi::AggregateNotesRemain() const
{
   if (!m_initialized) return 0;

   unsigned int aggregate = 0;
   for (MidiTrackList::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
   {
      aggregate += i->AggregateNotesRemain();
   }
   return aggregate;
}

unsigned int Midi::AggregateEventCount() const
{
   if (!m_initialized) return 0;

   unsigned int aggregate = 0;
   for (MidiTrackList::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
   {
      aggregate += i->AggregateEventCount();
   }
   return aggregate;
}

unsigned int Midi::AggregateNoteCount() const
{
   if (!m_initialized) return 0;

   unsigned int aggregate = 0;
   for (MidiTrackList::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
   {
      aggregate += i->AggregateNoteCount();
   }
   return aggregate;
}

double Midi::GetSongPercentageComplete() const
{
   if (!m_initialized) return 0.0;

   const double pos = static_cast<double>(m_microsecond_song_position);
   const double len = static_cast<double>(GetSongLengthInMicroseconds());

   if (len == 0) return 1.0;

   return min( (pos / len), 1.0 );
}
