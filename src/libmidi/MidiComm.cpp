// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "MidiEvent.h"
#include "MidiComm.h"
#include "MidiUtil.h"

#include <string>
#include <sstream>
using namespace std;

#include <Windows.h>

void midi_check(MMRESULT ret)
{
   if (ret == MMSYSERR_NOERROR) return;

   switch (ret)
   {
   case MIDIERR_NODEVICE:     throw MidiError_MM_NoDevice;
   case MMSYSERR_ALLOCATED:   throw MidiError_MM_AlreadyAlocated;
   case MMSYSERR_BADDEVICEID: throw MidiError_MM_BadDeviceID;
   case MMSYSERR_INVALPARAM:  throw MidiError_MM_InvalidParameter;
   case MMSYSERR_NODRIVER:    throw MidiError_MM_NoDriver;
   case MMSYSERR_NOMEM:       throw MidiError_MM_NoMemory;
   default:                   throw MidiError_MM_Unknown;
   }
}

void CALLBACK MidiInputCallback(HMIDIIN handle_in, UINT msg, DWORD_PTR instance, DWORD p1, DWORD p2)
{
   reinterpret_cast<MidiCommIn*>(instance)->InputCallback(msg, p1, p2);
}

MidiCommDescriptionList MidiCommIn::GetDeviceList()
{
   MidiCommDescriptionList devices;

   unsigned int dev_count = midiInGetNumDevs();
   for (unsigned int i = 0; i < dev_count; ++i)
   {
      MIDIINCAPS dev;
      midi_check(midiInGetDevCaps(i, &dev, sizeof(MIDIINCAPS)));

      MidiCommDescription d;
      d.id = i;
      d.name = dev.szPname;

      devices.push_back(d);
   }

   return devices;
}

MidiCommIn::MidiCommIn(unsigned int device_id)
{
   m_description = GetDeviceList()[device_id];

   InitializeCriticalSection(&m_buffer_mutex);

   midi_check(midiInOpen(&m_input_device, device_id,
      reinterpret_cast<DWORD_PTR>(MidiInputCallback),
      reinterpret_cast<DWORD_PTR>(this),
      CALLBACK_FUNCTION));
   
   midi_check(midiInStart(m_input_device));
}

MidiCommIn::~MidiCommIn()
{
   midi_check(midiInStop(m_input_device));
   midi_check(midiInReset(m_input_device));
   midi_check(midiInClose(m_input_device));

   DeleteCriticalSection(&m_buffer_mutex);
}

// This is only called by the callback function.  The reason this
// is public (and the callback isn't a static member) is to keep the
// HMIDIIN definition out of this classes header.
void MidiCommIn::InputCallback(unsigned int msg, unsigned long p1, unsigned long p2)
{
   switch (msg)
   {
   case MIM_DATA:
      {
         unsigned char status = LOBYTE(LOWORD(p1));
         unsigned char byte1  = HIBYTE(LOWORD(p1));
         unsigned char byte2  = LOBYTE(HIWORD(p1));
         MidiEvent ev = MidiEvent::Build(MidiEventSimple(status, byte1, byte2));

         EnterCriticalSection(&m_buffer_mutex);
         m_event_buffer.push(ev);
         LeaveCriticalSection(&m_buffer_mutex);
      }
      break;


   case MIM_OPEN:
   case MIM_CLOSE:
      // Ignore
      break;

   case MIM_LONGDATA:
   case MIM_ERROR:
   case MIM_LONGERROR:
   case MIM_MOREDATA:
      throw MidiError(MidiError_UnexpectedInput);
   }
}

void MidiCommIn::Reset()
{
   EnterCriticalSection(&m_buffer_mutex);
   while (!m_event_buffer.empty()) m_event_buffer.pop();
   LeaveCriticalSection(&m_buffer_mutex);
}

bool MidiCommIn::KeepReading() const
{
   bool buffer_empty;

   EnterCriticalSection(&m_buffer_mutex);
   buffer_empty = m_event_buffer.empty();
   LeaveCriticalSection(&m_buffer_mutex);

   return (!buffer_empty);
}

MidiEvent MidiCommIn::Read()
{
   MidiEvent ev(MidiEvent::NullEvent());
   bool buffer_empty;

   EnterCriticalSection(&m_buffer_mutex);
   buffer_empty = m_event_buffer.empty();
   if (!buffer_empty)
   {
      ev = m_event_buffer.front();
      m_event_buffer.pop();
   }
   LeaveCriticalSection(&m_buffer_mutex);

   if (buffer_empty) throw MidiError(MidiError_NoInputAvailable);
   return ev;
}

MidiCommDescriptionList MidiCommOut::GetDeviceList()
{
   MidiCommDescriptionList devices;

   unsigned int dev_count = midiOutGetNumDevs();
   for (unsigned int i = 0; i < dev_count; ++i)
   {
      MIDIOUTCAPS dev;
      midi_check(midiOutGetDevCaps(i, &dev, sizeof(MIDIOUTCAPS)));

      MidiCommDescription d;
      d.id = i;
      d.name = dev.szPname;

      devices.push_back(d);
   }

   return devices;
}

MidiCommOut::MidiCommOut(unsigned int device_id)
{
   m_description = GetDeviceList()[device_id];

   midi_check(midiOutOpen(&m_output_device, device_id, 0, 0, CALLBACK_NULL));
}

MidiCommOut::~MidiCommOut()
{
   midi_check(midiOutReset(m_output_device));
   midi_check(midiOutClose(m_output_device));
}

void MidiCommOut::Write(const MidiEvent &out)
{
   MidiEventSimple simple;
   if (out.GetSimpleEvent(&simple))
   {
      // You could use a bunch of MAKELONG(MAKEWORD(lo,hi), MAKEWORD(lo,hi)) stuff here, but
      // this is easier to read and likely faster.
      unsigned long message = simple.status | (simple.byte1 << 8) | (simple.byte2 << 16);

      midi_check(midiOutShortMsg(m_output_device, message));
   }
}

void MidiCommOut::Reset()
{
   midi_check(midiOutReset(m_output_device));
}
