// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __SYNTH_VOLUME_H
#define __SYNTH_VOLUME_H

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include <vector>

// Windows Media Player has started changing the MIDI synth
// volume to 0 after playing a MIDI file and exiting.  Seeing
// as how that's a common file-preview path just before starting
// Synthesia, we have to combat the behavior ourselves.
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

#endif
