// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_Playing.h"
#include "State_TrackSelection.h"
#include "State_Stats.h"
#include "version.h"

#include <string>
#include <iomanip>
using namespace std;

#include "string_util.h"
#include "MenuLayout.h"
#include "TextWriter.h"

#include "libmidi\Midi.h"
#include "libmidi\MidiTrack.h"
#include "libmidi\MidiEvent.h"
#include "libmidi\MidiUtil.h"

#include "libmidi\MidiComm.h"

void PlayingState::SetupNoteState()
{
   for (TranslatedNoteSet::iterator i = m_notes.begin(); i != m_notes.end(); ++i)
   {
      i->state = AutoPlayed;
      if (m_state.track_properties[i->track_id].mode == ModeYouPlay) i->state = UserPlayable;
   }
}

void PlayingState::ResetSong()
{
   if (m_state.midi_out) m_state.midi_out->Reset();
   if (m_state.midi_in) m_state.midi_in->Reset();

   // NOTE: These should be moved to a configuration file
   // along with ALL other "const static something" variables.
   const static unsigned long long LeadIn = 5000000;
   const static unsigned long long LeadOut = 1000000;

   if (!m_state.midi) return;

   m_state.midi->Reset(LeadIn, LeadOut);

   m_notes = m_state.midi->Notes();
   SetupNoteState();

   unsigned long long additional_time = m_state.midi->GetFirstNoteMicroseconds();
   additional_time -= LeadIn;
   Play(additional_time);

   m_state.stats = SongStatistics();
   m_state.stats.total_note_count = static_cast<int>(m_notes.size());

   m_current_combo = 0;
}

PlayingState::PlayingState(const SharedState &state)
   : m_state(state), m_keyboard(0), m_first_update(true), m_paused(false), m_any_you_play_tracks(false)
{ }

void PlayingState::Init()
{
   if (!m_state.midi) throw GameStateError("PlayingState: Init was passed a null MIDI!");

   for (size_t i = 0; i < m_state.track_properties.size(); ++i)
   {
      if (m_state.track_properties[i].mode == ModeYouPlay)
      {
         m_any_you_play_tracks = true;
         break;
      }
   }

   m_playback_speed = 100;

   // This many microseconds of the song will
   // be shown on the screen at once
   const static unsigned long long DefaultShowDurationMicroseconds = 4000000;
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
   height -= Layout::ButtonFontSize * 5;

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

      if (draw && (ev.Type() == MidiEventType_NoteOn || ev.Type() == MidiEventType_NoteOff))
      {
         int vel = ev.NoteVelocity();
         const string name = MidiEvent::NoteName(ev.NoteNumber());

         m_keyboard->SetKeyActive(name, (vel > 0), m_state.track_properties[track_id].color);
      }

      if (play && m_state.midi_out) m_state.midi_out->Write(ev);
   }
}

double PlayingState::CalculateScoreMultiplier() const
{
   const static double MaxMultiplier = 5.0;
   double multiplier = 1.0;

   const double combo_addition = m_current_combo / 10.0;
   multiplier += combo_addition;

   return min(MaxMultiplier, multiplier);
}

void PlayingState::Listen()
{
   if (!m_state.midi_in) return;

   while (m_state.midi_in->KeepReading())
   {
      unsigned long long cur_time = m_state.midi->GetSongPositionInMicroseconds();
      MidiEvent ev = m_state.midi_in->Read();

      // Just eat input if we're paused
      if (m_paused) continue;

      // We're only interested in NoteOn and NoteOff
      if (ev.Type() != MidiEventType_NoteOn && ev.Type() != MidiEventType_NoteOff) continue;
      string note_name = MidiEvent::NoteName(ev.NoteNumber());

      // If this was a key-release, we don't have to do much
      if (ev.Type() == MidiEventType_NoteOff || ev.NoteVelocity() == 0)
      {
         m_keyboard->SetKeyActive(note_name, false, FlatGray);
         continue;
      }

      bool any_found = false;
      for (TranslatedNoteSet::iterator i = m_notes.begin(); i != m_notes.end(); ++i)
      {
         const unsigned long long window_start = i->start - (KeyboardDisplay::NoteWindowLength / 2);
         const unsigned long long window_end = i->start + (KeyboardDisplay::NoteWindowLength / 2);

         if (window_start > cur_time) break;

         if (i->state != UserPlayable) continue;

         if (window_end > cur_time && i->note_id == ev.NoteNumber())
         {
            any_found = true;
            m_keyboard->SetKeyActive(note_name, true, m_state.track_properties[i->track_id].color);

            // Adjust our statistics
            const static double NoteValue = 100.0;
            m_state.stats.score += NoteValue * CalculateScoreMultiplier() * (m_playback_speed / 100.0);

            m_state.stats.notes_user_could_have_played++;
            m_state.stats.speed_integral += m_playback_speed;

            m_state.stats.notes_user_actually_played++;
            m_current_combo++;
            m_state.stats.longest_combo = max(m_current_combo, m_state.stats.longest_combo);

            i->state = UserHit;
            break;
         }
      }

      m_state.stats.total_notes_user_pressed++;

      if (!any_found)
      {
         m_state.stats.stray_notes++;

         m_keyboard->SetKeyActive(note_name, true, FlatGray);
      }
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
      Listen();
   }
   m_first_update = false;


   unsigned long long cur_time = m_state.midi->GetSongPositionInMicroseconds();

   // Delete notes that are finished playing (and are no longer available to hit)
   TranslatedNoteSet::iterator i = m_notes.begin();
   while (i != m_notes.end())
   {
      TranslatedNoteSet::iterator note = i++;

      const unsigned long long window_end = note->start + (KeyboardDisplay::NoteWindowLength / 2);

      if (m_state.midi_in && note->state == UserPlayable && window_end <= cur_time)
      {
         note->state = UserMissed;
      }

      if (note->start > cur_time) break;

      if (note->end < cur_time && window_end < cur_time)
      {
         if (note->state == UserMissed)
         {
            // They missed a note, reset the combo counter
            m_current_combo = 0;

            m_state.stats.notes_user_could_have_played++;
            m_state.stats.speed_integral += m_playback_speed;
         }

         m_notes.erase(note);
      }
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

   if (IsKeyPressed(KeySpace))
   {
      m_paused = !m_paused;
   }

   if (IsKeyPressed(KeyEscape))
   {
      if (m_state.midi_out) m_state.midi_out->Reset();
      if (m_state.midi_in) m_state.midi_in->Reset();

      ChangeState(new TrackSelectionState(m_state));
      return;
   }

   if (m_state.midi->GetSongPercentageComplete() >= 1.0)
   {
      if (m_state.midi_out) m_state.midi_out->Reset();
      if (m_state.midi_in) m_state.midi_in->Reset();

      if (m_state.midi_in && m_any_you_play_tracks) ChangeState(new StatsState(m_state));
      else ChangeState(new TrackSelectionState(m_state));

      return;
   }
}

void PlayingState::Draw(HDC hdc) const
{
   m_keyboard->Draw(hdc, Layout::ScreenMarginX, GetStateHeight() - CalcKeyboardHeight(), m_notes,
      m_show_duration, m_state.midi->GetSongPositionInMicroseconds(), m_state.track_properties);

   // Draw a song progress bar along the top of the screen
   HBRUSH pb_brush = CreateSolidBrush(RGB(0x50,0x50,0x50));

   const int pb_width = static_cast<int>(m_state.midi->GetSongPercentageComplete() * (GetStateWidth() - Layout::ScreenMarginX*2));
   const int pb_x = Layout::ScreenMarginX;
   const int pb_y = GetStateHeight() - CalcKeyboardHeight() - 20;

   RECT pb = { pb_x, pb_y, pb_x + pb_width, pb_y + 16 };
   FillRect(hdc, &pb, pb_brush);

   DeleteObject(pb_brush);


   Layout::DrawTitle(hdc, m_state.song_title);
   Layout::DrawHorizontalRule(hdc, GetStateWidth(), Layout::ScreenMarginY);

   // Some old time formatting code
   /*
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
   */

   if (m_state.midi_in && m_any_you_play_tracks)
   {
      int text_y = Layout::ScreenMarginY + Layout::SmallFontSize;

      wstring multiplier_text = WSTRING(fixed << setprecision(1) << CalculateScoreMultiplier());
      wstring speed_text = WSTRING(m_playback_speed << "%");

      TextWriter score(Layout::ScreenMarginX, text_y, hdc, false, Layout::TitleFontSize);
      score << Text(L"Score: ", Gray) << static_cast<int>(m_state.stats.score)
         << Text(L"  x  ", Gray) << Text(multiplier_text, RGB(138, 226, 52))
         << Text(L"  x  ", Gray) << Text(speed_text, RGB(114, 159, 207))
         << newline;
   }
}

