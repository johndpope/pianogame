// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __STATE_PLAYING_H
#define __STATE_PLAYING_H

#include <string>
#include <vector>

#include "SharedState.h"
#include "GameState.h"
#include "KeyboardDisplay.h"

struct TrackProperties;
class Midi;
class MidiCommOut;
class MidiCommIn;

class PlayingState : public GameState
{
public:
   PlayingState(const SharedState &state);
   ~PlayingState();

protected:
   virtual void Init();
   virtual void Update();
   virtual void Draw(HDC hdc) const;

private:

   int CalcKeyboardHeight() const;
   void SetupNoteState();

   void ResetSong();
   void Play(microseconds_t delta_microseconds);
   void Listen();

   double CalculateScoreMultiplier() const;

   bool m_paused;
   int m_playback_speed;

   KeyboardDisplay *m_keyboard;
   microseconds_t m_show_duration;
   TranslatedNoteSet m_notes;

   bool m_any_you_play_tracks;
   size_t m_look_ahead_you_play_note_count;

   bool m_first_update;

   SharedState m_state;
   int m_current_combo;
};

#endif