// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "MidiUtil.h"
#include "../string_util.h"

using namespace std;


#ifdef WIN32

unsigned long swap32(unsigned long x) 
{
   return ((((x) & 0x00ff0000) >> 8 )  |
          (( (x) & 0x0000ff00) << 8 )  |
          (( (x) & 0xff000000) >> 24)  |
          (( (x) & 0x000000ff) << 24));
}

unsigned short swap16(unsigned short x)
{
   return ((((x) & 0xff00) >> 8) |
          (( (x) & 0x00ff) << 8));
}

#endif


unsigned long parse_variable_length(istream &in)
{
   register unsigned long value = in.get();

   if (in.good() && (value & 0x80) )
   {
      value &= 0x7F;

      register unsigned long c;
      do
      {
         c = in.get();
         value = (value << 7) + (c & 0x7F);
      } while (in.good() && (c & 0x80) );
   }

   return(value);
}

std::wstring MidiError::GetErrorDescription() const
{
   switch (m_error)
   {
   case MidiError_UnknownHeaderType:                  return L"Found an unknown header type.\n\nThis probably isn't a valid MIDI file.";
   case MidiError_BadFilename:                        return L"Could not open file for input. Check that file exists.";
   case MidiError_NoHeader:                           return L"No MIDI header could be read.  File too short.";
   case MidiError_BadHeaderSize:                      return L"Incorrect header size.";
   case MidiError_Type2MidiNotSupported:              return L"Type 2 MIDI is not supported.";
   case MidiError_BadType0Midi:                       return L"Type 0 MIDI should only have 1 track.";
   case MidiError_SMTPETimingNotImplemented:          return L"MIDI using SMTP time division is not implemented.";

   case MidiError_BadTrackHeaderType:                 return L"Found an unknown track header type.";
   case MidiError_TrackHeaderTooShort:                return L"File terminated before reading track header.";
   case MidiError_TrackTooShort:                      return L"Data stream too short to read entire track.";
   case MidiError_BadTrackEnd:                        return L"MIDI track did not end with End-Of-Track event.";

   case MidiError_EventTooShort:                      return L"Data stream ended before reported end of MIDI event.";
   case MidiError_UnknownEventType:                   return L"Found an unknown MIDI Event Type.";
   case MidiError_UnknownMetaEventType:               return L"Found an unknown MIDI Meta Event Type.";

   case MidiError_MM_NoDevice:                        return L"Could not open the specified MIDI device.";
   case MidiError_MM_AlreadyAlocated:                 return L"The specified MIDI device is already in use.";
   case MidiError_MM_BadDeviceID:                     return L"The MIDI device ID specified is out of range.";
   case MidiError_MM_InvalidParameter:                return L"An invalid parameter was used with the MIDI device.";
   case MidiError_MM_NoDriver:                        return L"The specified MIDI driver is not installed.";
   case MidiError_MM_NoMemory:                        return L"Cannot allocate or lock memory for MIDI device.";
   case MidiError_MM_Unknown:                         return L"An unknown MIDI I/O error has occurred.";

   case MidiError_NoInputAvailable:                   return L"Attempted to read MIDI event from an empty input buffer.";
   case MidiError_UnexpectedInput:                    return L"Received unexpected MIDI input from device.";
   case MidiError_MetaEventOnInput:                   return L"MIDI Input device sent a Meta Event.";

   case MidiError_RequestedTempoFromNonTempoEvent:    return L"Tempo data was requested from a non-tempo MIDI event.";
   case MidiError_UnresolvedNoteEvents:               return L"Found a 'note on' event without a matching 'note off'.";

   default:                                           return WSTRING(L"Unknown MidiError Code (" << m_error << L").");
   }
}

std::wstring GetMidiEventTypeDescription(MidiEventType type)
{
   switch (type)
   {
   case MidiEventType_Meta:             return L"Meta";
   case MidiEventType_SysEx:            return L"System Exclusive";

   case MidiEventType_NoteOff:          return L"Note-Off";
   case MidiEventType_NoteOn:           return L"Note-On";
   case MidiEventType_Aftertouch:       return L"Aftertouch";
   case MidiEventType_Controller:       return L"Controller";
   case MidiEventType_ProgramChange:    return L"Program Change";
   case MidiEventType_ChannelPressure:  return L"Channel Pressure";
   case MidiEventType_PitchWheel:       return L"Pitch Wheel";

   case MidiEventType_Unknown:          return L"Unknown";
   default:                             return L"BAD EVENT TYPE";
   }

}

std::wstring GetMidiMetaEventTypeDescription(MidiMetaEventType type)
{
   switch (type)
   {
   case MidiMetaEvent_SequenceNumber:   return L"Sequence Number";

   case MidiMetaEvent_Text:             return L"Text";
   case MidiMetaEvent_Copyright:        return L"Copyright";
   case MidiMetaEvent_TrackName:        return L"Track Name";
   case MidiMetaEvent_Instrument:       return L"Instrument";
   case MidiMetaEvent_Lyric:            return L"Lyric";
   case MidiMetaEvent_Marker:           return L"Marker";
   case MidiMetaEvent_Cue:              return L"Cue Point";
   case MidiMetaEvent_PatchName:        return L"Patch Name";
   case MidiMetaEvent_DeviceName:       return L"Device Name";

   case MidiMetaEvent_EndOfTrack:       return L"End Of Track";
   case MidiMetaEvent_TempoChange:      return L"Tempo Change";
   case MidiMetaEvent_SMPTEOffset:      return L"SMPTE Offset";
   case MidiMetaEvent_TimeSignature:    return L"Time Signature";
   case MidiMetaEvent_KeySignature:     return L"Key Signature";

   case MidiMetaEvent_Proprietary:      return L"Proprietary";

   case MidiMetaEvent_ChannelPrefix:    return L"(Deprecated) Channel Prefix";
   case MidiMetaEvent_MidiPort:         return L"(Deprecated) MIDI Port";

   case MidiMetaEvent_Unknown:          return L"Unknown Meta Event Type";
   default:                             return L"BAD META EVENT TYPE";
   }
}

std::wstring const InstrumentNames[InstrumentCount] = {
   L"Acoustic Grand Piano",
   L"Bright Acoustic Piano",
   L"Electric Grand Piano",
   L"Honky-tonk Piano",
   L"Electric Piano 1",
   L"Electric Piano 2",
   L"Harpsichord",
   L"Clavi",
   L"Celesta",
   L"Glockenspiel",
   L"Music Box",
   L"Vibraphone",
   L"Marimba",
   L"Xylophone",
   L"Tubular Bells",
   L"Dulcimer",
   L"Drawbar Organ",
   L"Percussive Organ",
   L"Rock Organ",
   L"Church Organ",
   L"Reed Organ",
   L"Accordion",
   L"Harmonica",
   L"Tango Accordion",
   L"Acoustic Guitar (nylon)",
   L"Acoustic Guitar (steel)",
   L"Electric Guitar (jazz)",
   L"Electric Guitar (clean)",
   L"Electric Guitar (muted)",
   L"Overdriven Guitar",
   L"Distortion Guitar",
   L"Guitar harmonics",
   L"Acoustic Bass",
   L"Electric Bass (finger)",
   L"Electric Bass (pick)",
   L"Fretless Bass",
   L"Slap Bass 1",
   L"Slap Bass 2",
   L"Synth Bass 1",
   L"Synth Bass 2",
   L"Violin",
   L"Viola",
   L"Cello",
   L"Contrabass",
   L"Tremolo Strings",
   L"Pizzicato Strings",
   L"Orchestral Harp",
   L"Timpani",
   L"String Ensemble 1",
   L"String Ensemble 2",
   L"SynthStrings 1",
   L"SynthStrings 2",
   L"Choir Aahs",
   L"Voice Oohs",
   L"Synth Voice",
   L"Orchestra Hit",
   L"Trumpet",
   L"Trombone",
   L"Tuba",
   L"Muted Trumpet",
   L"French Horn",
   L"Brass Section",
   L"SynthBrass 1",
   L"SynthBrass 2",
   L"Soprano Sax",
   L"Alto Sax",
   L"Tenor Sax",
   L"Baritone Sax",
   L"Oboe",
   L"English Horn",
   L"Bassoon",
   L"Clarinet",
   L"Piccolo",
   L"Flute",
   L"Recorder",
   L"Pan Flute",
   L"Blown Bottle",
   L"Shakuhachi",
   L"Whistle",
   L"Ocarina",
   L"Lead 1 (square)",
   L"Lead 2 (sawtooth)",
   L"Lead 3 (calliope)",
   L"Lead 4 (chiff)",
   L"Lead 5 (charang)",
   L"Lead 6 (voice)",
   L"Lead 7 (fifths)",
   L"Lead 8 (bass + lead)",
   L"Pad 1 (new age)",
   L"Pad 2 (warm)",
   L"Pad 3 (polysynth)",
   L"Pad 4 (choir)",
   L"Pad 5 (bowed)",
   L"Pad 6 (metallic)",
   L"Pad 7 (halo)",
   L"Pad 8 (sweep)",
   L"FX 1 (rain)",
   L"FX 2 (soundtrack)",
   L"FX 3 (crystal)",
   L"FX 4 (atmosphere)",
   L"FX 5 (brightness)",
   L"FX 6 (goblins)",
   L"FX 7 (echoes)",
   L"FX 8 (sci-fi)",
   L"Sitar",
   L"Banjo",
   L"Shamisen",
   L"Koto",
   L"Kalimba",
   L"Bag pipe",
   L"Fiddle",
   L"Shanai",
   L"Tinkle Bell",
   L"Agogo",
   L"Steel Drums",
   L"Woodblock",
   L"Taiko Drum",
   L"Melodic Tom",
   L"Synth Drum",
   L"Reverse Cymbal",
   L"Guitar Fret Noise",
   L"Breath Noise",
   L"Seashore",
   L"Bird Tweet",
   L"Telephone Ring",
   L"Helicopter",
   L"Applause",
   L"Gunshot",

   //
   // NOTE: These aren't actually General MIDI instruments!
   //
   L"Percussion", // for Tracks that use Channel 10
   L"Various"     // for Tracks that use more than one
};
