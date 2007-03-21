// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "SynthVolume.h"

// The following Mixer routines are based off Chen Su's Audio Mixer Functions Demo
// which can be found at: http://www.codeproject.com/audio/admixer.asp

ReasonableSynthVolume::ReasonableSynthVolume()
{
   int mixer_count = mixerGetNumDevs();
   for (int i = 0; i < mixer_count; ++i)
   {
      m_states.push_back(SynthVolumeState());
      SynthVolumeState &s = m_states[m_states.size() - 1];

      if (mixerOpen(&s.mixer, 0, 0, 0, MIXER_OBJECTF_MIXER) != MMSYSERR_NOERROR) continue;

      MIXERCAPS caps;
      if (mixerGetDevCaps((UINT_PTR)s.mixer, &caps, sizeof(MIXERCAPS)) != MMSYSERR_NOERROR) continue;

      // Open the MIDI "line" of the mixer
      MIXERLINE line;
      line.cbStruct = sizeof(MIXERLINE);
      line.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER;
      if (mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(s.mixer), &line, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR) continue;


      MIXERLINECONTROLS list;
      MIXERCONTROL control;
      list.cbStruct = sizeof(MIXERLINECONTROLS);
      list.dwLineID = line.dwLineID;
      list.cControls = 1;
      list.cbmxctrl = sizeof(MIXERCONTROL);
      list.pamxctrl = &control;

      // Record the "mute" control ID for the MIDI line
      list.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
      if (mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(s.mixer), &list, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) continue;
      s.mute_control_id = control.dwControlID;

      // Record the "volume" control ID for the MIDI line (as well as min/max volume settings)
      list.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
      if (mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(s.mixer), &list, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) continue;
      s.volume_control_id = control.dwControlID;
      DWORD min_volume = control.Bounds.dwMinimum;
      DWORD max_volume = control.Bounds.dwMaximum;


      MIXERCONTROLDETAILS details;
      details.cbStruct = sizeof(MIXERCONTROLDETAILS);
      details.cChannels = 1;
      details.cMultipleItems = 0;

      // Record whether mute is on
      MIXERCONTROLDETAILS_BOOLEAN muted_details;
      details.dwControlID = s.mute_control_id;
      details.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      details.paDetails = &muted_details;
      if (mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(s.mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) continue;
      s.old_mute = muted_details.fValue;

      // Always unmute if muted
      if (s.old_mute == TRUE)
      {
         muted_details.fValue = FALSE;
         if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(s.mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) continue;
      }

      // Record current volume
      MIXERCONTROLDETAILS_UNSIGNED volume_details;
      details.dwControlID = s.volume_control_id;
      details.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      details.paDetails = &volume_details;
      if (mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(s.mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) continue;
      s.old_volume = volume_details.dwValue;

      // Decide whether we should increase the volume
      if (max_volume == min_volume) return;
      double volume_percent = (1.0 * s.old_volume - min_volume) / (1.0 * max_volume - min_volume);

      const static double LowVolumePercent = 0.33;
      const static double ComfortableVolumePercent = 0.75;
      if (volume_percent < LowVolumePercent)
      {
         DWORD comfortable_volume = static_cast<DWORD>(ComfortableVolumePercent * (max_volume - min_volume)) + min_volume;

         volume_details.dwValue = comfortable_volume;
         if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(s.mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) continue;
      }
   }
}

ReasonableSynthVolume::~ReasonableSynthVolume()
{
   int mixer_count = mixerGetNumDevs();
   for (int i = 0; i < mixer_count; ++i)
   {
      if (m_states.size() <= static_cast<unsigned int>(i)) continue;
      SynthVolumeState &s = m_states[i];

      MIXERCONTROLDETAILS details;
      details.cbStruct = sizeof(MIXERCONTROLDETAILS);
      details.cChannels = 1;
      details.cMultipleItems = 0;


      // Restore Mute
      MIXERCONTROLDETAILS_BOOLEAN muted_details;
      details.dwControlID = s.mute_control_id;
      details.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      details.paDetails = &muted_details;

      muted_details.fValue = s.old_mute;
      if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(s.mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) continue;


      // Restore Volume
      MIXERCONTROLDETAILS_UNSIGNED volume_details;
      details.dwControlID = s.volume_control_id;
      details.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
      details.paDetails = &volume_details;

      volume_details.dwValue = s.old_volume;
      if (mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(s.mixer), &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR) continue;

      mixerClose(s.mixer);
   }
}


