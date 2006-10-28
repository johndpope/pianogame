// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __MIDI_TRACK_H
#define __MIDI_TRACK_H

#include <vector>
#include <iostream>

#include "Note.h"
#include "MidiEvent.h"
#include "MidiUtil.h"

class MidiEvent;

typedef std::vector<MidiEvent> MidiEventList;
typedef std::vector<unsigned long> MidiEventPulsesList;

class MidiTrack
{
public:
   static MidiTrack ReadFromStream(std::istream &stream);
   static MidiTrack CreateBlankTrack() { return MidiTrack(); }

   MidiEventList &Events() { return m_events; }
   MidiEventPulsesList &EventPulses() { return m_event_pulses; }

   const MidiEventList &Events() const { return m_events; }
   const MidiEventPulsesList &EventPulses() const { return m_event_pulses; }

   const std::wstring InstrumentName() const { return InstrumentNames[m_instrument_id]; }
   bool IsPercussion() const { return m_instrument_id == InstrumentIdPercussion; }

   const NoteSet &Notes() const { return m_note_set; }

   void SetTrackId(size_t track_id);

   // Reports whether this track contains any Note-On MIDI events
   // (vs. just being an information track with a title or copyright)
   bool hasNotes() const { return (m_note_set.size() > 0); }

   void Reset();
   MidiEventList Update(unsigned long delta_pulses);

   unsigned int AggregateEventsRemain() const { return static_cast<unsigned int>(m_events.size() - (m_last_event + 1)); }
   unsigned int AggregateEventCount() const { return static_cast<unsigned int>(m_events.size()); }

   unsigned int AggregateNotesRemain() const { return m_notes_remaining; }
   unsigned int AggregateNoteCount() const { return static_cast<unsigned int>(m_note_set.size()); }

private:
   MidiTrack() : m_instrument_id(0) { Reset(); }

   void BuildNoteSet();
   void DiscoverInstrument();

   MidiEventList m_events;
   MidiEventPulsesList m_event_pulses;

   NoteSet m_note_set;

   int m_instrument_id;

   unsigned long m_running_pulses;
   unsigned long m_last_event_pulses;
   long m_last_event;

   unsigned int m_notes_remaining;
};

#endif
