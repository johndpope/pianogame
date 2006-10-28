// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "DeviceTile.h"
#include "Image.h"

#include "libmidi\Midi.h"
#include "libmidi\MidiComm.h"

const static int GraphicWidth = 36;
const static int GraphicHeight = 36;

DeviceTile::DeviceTile(int x, int y, unsigned int out_id)
   : m_x(x), m_y(y), m_out_id(out_id), m_preview_on(false)
{
   // Initialize the size and position of each button
   whole_tile = ButtonState(0, 0, DeviceTileWidth, DeviceTileHeight);
   button_mode_left  = ButtonState(  6, 38, GraphicWidth, GraphicHeight);
   button_mode_right = ButtonState(428, 38, GraphicWidth, GraphicHeight);
   button_preview    = ButtonState(469, 38, GraphicWidth, GraphicHeight);
}

void DeviceTile::Update(const MouseInfo &translated_mouse)
{
   // Update the mouse state of each button
   whole_tile.Update(translated_mouse);
   button_preview.Update(translated_mouse);
   button_mode_left.Update(translated_mouse);
   button_mode_right.Update(translated_mouse);

   const MidiCommDescriptionList devices = MidiCommOut::GetDeviceList();

   if (devices.size() > 0)
   {
      const unsigned int last_device = static_cast<unsigned int>(devices.size() - 1);

      if (button_mode_left.hit)
      {
         if (m_out_id == 0) m_out_id = last_device;
         else --m_out_id;
      }

      if (button_mode_right.hit)
      {
         if (m_out_id == last_device) m_out_id = 0;
         else ++m_out_id;
      }
   }

   if (button_preview.hit)
   {
      m_preview_on = !m_preview_on;
   }
}

int DeviceTile::LookupGraphic(TrackTileGraphic graphic, bool button_hovering) const
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

void DeviceTile::Draw(HDC hdc) const
{
   COLORREF light  = RGB(0xB0,0xB0,0xB0);
   COLORREF medium = RGB(0x70,0x70,0x70);
   COLORREF dark   = RGB(0x50,0x50,0x50);

   Image tile(DeviceTileWidth, DeviceTileHeight, whole_tile.hovering ? medium : dark );
   HDC tile_hdc = tile.beginDrawingOn();

   // Draw horizontal rule
   HPEN pen = CreatePen(PS_SOLID, 1, light);
   HPEN old_pen = static_cast<HPEN>(SelectObject(tile_hdc, pen));
   MoveToEx(tile_hdc, 10, 30, 0);
   LineTo(tile_hdc, DeviceTileWidth - 10, 30);
   SelectObject(tile_hdc, old_pen);
   DeleteObject(pen);

   // Write song info to the tile
   TextWriter title(10, 10, tile_hdc, false, 14);
   title << Text(L"Choose MIDI Output Device:", light);

   Image graphics(Image::GetGlobalModuleInstance(), L"BITMAP_TRACKTILE");
   graphics.EnableTransparency();

   // Choose the last (gray) color in the TrackTile bitmap
   int color_offset = GraphicHeight * TrackColorCount;

   graphics.beginDrawing(tile_hdc);
   graphics.draw(BUTTON_RECT(button_mode_left),  LookupGraphic(GraphicLeftArrow,  button_mode_left.hovering), color_offset);
   graphics.draw(BUTTON_RECT(button_mode_right), LookupGraphic(GraphicRightArrow, button_mode_right.hovering), color_offset);

   TrackTileGraphic preview_graphic = GraphicPreviewTurnOn;
   if (m_preview_on) preview_graphic = GraphicPreviewTurnOff;
   graphics.draw(BUTTON_RECT(button_preview), LookupGraphic(preview_graphic, button_preview.hovering), color_offset);

   graphics.endDrawing();

   const MidiCommDescriptionList devices = MidiCommOut::GetDeviceList();

   // Draw mode text
   TextWriter mode(44, 46, tile_hdc, false, 14);
   if (devices.size() > m_out_id)
   {
      mode << devices[m_out_id].name;
   }
   else
   {
      mode << L"No Midi Output Devices Found!";
   }

   tile.endDrawingOn();

   // Draw the tile to the screen
   tile.beginDrawing(hdc);
   tile.draw(m_x, m_y);
   tile.endDrawing();
}

