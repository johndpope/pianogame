// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "StringTile.h"
#include "Image.h"
#include "TextWriter.h"
#include "Renderer.h"

StringTile::StringTile(int x, int y)
: m_x(x), m_y(y)
{
   whole_tile = ButtonState(0, 0, StringTileWidth, StringTileHeight);
}

void StringTile::Update(const MouseInfo &translated_mouse)
{
   whole_tile.Update(translated_mouse);
}

void StringTile::Draw(Renderer &renderer) const
{
   COLORREF light  = RGB(0xB0,0xB0,0xB0);
   COLORREF medium = RGB(0x70,0x70,0x70);
   COLORREF dark   = RGB(0x50,0x50,0x50);

   Image tile(StringTileWidth, StringTileHeight, whole_tile.hovering ? medium : dark );
   Renderer tile_renderer = tile.beginDrawingOn();

   // Draw horizontal rule
   tile_renderer.SetColor(0xB0, 0xB0, 0xB0);
   tile_renderer.DrawQuad(10, 30, StringTileWidth - 20, 1);

   TextWriter title(10, 10, tile_renderer, false, 14);
   title << Text(m_title, light);

   TextWriter text(10, 46, tile_renderer, false, 14);
   text << m_string;

   tile.endDrawingOn();

   // Draw the tile to the screen
   tile.beginDrawing(renderer);
   tile.draw(m_x, m_y);
   tile.endDrawing();
}

