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
   midi_check(midiOutClose(m_output_device));
   midi_check(midiOutOpen(&m_output_device, m_description.id, 0, 0, CALLBACK_NULL));
}



// The following Mixer routines are based off Chen Su's Audio Mixer Functions Demo
// which can be found at: http://www.codeproject.com/audio/admixer.asp
//
// NOTE: This code only supports the first audio mixer.  If a system has more than
// one sound card installed, these will only affect the "first" card.

// TODO: This could use some cleanup.  A ReasonableSynthVolume object with interesting
// constructor and destructor sounds like a great idea to be plopped down in main.cpp
// just before the game.run() call.

static LONG g_old_mute = 1;
static DWORD g_old_volume = 0;

static HMIXER g_mixer = 0;
static DWORD g_mute_control_id = 0;
static DWORD g_volume_control_id = 0;

void MidiCommOut::SetReasonableSynthesizerVolume()
{
   int mixer_count = mixerGetNumDevs();
   if (mixer_count == 0) return;

   if (mixerOpen(&g_mixer, 0, 0, 0, MIXER_OBJECTF_MIXER) != MMSYSERR_NOERROR) return;

   MIXERCAPS caps;
   if (mixerGetDevCaps((UINT_PTR)g_mixer, &caps, sizeof(MIXERCAPS)) != MMSYSERR_NOERROR) return;

   // Open the MIDI "line" of the mixer
   MIXERLINE line;
   line.cbStruct = sizeof(MIXERLINE);
   line.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER;
   if (mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(g_mixer), &line, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR) return;


   MIXERLINECONTROLS list;
   MIXERCONTROL control;
   list.cbStruct = sizeof(MIXERLINECONTROLS);
   list.dwLineID = line.dwLineID;
   list.cControls = 1;
   list.cbmxctrl = sizeof(MIXERCONTROL);
   list.pamxctrl = &control;

   // Record the "mute" control ID for the MIDI line
   list.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
   if (mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(g_mixer), &list, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return;
   g_mute_control_id = control.dwControlID;

   // Record the "volume" control ID for the MIDI line (as well as min/max volume settings)
   list.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
   if (mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(g_mixer), &list, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return;
   g_volume_control_id = control.dwControlID;
   DWORD min_volume = control.Bounds.dwMinimum;
   DWORD max_volume = control.Bounds.dwMaximum;


   MIXERCONTROLDETAILS details;
   details.cbStruct = sizeof(MIXERCONTROLDETAILS);
   details.cChannels = 1;
   details.cMultipleItems = 0;

   // Record whether mute is on
   MIXERCONTROLDETAILS_BOOLEAN muted_details;
   details.dwControlID = g_mute_control_id;
   details.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
   details.paDetails = &muted_details;
   if (mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(g_mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) return;
   g_old_mute = muted_details.fValue;

   // Always unmute if muted
   if (g_old_mute == TRUE)
   {
      muted_details.fValue = FALSE;
      if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(g_mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) return;
   }

   // Record current volume
   MIXERCONTROLDETAILS_UNSIGNED volume_details;
   details.dwControlID = g_volume_control_id;
   details.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
   details.paDetails = &volume_details;
   if (mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(g_mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) return;
   g_old_volume = volume_details.dwValue;

   // Decide whether we should increase the volume
   if (max_volume == min_volume) return;
   double volume_percent = (1.0 * g_old_volume - min_volume) / (1.0 * max_volume - min_volume);

   const static double LowVolumePercent = 0.33;
   const static double ComfortableVolumePercent = 0.75;
   if (volume_percent < LowVolumePercent)
   {
      DWORD comfortable_volume = static_cast<DWORD>(ComfortableVolumePercent * (max_volume - min_volume)) + min_volume;

      volume_details.dwValue = comfortable_volume;
      if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(g_mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) return;
   }
}

void MidiCommOut::RestoreSynthesizerVolume()
{
   MIXERCONTROLDETAILS details;
   details.cbStruct = sizeof(MIXERCONTROLDETAILS);
   details.cChannels = 1;
   details.cMultipleItems = 0;


   // Restore Mute
   MIXERCONTROLDETAILS_BOOLEAN muted_details;
   details.dwControlID = g_mute_control_id;
   details.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
   details.paDetails = &muted_details;

   muted_details.fValue = g_old_mute;
   if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(g_mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) return;


   // Restore Volume
   MIXERCONTROLDETAILS_UNSIGNED volume_details;
   details.dwControlID = g_volume_control_id;
   details.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
   details.paDetails = &volume_details;

   volume_details.dwValue = g_old_volume;
   if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(g_mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) return;


   mixerClose(g_mixer);
}


