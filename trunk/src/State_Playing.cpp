// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_Playing.h"
#include "State_TrackSelection.h"
#include "version.h"

#include <string>
using namespace std;

#include "string_util.h"
#include "MenuLayout.h"
#include "TextWriter.h"

#include "libmidi\Midi.h"
#include "libmidi\MidiTrack.h"
#include "libmidi\MidiEvent.h"
#include "libmidi\MidiUtil.h"

#include "libmidi\MidiComm.h"

void PlayingState::ResetSong()
{
   m_state.midi_out->Reset();

   // NOTE: These should be moved to a configuration file
   // along with ALL other "const static something" variables.
   const static unsigned long long LeadIn = 6000000;
   const static unsigned long long LeadOut = 2500000;

   if (!m_state.midi) return;

   m_state.midi->Reset(LeadIn, LeadOut);
   m_notes = m_state.midi->Notes();

   unsigned long long additional_time = m_state.midi->GetFirstNoteMicroseconds();
   additional_time -= LeadIn;
   Play(additional_time);
}

PlayingState::PlayingState(const SharedState &state)
   : m_state(state), m_keyboard(0), m_first_update(true), m_paused(false)
{ }

void PlayingState::Init()
{
   if (!m_state.midi) throw GameStateError("PlayingState: Init was passed a null MIDI!");

   m_playback_speed = 100;

   // This many microseconds of the song will
   // be shown on the screen at once
   const static unsigned long long DefaultShowDurationMicroseconds = 5000000;
   m_show_duration = DefaultShowDurationMicroseconds;

   m_keyboard = new KeyboardDisplay(KeyboardSize88, GetStateWidth() - 2*Layout::ScreenMarginX, CalcKeyboardHeight());

   // Hide the mouse cursor while we're playing
   ShowCursor(false);

   ResetSong();
}

PlayingState::~PlayingState()
{
   ShowCursor(true);
}

int PlayingState::CalcKeyboardHeight() const
{
   // Start with the size of the screen
   int height = GetStateHeight();

   // Leave at least enough for a title bar and horizontal
   // rule at the top
   height -= Layout::ScreenMarginY;

   // Allow another couple lines of text below the HR
   height -= Layout::ButtonFontSize * 4;

   return height;
}

void PlayingState::Play(unsigned long long delta_microseconds)
{
   MidiEventListWithTrackId evs = m_state.midi->Update(delta_microseconds);

   const size_t length = evs.size();
   for (size_t i = 0; i < length; ++i)
   {
      const size_t &track_id = evs[i].first;
      const MidiEvent &ev = evs[i].second;

      // Draw refers to the keys lighting up (automatically) -- not necessarily
      // the falling notes.  The KeyboardDisplay object contains its own logic
      // to decide how to draw the falling notes
      bool draw = false;
      bool play = false;
      switch (m_state.track_properties[track_id].mode)
      {
      case ModeNotPlayed:           draw = false;  play = false;  break;
      case ModePlayedButHidden:     draw = false;  play = true;   break;
      case ModeYouPlay:             draw = false;  play = false;  break;
      case ModePlayedAutomatically: draw = true;   play = true;   break;
      }

      if (draw && ev.Type() == MidiEventType_NoteOn || ev.Type() == MidiEventType_NoteOff)
      {
         int vel = ev.NoteVelocity();
         const string name = MidiEvent::NoteName(ev.NoteNumber());

         m_keyboard->SetKeyActive(name, (vel > 0), m_state.track_properties[track_id].color);
      }

      if (play) m_state.midi_out->Write(ev);
   }
}

void PlayingState::Update()
{
   const unsigned long long current_microseconds = GetStateMilliseconds() * 1000;
   unsigned long long delta_microseconds = static_cast<unsigned long long>(GetDeltaMilliseconds()) * 1000;

   // The 100 term is really paired with the playback speed, but this
   // formation is less likely to produce overflow errors.
   delta_microseconds = (delta_microseconds / 100) * m_playback_speed;

   if (m_paused) delta_microseconds = 0;

   // Our delta milliseconds on the first frame after state start is extra
   // long because we just reset the MIDI.  By skipping the "Play" that
   // update, we don't have an artificially fast-forwarded start.
   if (!m_first_update)
   {
      Play(delta_microseconds);
   }
   m_first_update = false;


   // Delete notes that are finished playing
   unsigned long long cur_time = m_state.midi->GetSongPositionInMicroseconds();
   for (TranslatedNoteSet::iterator i = m_notes.begin(); i != m_notes.end(); )
   {
      TranslatedNoteSet::iterator next = i;
      ++next;

      if (i->end < cur_time) m_notes.erase(i);
      i = next;
   }

   if (IsKeyPressed(KeyUp))
   {
      m_show_duration -= 250000;
      if (m_show_duration < 250000) m_show_duration = 250000;
   }

   if (IsKeyPressed(KeyDown))
   {
      m_show_duration += 250000;
      if (m_show_duration > 10000000) m_show_duration = 10000000;
   }

   if (IsKeyPressed(KeyLeft))
   {
      m_playback_speed -= 10;
      if (m_playback_speed < 0) m_playback_speed = 0;
   }

   if (IsKeyPressed(KeyRight))
   {
      m_playback_speed += 10;
      if (m_playback_speed > 400) m_playback_speed = 400;
   }

   if (IsKeyPressed(KeyEnter))
   {
      ResetSong();
      m_keyboard->ResetActiveKeys();
   }

   if (IsKeyPressed(KeySpace))
   {
      m_paused = !m_paused;
   }

   if (IsKeyPressed(KeyEscape) || m_state.midi->GetSongPercentageComplete() >= 1.0)
   {
      ChangeState(new TrackSelectionState(m_state));
   }
}

void PlayingState::Draw(HDC hdc) const
{
   m_keyboard->Draw(hdc, Layout::ScreenMarginX, GetStateHeight() - CalcKeyboardHeight(), m_notes,
      m_show_duration, m_state.midi->GetSongPositionInMicroseconds(), m_state.track_properties);

   Layout::DrawTitle(hdc, m_state.song_title);
   Layout::DrawHorizontalRule(hdc, GetStateWidth(), Layout::ScreenMarginY);

   double non_zero_playback_speed = ( (m_playback_speed == 0) ? 0.1 : (m_playback_speed/100.0) );
   unsigned long long tot_seconds = static_cast<unsigned long long>((m_state.midi->GetSongLengthInMicroseconds() / 100000.0) / non_zero_playback_speed);
   unsigned long long cur_seconds = static_cast<unsigned long long>((m_state.midi->GetSongPositionInMicroseconds() / 100000.0) / non_zero_playback_speed);
   int completion  = static_cast<int>(m_state.midi->GetSongPercentageComplete() * 100.0);

   unsigned int tot_min = static_cast<unsigned int>((tot_seconds/10) / 60);
   unsigned int tot_sec = static_cast<unsigned int>((tot_seconds/10) % 60);
   unsigned int tot_ten = static_cast<unsigned int>( tot_seconds%10      );
   const wstring total_time = WSTRING(tot_min << L":" << setfill(L'0') << setw(2) << tot_sec << L"." << tot_ten);
   
   unsigned int cur_min = static_cast<unsigned int>((cur_seconds/10) / 60);
   unsigned int cur_sec = static_cast<unsigned int>((cur_seconds/10) % 60);
   unsigned int cur_ten = static_cast<unsigned int>( cur_seconds%10      );
   const wstring current_time = WSTRING(cur_min << L":" << setfill(L'0') << setw(2) << cur_sec << L"." << cur_ten);
   const wstring percent_complete = WSTRING(L" (" << completion << L"%)");

   int small_text_y = Layout::ScreenMarginY + Layout::SmallFontSize;
   TextWriter stats1(Layout::ScreenMarginX, small_text_y, hdc, false, Layout::SmallFontSize);
   stats1 << Text(L"Time: ", Gray) << current_time << L" / " << total_time << percent_complete << newline;
   stats1 << Text(L"Speed: ", Gray) << m_playback_speed << L"%" << newline;

   TextWriter stats2(Layout::ScreenMarginX + 220, small_text_y, hdc, false, Layout::SmallFontSize);
   stats2 << Text(L"Events: ", Gray) << m_state.midi->AggregateEventCount() - m_state.midi->AggregateEventsRemain() << L" / " << m_state.midi->AggregateEventCount() << newline;
   stats2 << Text(L"Notes: ", Gray) << m_state.midi->AggregateNoteCount() - m_state.midi->AggregateNotesRemain() << L" / " << m_state.midi->AggregateNoteCount() << newline;
}

