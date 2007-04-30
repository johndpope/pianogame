// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "MidiEvent.h"
#include "MidiComm.h"
#include "MidiUtil.h"

#include <string>
#include <sstream>
using namespace std;

#include "string_util.h"

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#ifdef WIN32

void midi_check(MMRESULT ret)
{
   if (ret == MMSYSERR_NOERROR) return;

   switch (ret)
   {
   case MIDIERR_NODEVICE:     throw MidiError(MidiError_MM_NoDevice);
   case MMSYSERR_NOTENABLED:  throw MidiError(MidiError_MM_NotEnabled);
   case MMSYSERR_ALLOCATED:   throw MidiError(MidiError_MM_AlreadyAllocated);
   case MMSYSERR_BADDEVICEID: throw MidiError(MidiError_MM_BadDeviceID);
   case MMSYSERR_INVALPARAM:  throw MidiError(MidiError_MM_InvalidParameter);
   case MMSYSERR_NODRIVER:    throw MidiError(MidiError_MM_NoDriver);
   case MMSYSERR_NOMEM:       throw MidiError(MidiError_MM_NoMemory);
   default:                   throw MidiError(MidiError_MM_Unknown);
   }
}

void CALLBACK MidiInputCallback(HMIDIIN, UINT msg, DWORD_PTR instance, DWORD p1, DWORD p2)
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

      const static int MaxTries = 10;
      int tries = 0;
      while (tries++ < MaxTries)
      {
         try
         {
            midi_check(midiInGetDevCaps(i, &dev, sizeof(MIDIINCAPS)));
            break;
         }
         catch (MidiError ex)
         {
            // Sometimes input needs to take a quick break
            if (ex.m_error != MidiError_MM_NotEnabled) throw;
            Sleep(50);
         }
      }
      if (tries == MaxTries) throw MidiError_MM_NotEnabled;

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
void MidiCommIn::InputCallback(unsigned int msg, unsigned long p1, unsigned long)
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

#else

// TODO: Clean up this function.  It was grabbed from
// some place online and needs tending.
static CFStringRef EndpointName(MIDIEndpointRef endpoint)
{
   CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
   CFStringRef str;
   
   // begin with the endpoint's name
   str = NULL;
   MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
   if (str != NULL) {
      CFStringAppend(result, str);
      CFRelease(str);
   }
   
   MIDIEntityRef entity = NULL;
   MIDIEndpointGetEntity(endpoint, &entity);
   if (entity == NULL)
      // probably virtual
      return result;
   
   if (CFStringGetLength(result) == 0) {
      // endpoint name has zero length -- try the entity
      str = NULL;
      MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
      if (str != NULL) {
         CFStringAppend(result, str);
         CFRelease(str);
      }
   }
   // now consider the device's name
   MIDIDeviceRef device = NULL;
   MIDIEntityGetDevice(entity, &device);
   if (device == NULL)
      return result;
   
   str = NULL;
   MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
   if (str != NULL) {
      // if an external device has only one entity, throw away
      // the endpoint name and just use the device name
      if (MIDIDeviceGetNumberOfEntities(device) < 2)
      {
         CFRelease(result);
         return str;
      }
      else
      {
         // does the entity name already start with the device name?
         // (some drivers do this though they shouldn't)
         // if so, do not prepend
            if (CFStringCompareWithOptions(str /* device name */,
            result /* endpoint name */,
            CFRangeMake(0, CFStringGetLength(str)), 0) != kCFCompareEqualTo) {
            // prepend the device name to the entity name
            if (CFStringGetLength(result) > 0)
               CFStringInsert(result, 0, CFSTR(" "));
            CFStringInsert(result, 0, str);
         }
CFRelease(str);
      }
   }
return result;
}


static bool built_input_list = false;
static MidiCommDescriptionList in_list(MidiCommIn::GetDeviceList());


MidiCommDescriptionList MidiCommIn::GetDeviceList()
{
   if (built_input_list) return in_list;

   MidiCommDescriptionList devices;

   ItemCount sources = MIDIGetNumberOfSources();
   for (int i = 0; i < sources; ++i)
   {
      MIDIEndpointRef endpoint = MIDIGetSource(i);
      
      CFStringRef cf_name = EndpointName(endpoint);
      
      MidiCommDescription d;
      d.id = i;
      d.name = WideFromMacString(cf_name);
      CFRelease(cf_name);
      
      devices.push_back(d);
   }   

   built_input_list = true;
   return devices;
}

void midi_input(const MIDIPacketList *packet_list, void *read_ref_con, void *source_ref_con)
{
   MidiCommIn *comm_in = (MidiCommIn*)source_ref_con;

   // TODO: There is no guarantee that events are coming in one at a time!   
   const MIDIPacket *packet = &packet_list->packet[0];
   for (int i = 0; i < packet_list->numPackets; ++i)
   {
      comm_in->InputCallback(packet->data[0], packet->data[1], packet->data[2]);
      packet = MIDIPacketNext(packet);
   }
}

MidiCommIn::MidiCommIn(unsigned int device_id)
{
   pthread_mutex_init(&m_mutex, 0);

   m_description = MidiCommIn::GetDeviceList()[device_id];

   MIDIClientCreate(CFSTR("Synthesia"), 0, this, &m_client);
   MIDIInputPortCreate(m_client, CFSTR("Synthesia In"), midi_input, this, &m_port);
   
   MIDIEndpointRef source = MIDIGetSource(device_id);
   MIDIPortConnectSource(m_port, source, this);
}

MidiCommIn::~MidiCommIn()
{
   MIDIEndpointRef source = MIDIGetSource(m_description.id);
   MIDIPortDisconnectSource(m_port, source);

   pthread_mutex_destroy(&m_mutex);
}

void MidiCommIn::InputCallback(unsigned int status, unsigned long byte1, unsigned long byte2)
{
   unsigned char small_status = (unsigned char)status;
   unsigned char small_byte1  = (unsigned char)byte1;
   unsigned char small_byte2  = (unsigned char)byte2;
   MidiEvent ev = MidiEvent::Build(MidiEventSimple(small_status, small_byte1, small_byte2));

   pthread_mutex_lock(&m_mutex);
   m_event_buffer.push(ev);
   pthread_mutex_unlock(&m_mutex);
}

void MidiCommIn::Reset()
{
   pthread_mutex_lock(&m_mutex);
   while (!m_event_buffer.empty()) m_event_buffer.pop();
   pthread_mutex_unlock(&m_mutex);
}

bool MidiCommIn::KeepReading() const
{
   bool buffer_empty;

   pthread_mutex_lock(&m_mutex);
   buffer_empty = m_event_buffer.empty();
   pthread_mutex_unlock(&m_mutex);

   return (!buffer_empty);
}

MidiEvent MidiCommIn::Read()
{
   MidiEvent ev(MidiEvent::NullEvent());
   bool buffer_empty;

   pthread_mutex_lock(&m_mutex);
   buffer_empty = m_event_buffer.empty();
   if (!buffer_empty)
   {
      ev = m_event_buffer.front();
      m_event_buffer.pop();
   }
   pthread_mutex_unlock(&m_mutex);

   if (buffer_empty) throw MidiError(MidiError_NoInputAvailable);
   return ev;
}




MidiCommDescriptionList MidiCommOut::GetDeviceList()
{
   MidiCommDescriptionList devices;

   MidiCommDescription mac_synth;
   mac_synth.id = 0;
   mac_synth.name = L"Built-in MIDI Synthesizer";
   
   devices.push_back(mac_synth);

   return devices;
}

MidiCommOut::MidiCommOut(unsigned int device_id)
{
   // MACNOTE: For now we ignore device_id.  We only support the built-in synth
   m_description.id = 0;
   m_description.name = L"Built-in MIDI Synthesizer";


   // Open the Music Device
   ComponentDescription compdesc;
   compdesc.componentType = kAudioUnitComponentType;
   compdesc.componentSubType = kAudioUnitSubType_MusicDevice;
   compdesc.componentManufacturer = kAudioUnitID_DLSSynth;
   compdesc.componentFlags = 0;
   compdesc.componentFlagsMask = 0;

   Component compid = FindNextComponent(NULL, &compdesc);
   m_device = static_cast<AudioUnit>(OpenComponent(compid));
	
   // open the output unit
   m_output = static_cast<AudioUnit>(OpenDefaultComponent(kAudioUnitComponentType, kAudioUnitSubType_Output));
	
   // connect the units
   AudioUnitConnection auconnect;
   auconnect.sourceAudioUnit = m_device;
   auconnect.sourceOutputNumber = 0;
   auconnect.destInputNumber = 0;
   AudioUnitSetProperty(m_output, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
                           static_cast<void*>(&auconnect), sizeof(AudioUnitConnection));
	
   // initialize the units
   AudioUnitInitialize(m_device);
   AudioUnitInitialize(m_output);
	
   // start the output
   AudioOutputUnitStart(m_output);
}

MidiCommOut::~MidiCommOut()
{
   AudioOutputUnitStop(m_output);
	
   CloseComponent(m_output);
   CloseComponent(m_device);
}

void MidiCommOut::Write(const MidiEvent &out)
{
   MidiEventSimple simple;
   if (out.GetSimpleEvent(&simple))
   {
      MusicDeviceMIDIEvent(m_device, simple.status, simple.byte1, simple.byte2, 0);
   }
}

void MidiCommOut::Reset()
{
   for (int i = 0; i < 16; ++i)
   {
      AudioUnitReset(m_device, kAudioUnitScope_Group, i);
      AudioUnitReset(m_output, kAudioUnitScope_Group, i);
   }
}





#endif
