// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "DeviceTile.h"
#include "TextWriter.h"
#include "Renderer.h"
#include "Tga.h"

#include "libmidi\Midi.h"
#include "libmidi\MidiComm.h"

const static int GraphicWidth = 36;
const static int GraphicHeight = 36;

DeviceTile::DeviceTile(int x, int y, DeviceTileType type, int device_id, Tga *button_graphics, Tga *frame_graphics)
: m_x(x), m_y(y), m_device_id(device_id), m_preview_on(false), m_tile_type(type),
   m_button_graphics(button_graphics), m_frame_graphics(frame_graphics)
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

   const MidiCommDescriptionList input_devices = MidiCommIn::GetDeviceList();
   const MidiCommDescriptionList output_devices = MidiCommOut::GetDeviceList();

   const MidiCommDescriptionList *devices = 0;
   switch (m_tile_type)
   {
   case DeviceTileOutput: devices = &output_devices; break;
   case DeviceTileInput:  devices = &input_devices;  break;
   }

   if (devices && devices->size() > 0)
   {
      const int last_device = static_cast<int>(devices->size() - 1);

      if (button_mode_left.hit)
      {
         if (m_device_id == -1) m_device_id = last_device;
         else --m_device_id;
      }

      if (button_mode_right.hit)
      {
         if (m_device_id == last_device) m_device_id = -1;
         else ++m_device_id;
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

void DeviceTile::Draw(Renderer &renderer) const
{
   renderer.SetOffset(m_x, m_y);

   const Color hover = ToColor(0xFF,0xFF,0xFF);
   const Color no_hover = ToColor(0xE0,0xE0,0xE0);
   renderer.SetColor(whole_tile.hovering ? hover : no_hover);
   renderer.DrawTga(m_frame_graphics, 0, 0);

   // Choose the last (gray) color in the TrackTile bitmap
   int color_offset = GraphicHeight * UserSelectableColorCount;

   renderer.DrawTga(m_button_graphics, BUTTON_RECT(button_mode_left), LookupGraphic(GraphicLeftArrow,  button_mode_left.hovering), color_offset);
   renderer.DrawTga(m_button_graphics, BUTTON_RECT(button_mode_right), LookupGraphic(GraphicRightArrow, button_mode_right.hovering), color_offset);

   TrackTileGraphic preview_graphic = GraphicPreviewTurnOn;
   if (m_preview_on) preview_graphic = GraphicPreviewTurnOff;
   renderer.DrawTga(m_button_graphics, BUTTON_RECT(button_preview), LookupGraphic(preview_graphic, button_preview.hovering), color_offset);

   const MidiCommDescriptionList input_devices = MidiCommIn::GetDeviceList();
   const MidiCommDescriptionList output_devices = MidiCommOut::GetDeviceList();

   const MidiCommDescriptionList *devices = 0;
   switch (m_tile_type)
   {
   case DeviceTileOutput: devices = &output_devices; break;
   case DeviceTileInput:  devices = &input_devices;  break;
   }

   // Draw mode text
   TextWriter mode(44, 46, renderer, false, 14);
   if (devices->size() == 0)
   {
      mode << L"[No Devices Found]";
   }
   else
   {
      // A -1 for device_id means "disabled"
      if (m_device_id >= 0)
      {
         MidiCommDescriptionList d = *devices;
         mode << d[m_device_id].name;
      }
      else
      {
         switch (m_tile_type)
         {
         case DeviceTileOutput: mode << L"[Output Off: Display only with no audio]"; break;
         case DeviceTileInput:  mode << L"[Input Off: Play along with no scoring]";  break;
         }
      }
   }

   renderer.ResetOffset();
}

