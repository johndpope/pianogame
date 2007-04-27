// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __MIDI_COMM_H
#define __MIDI_COMM_H

#include <string>
#include <vector>
#include <queue>

#ifdef WIN32
#include <Windows.h>
#endif

#include "MidiEvent.h"

struct MidiCommDescription
{
   unsigned int id;
   std::wstring name;
};

typedef std::vector<MidiCommDescription> MidiCommDescriptionList;
typedef std::queue<MidiEvent> MidiEventQueue;

// Once you create a MidiCommIn object, MIDI events are read continuously
// in a separate thread and stored in a buffer.  Use the Read() function
// to grab one event at a time from the buffer.
class MidiCommIn
{
public:
   static MidiCommDescriptionList GetDeviceList();

   // device_id is obtained from GetDeviceList()
   MidiCommIn(unsigned int device_id);
   ~MidiCommIn();

   MidiCommDescription GetDeviceDescription() const { return m_description; }

   // Returns the next buffered input event.  Use KeepReading() (usually in
   // a while loop) to see if you should call this function.  If called when
   // KeepReading() is false, this will throw MidiError_NoInputAvailable.
   MidiEvent Read();

   // Discard events from the input buffer
   void Reset();

   // Returns whether the input device has more buffered events.
   bool KeepReading() const;

   // Do Not Use
   void InputCallback(unsigned int msg, unsigned long p1, unsigned long p2);

private:
   MidiCommDescription m_description;

   MidiEventQueue m_event_buffer;

#ifdef WIN32
   HMIDIIN m_input_device;
   mutable CRITICAL_SECTION m_buffer_mutex;
#endif

};

class MidiCommOut
{
public:
   static MidiCommDescriptionList GetDeviceList();

   // device_id is obtained from GetDeviceList()
   MidiCommOut(unsigned int device_id);
   ~MidiCommOut();

   MidiCommDescription GetDeviceDescription() const { return m_description; }

   // Send a single event out to the device.
   void Write(const MidiEvent &out);

   // Turns all notes off and resets all controllers
   void Reset();

private:
   MidiCommDescription m_description;

#ifdef WIN32
   HMIDIOUT m_output_device;
#endif

};

#endif
