// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __STATE_TRACKSELECTION_H
#define __STATE_TRACKSELECTION_H

#include "SharedState.h"
#include "GameState.h"
#include "TrackTile.h"
#include "libmidi/MidiTypes.h"
#include <vector>

class Midi;
class MidiCommOut;

class TrackSelectionState : public GameState
{
public:
   TrackSelectionState(const SharedState &state);

protected:
   virtual void Init();
   virtual void Update();
   virtual void Draw(Renderer &renderer) const;

private:
   void PlayTrackPreview(microseconds_t additional_time);
   std::vector<Track::Properties> BuildTrackProperties() const;

   int m_page_count;
   int m_current_page;
   int m_tiles_per_page;

   bool m_preview_on;
   bool m_first_update_after_seek;
   size_t m_preview_track_id;

   ButtonState m_continue_button;
   ButtonState m_back_button;

   std::wstring m_tooltip;

   std::vector<TrackTile> m_track_tiles;

   SharedState m_state;
};

#endif
