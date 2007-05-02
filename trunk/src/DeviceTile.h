// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __DEVICE_TILE_H
#define __DEVICE_TILE_H

#include "GameState.h"
#include "MenuLayout.h"
#include "TrackTile.h"
#include <vector>

#include "libmidi/Midi.h"
#include "libmidi/MidiComm.h"

class Renderer;
class Tga;

const int DeviceTileWidth = 510;
const int DeviceTileHeight = 80;

enum TrackTileGraphic;

enum DeviceTileType
{
   DeviceTileOutput,
   DeviceTileInput
};

class DeviceTile
{
public:
   DeviceTile(int x, int y, int device_id, DeviceTileType type, const MidiCommDescriptionList &device_list, 
                       Tga *button_graphics, Tga *frame_graphics);

   void Update(const MouseInfo &translated_mouse);
   void Draw(Renderer &renderer) const;

   int GetX() const { return m_x; }
   int GetY() const { return m_y; }

   bool HitPreviewButton() const { return button_preview.hit; }
   bool IsPreviewOn() const { return m_preview_on; }
   void TurnOffPreview() { m_preview_on = false; }

   int GetDeviceId() const { return m_device_id; }

   const ButtonState WholeTile() const { return whole_tile; }
   const ButtonState ButtonPreview() const { return button_preview; }
   const ButtonState ButtonLeft() const { return button_mode_left; }
   const ButtonState ButtonRight() const { return button_mode_right; }

private:
   DeviceTile operator=(const DeviceTile &);

   int m_x;
   int m_y;

   bool m_preview_on;
   int m_device_id;

   const MidiCommDescriptionList m_device_list;

   DeviceTileType m_tile_type;

   Tga *m_button_graphics;
   Tga *m_frame_graphics;

   ButtonState whole_tile;
   ButtonState button_preview;
   ButtonState button_mode_left;
   ButtonState button_mode_right;

   int LookupGraphic(TrackTileGraphic graphic, bool button_hovering) const;
};

#endif
