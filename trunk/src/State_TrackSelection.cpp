// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_TrackSelection.h"

#include "State_Title.h"
#include "State_Playing.h"
#include "MenuLayout.h"

#include "libmidi\Midi.h"
#include "libmidi\MidiUtil.h"
#include "libmidi\MidiComm.h"

TrackSelectionState::TrackSelectionState(const SharedState &state)
   : m_state(state), m_preview_on(false), m_preview_track_id(0),
   m_first_update_after_seek(false),
   m_page_count(0), m_current_page(0), m_tiles_per_page(0)
{ }

void TrackSelectionState::Init()
{
   if (m_state.midi_out) m_state.midi_out->Reset();

   Midi &m = *m_state.midi;

   // Prepare a very simple count of the playable tracks first
   int track_count = 0;
   for (size_t i = 0; i < m.Tracks().size(); ++i)
   {
      if (m.Tracks()[i].Notes().size()) track_count++;
   }

   m_back_button = ButtonState(Layout::ScreenMarginX,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   m_continue_button = ButtonState(GetStateWidth() - Layout::ScreenMarginX - Layout::ButtonWidth,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   // Determine how many track tiles we can fit
   // horizontally and vertically. Integer division
   // helps us round down here.
   int tiles_across = (GetStateWidth() + Layout::ScreenMarginX) / (TrackTileWidth + Layout::ScreenMarginX);
   tiles_across = max(tiles_across, 1);

   int tiles_down = (GetStateHeight() - Layout::ScreenMarginX - Layout::ScreenMarginY * 2) / (TrackTileHeight + Layout::ScreenMarginX);
   tiles_down = max(tiles_down, 1);

   // Calculate how many pages of tracks there will be
   m_tiles_per_page = tiles_across * tiles_down;

   m_page_count        = track_count / m_tiles_per_page;
   const int remainder = track_count % m_tiles_per_page;
   if (remainder > 0) m_page_count++;

   // If we have fewer than one row of tracks, just
   // center the tracks we do have
   if (track_count < tiles_across) tiles_across = track_count;

   // Determine how wide that many track tiles will
   // actually be, so we can center the list
   int all_tile_widths = tiles_across * TrackTileWidth + (tiles_across-1) * Layout::ScreenMarginX;
   int global_x_offset = (GetStateWidth() - all_tile_widths) / 2;

   const static int starting_y = 90;

   int tiles_on_this_line = 0;
   int tiles_on_this_page = 0;
   int current_y = starting_y;
   for (size_t i = 0; i < m.Tracks().size(); ++i)
   {
      const MidiTrack &t = m.Tracks()[i];
      if (t.Notes().size() == 0) continue;

      int x = global_x_offset + (TrackTileWidth + Layout::ScreenMarginX)*tiles_on_this_line;
      int y = current_y;

      TrackMode mode = ModePlayedAutomatically;
      if (t.IsPercussion()) mode = ModePlayedButHidden;

      // The "less 1" here is because the first track
      // is always the tempo track
      TrackColor color = static_cast<TrackColor>((i-1) % UserSelectableColorCount);

      // If we came back here from StatePlaying, reload all our preferences
      if (m_state.track_properties.size() > i)
      {
         color = m_state.track_properties[i].color;
         mode = m_state.track_properties[i].mode;
      }

      TrackTile tile(x, y, i, color, mode);

      m_track_tiles.push_back(tile);


      tiles_on_this_line++;
      tiles_on_this_line %= tiles_across;
      if (!tiles_on_this_line)
      {
         current_y += TrackTileHeight + Layout::ScreenMarginX;
      }

      tiles_on_this_page++;
      tiles_on_this_page %= m_tiles_per_page;
      if (!tiles_on_this_page)
      {
         current_y = starting_y;
         tiles_on_this_line = 0;
      }
   }
}

std::vector<TrackProperties> TrackSelectionState::BuildTrackProperties() const
{
   std::vector<TrackProperties> props;
   for (size_t i = 0; i < m_state.midi->Tracks().size(); ++i)
   {
      props.push_back(TrackProperties());
   }

   // Populate it with the tracks that have notes
   for (std::vector<TrackTile>::const_iterator i = m_track_tiles.begin(); i != m_track_tiles.end(); ++i)
   {
      props[i->GetTrackId()].color = i->GetColor();
      props[i->GetTrackId()].mode = i->GetMode();
   }

   return props;
}

void TrackSelectionState::Update()
{
   m_continue_button.Update(MouseInfo(Mouse()));
   m_back_button.Update(MouseInfo(Mouse()));

   if (IsKeyPressed(KeyEscape) || m_back_button.hit)
   {
      if (m_state.midi_out) m_state.midi_out->Reset();
      m_state.track_properties = BuildTrackProperties();
      ChangeState(new TitleState(m_state));
      return;
   }

   if (IsKeyPressed(KeyEnter) || m_continue_button.hit)
   {

      if (m_state.midi_out) m_state.midi_out->Reset();
      m_state.track_properties = BuildTrackProperties();
      ChangeState(new PlayingState(m_state));

      return;
   }

   if (IsKeyPressed(KeyDown) || IsKeyPressed(KeyRight))
   {
      m_current_page++;
      if (m_current_page == m_page_count) m_current_page = 0;
   }

   if (IsKeyPressed(KeyUp) || IsKeyPressed(KeyLeft))
   {
      m_current_page--;
      if (m_current_page < 0) m_current_page += m_page_count;
   }


   // Our delta milliseconds on the first frame after we seek down to the
   // first note is extra long because the seek takes a while.  By skipping
   // the "Play" that update, we don't have an artificially fast-forwarded
   // start.
   if (!m_first_update_after_seek)
   {
      PlayTrackPreview(static_cast<unsigned long long>(GetDeltaMilliseconds()) * 1000);
   }
   m_first_update_after_seek = false;

   // Do hit testing on each tile button on this page
   size_t start = m_current_page * m_tiles_per_page;
   size_t end = min( static_cast<size_t>((m_current_page+1) * m_tiles_per_page), m_track_tiles.size() );
   for (size_t i = start; i < end; ++i)
   {
      TrackTile &t = m_track_tiles[i];

      MouseInfo mouse = MouseInfo(Mouse());
      mouse.x -= t.GetX();
      mouse.y -= t.GetY();

      t.Update(mouse, m_state.midi);

      if (t.HitPreviewButton())
      {
         if (m_state.midi_out) m_state.midi_out->Reset();

         if (t.IsPreviewOn())
         {
            // Turn off any other preview modes
            for (size_t j = 0; j < m_track_tiles.size(); ++j)
            {
               if (i == j) continue;
               m_track_tiles[j].TurnOffPreview();
            }

            const unsigned long long PreviewLeadIn  = 250000;
            const unsigned long long PreviewLeadOut = 250000;

            m_preview_on = true;
            m_preview_track_id = t.GetTrackId();
            m_state.midi->Reset(PreviewLeadIn, PreviewLeadOut);
            PlayTrackPreview(0);

            // Find the first note in this track so we can skip right to the good part.
            unsigned long first_note_pulse = m_state.midi->Tracks()[m_preview_track_id].Notes().begin()->start;
            unsigned long long additional_time = m_state.midi->GetEventPulseInMicroseconds(first_note_pulse);
            additional_time -= PreviewLeadIn;

            PlayTrackPreview(additional_time);
            m_first_update_after_seek = true;
         }
         else
         {
            m_preview_on = false;
         }
      }
   }
}

void TrackSelectionState::PlayTrackPreview(unsigned long long delta_microseconds)
{
   if (!m_preview_on) return;

   MidiEventListWithTrackId evs = m_state.midi->Update(delta_microseconds);

   for (MidiEventListWithTrackId::const_iterator i = evs.begin(); i != evs.end(); ++i)
   {
      const MidiEvent &ev = i->second;
      if (i->first != m_preview_track_id) continue;

      if (m_state.midi_out) m_state.midi_out->Write(ev);
   }
}

void TrackSelectionState::Draw(HDC hdc) const
{
   Layout::DrawTitle(hdc, L"Choose Tracks To Play");

   Layout::DrawHorizontalRule(hdc, GetStateWidth(), Layout::ScreenMarginY);
   Layout::DrawHorizontalRule(hdc, GetStateWidth(), GetStateHeight() - Layout::ScreenMarginY);

   Layout::DrawButton(hdc, m_continue_button, L"Play Song", 33);
   Layout::DrawButton(hdc, m_back_button, L"Back to Title", 23);

   // Write our page count on the screen
   const static int TypicalPaginationTextWidth = 280;
   TextWriter pagination((GetStateWidth() - TypicalPaginationTextWidth)/2, GetStateHeight() - Layout::ScreenMarginY
      + Layout::ScreenMarginX + Layout::SmallFontSize, hdc, false, Layout::ButtonFontSize);
   
   pagination << Text(L"Page ", Gray) << (m_current_page+1) << Text(L" of ", Gray) << m_page_count <<
      Text(L" (arrow keys change page)", Gray);

   // Draw each track tile on the current page
   size_t start = m_current_page * m_tiles_per_page;
   size_t end = min( static_cast<size_t>((m_current_page+1) * m_tiles_per_page), m_track_tiles.size() );
   for (size_t i = start; i < end; ++i)
   {
      m_track_tiles[i].Draw(hdc, m_state.midi);
   }
}
