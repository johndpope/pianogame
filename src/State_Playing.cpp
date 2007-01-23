// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_Playing.h"
#include "State_TrackSelection.h"
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

TranslatedNoteSet PlayingState::CalculateNoteWindows(const unsigned long long opportunity_length)
{
   TranslatedNoteSet windows;

   const TranslatedNoteSet &notes = m_state.midi->Notes();
   for (TranslatedNoteSet::const_iterator i = notes.begin(); i != notes.end(); ++i)
   {
      const TranslatedNote &n = *i;
      if (m_state.track_properties[n.track_id].mode == ModeYouPlay)
      {
         // Start with the actual note
         TranslatedNote window = n;

         // Adjust the start and end to fit just around the start of the actual note
         window.start = n.start - (opportunity_length / 2);
         window.end = n.start + (opportunity_length / 2);

         windows.insert(window);
      }
   }

   return windows;
}

void PlayingState::ResetSong()
{
   if (m_state.midi_out) m_state.midi_out->Reset();

   // NOTE: These should be moved to a configuration file
   // along with ALL other "const static something" variables.
   const static unsigned long long LeadIn = 6000000;
   const static unsigned long long LeadOut = 1500000;
   const static unsigned long long NoteWindowLength = 320000;

   if (!m_state.midi) return;

   m_state.midi->Reset(LeadIn, LeadOut);

   m_notes = m_state.midi->Notes();
   m_user_note_windows = CalculateNoteWindows(NoteWindowLength);

   unsigned long long additional_time = m_state.midi->GetFirstNoteMicroseconds();
   additional_time -= LeadIn;
   Play(additional_time);

   m_state.stats = SongStatistics();
   m_state.stats.total_note_count = static_cast<int>(m_notes.size());
   m_state.stats.notes_user_actually_played = static_cast<int>(m_user_note_windows.size());

   m_current_combo = 0;
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
   while (m_state.midi_in->KeepReading())
   {
      unsigned long long cur_time = m_state.midi->GetSongPositionInMicroseconds();
      MidiEvent ev = m_state.midi_in->Read();

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
      TranslatedNoteSet::iterator i = m_user_note_windows.begin();
      while (i != m_user_note_windows.end())
      {
         if (i->start > cur_time) break;

         TranslatedNoteSet::iterator check = i++;
         if (check->start < cur_time && check->end > cur_time && check->note_id == ev.NoteNumber())
         {
            any_found = true;
            m_keyboard->SetKeyActive(note_name, true, m_state.track_properties[check->track_id].color);

            // Adjust our statistics
            const static double NoteValue = 10.0;
            m_state.stats.score += NoteValue * CalculateScoreMultiplier() * (m_playback_speed / 100.0);

            m_state.stats.notes_user_actually_played++;
            m_current_combo++;
            m_state.stats.longest_combo = max(m_current_combo, m_state.stats.longest_combo);

            m_user_note_windows.erase(check);
         }
      }

      if (!any_found) m_keyboard->SetKeyActive(note_name, true, FlatGray);
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

   // Delete notes that are finished playing
   TranslatedNoteSet::iterator i = m_notes.begin();
   while (i != m_notes.end())
   {
      if (i->start > cur_time) break;

      TranslatedNoteSet::iterator pending_delete = i++;
      if (pending_delete->end < cur_time) m_notes.erase(pending_delete);
   }

   // Remove missed opportunities
   i = m_user_note_windows.begin();
   while (i != m_user_note_windows.end())
   {
      if (i->start > cur_time) break;

      TranslatedNoteSet::iterator pending_delete = i++;
      if (pending_delete->end < cur_time)
      {
         m_user_note_windows.erase(pending_delete);

         // They missed a note, reset the combo counter
         m_current_combo = 0;
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

   wstring multiplier_text = WSTRING( L"  x" << fixed << setprecision(1) << CalculateScoreMultiplier());

   TextWriter stats2(Layout::ScreenMarginX + 220, small_text_y, hdc, false, Layout::TitleFontSize);
   stats2 << Text(L"Score: ", Gray) << static_cast<int>(m_state.stats.score)
      << Text(multiplier_text, CheatYellow) << newline;
}

