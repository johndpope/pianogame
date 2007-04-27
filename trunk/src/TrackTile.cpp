// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "TrackTile.h"
#include "libmidi/Midi.h"
#include "Renderer.h"
#include "Tga.h"

const static int GraphicWidth = 36;
const static int GraphicHeight = 36;

TrackTile::TrackTile(int x, int y, size_t track_id, TrackColor color, TrackMode mode)
   : m_x(x), m_y(y), m_track_id(track_id), m_color(color), m_mode(mode), m_preview_on(false)
{
   // Initialize the size and position of each button
   whole_tile = ButtonState(0, 0, TrackTileWidth, TrackTileHeight);
   button_mode_left  = ButtonState(  2, 68, GraphicWidth, GraphicHeight);
   button_mode_right = ButtonState(192, 68, GraphicWidth, GraphicHeight);
   button_color      = ButtonState(228, 68, GraphicWidth, GraphicHeight);
   button_preview    = ButtonState(264, 68, GraphicWidth, GraphicHeight);
}

void TrackTile::Update(const MouseInfo &translated_mouse)
{
   // Update the mouse state of each button
   whole_tile.Update(translated_mouse);
   button_preview.Update(translated_mouse);
   button_color.Update(translated_mouse);
   button_mode_left.Update(translated_mouse);
   button_mode_right.Update(translated_mouse);

   if (button_mode_left.hit)
   {
      int mode = static_cast<int>(m_mode) - 1;
      if (mode < 0) mode = 3;

      m_mode = static_cast<TrackMode>(mode);
   }

   if (button_mode_right.hit)
   {
      int mode = static_cast<int>(m_mode) + 1;
      if (mode > 3) mode = 0;

      m_mode = static_cast<TrackMode>(mode);
   }

   if (button_preview.hit)
   {
      m_preview_on = !m_preview_on;
   }

   if (button_color.hit && m_mode != ModeNotPlayed && m_mode != ModePlayedButHidden)
   {
      int color = static_cast<int>(m_color) + 1;
      if (color >= UserSelectableColorCount) color = 0;

      m_color = static_cast<TrackColor>(color);
   }

}

int TrackTile::LookupGraphic(TrackTileGraphic graphic, bool button_hovering) const
{
   // There are three sets of graphics
   // set 0: window lit, hovering
   // set 1: window lit, not-hovering
   // set 2: window unlit, (implied not-hovering)
   int graphic_set = 2;
   if (whole_tile.hovering) graphic_set--;
   if (button_hovering) graphic_set--;

   const int set_offset = GraphicWidth * Graphic_COUNT;
   const int graphic_offset = GraphicWidth * graphic;

   return (set_offset * graphic_set) + graphic_offset;
}

void TrackTile::Draw(Renderer &renderer, const Midi *midi, Tga *buttons, Tga *box) const
{
   const MidiTrack &track = midi->Tracks()[m_track_id];

   bool gray_out_buttons = false;
   Color light  = TrackColorNoteWhite[m_color];
   Color medium = TrackColorNoteBlack[m_color];

   if (m_mode == ModePlayedButHidden || m_mode == ModeNotPlayed)
   {
      gray_out_buttons = true;
      light  = ToColor(0xB0,0xB0,0xB0);
      medium = ToColor(0x70,0x70,0x70);
   }

   Color color_tile = medium;
   Color color_tile_hovered = light;

   renderer.SetOffset(m_x, m_y);

   renderer.SetColor(whole_tile.hovering ? color_tile_hovered : color_tile);
   renderer.DrawTga(box, -10, -6);

   renderer.SetColor(White);

   // Write song info to the tile
   TextWriter instrument(95, 12, renderer, false, 14);
   instrument << track.InstrumentName();
   TextWriter note_count(95, 33, renderer, false, 14);
   note_count << static_cast<const unsigned int>(track.Notes().size());

   int color_offset = GraphicHeight * static_cast<int>(m_color);
   if (gray_out_buttons) color_offset = GraphicHeight * UserSelectableColorCount;

   renderer.DrawTga(buttons, BUTTON_RECT(button_mode_left),  LookupGraphic(GraphicLeftArrow,  button_mode_left.hovering), color_offset);
   renderer.DrawTga(buttons, BUTTON_RECT(button_mode_right), LookupGraphic(GraphicRightArrow, button_mode_right.hovering), color_offset);
   renderer.DrawTga(buttons, BUTTON_RECT(button_color),      LookupGraphic(GraphicColor,      button_color.hovering), color_offset);

   TrackTileGraphic preview_graphic = GraphicPreviewTurnOn;
   if (m_preview_on) preview_graphic = GraphicPreviewTurnOff;
   renderer.DrawTga(buttons, BUTTON_RECT(button_preview), LookupGraphic(preview_graphic, button_preview.hovering), color_offset);

   // Draw mode text
   TextWriter mode(42, 76, renderer, false, 14);
   mode << TrackModeText[m_mode];

   renderer.ResetOffset();
}

