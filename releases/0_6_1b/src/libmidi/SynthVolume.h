
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __SYNTH_VOLUME_H
#define __SYNTH_VOLUME_H

#ifdef WIN32

#include <Windows.h>
#include <vector>

// Windows Media Player (starting in version 11) has started changing
// the MIDI synth volume to 0 after playing a MIDI file and exiting.
// Seeing as how that's a common file-preview path just before starting
// Piano Game, we have to combat the behavior ourselves.
//
// Once initialized, it checks to see if the system MIDI synth volume
// is "reasonable" (above 33% or so volume), and if not, it will set
// it to something like 75%.  On object destruction, it sets the volume
// back to its previous volume.
class ReasonableSynthVolume
{
public:
   ReasonableSynthVolume();
   ~ReasonableSynthVolume();

private:

   struct SynthVolumeState
   {
      SynthVolumeState() : old_mute(1), old_volume(0), mixer(0),
         mute_control_id(0), volume_control_id(0) { }

      LONG old_mute;
      DWORD old_volume;

      HMIXER mixer;
      DWORD mute_control_id;
      DWORD volume_control_id;
   };

   std::vector<SynthVolumeState> m_states;
};

#else

// Don't do anything on the Mac side

class ReasonableSynthVolume
{
public:
   ReasonableSynthVolume() { }
};

#endif


#endif
