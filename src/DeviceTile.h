// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __DEVICE_TILE_H
#define __DEVICE_TILE_H

#include "GameState.h"
#include "TextWriter.h"
#include "MenuLayout.h"
#include "TrackTile.h"
#include <vector>

class MidiCommOut;

const int DeviceTileWidth = 510;
const int DeviceTileHeight = 80;

enum TrackTileGraphic;

class DeviceTile
{
public:
   DeviceTile(int x, int y, unsigned int out_id);

   void Update(const MouseInfo &translated_mouse);
   void Draw(HDC hdc) const;

   int GetX() { return m_x; }
   int GetY() { return m_y; }

   bool HitPreviewButton() const { return button_preview.hit; }
   bool IsPreviewOn() const { return m_preview_on; }
   void TurnOffPreview() { m_preview_on = false; }

   unsigned int GetOutId() const { return m_out_id; }

private:
   int m_x;
   int m_y;

   bool m_preview_on;
   unsigned int m_out_id;

   ButtonState whole_tile;
   ButtonState button_preview;
   ButtonState button_mode_left;
   ButtonState button_mode_right;

   int LookupGraphic(TrackTileGraphic graphic, bool button_hovering) const;
};

#endif