// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "StringTile.h"
#include "TextWriter.h"
#include "Renderer.h"

StringTile::StringTile(int x, int y) : m_x(x), m_y(y)
{
   whole_tile = ButtonState(0, 0, StringTileWidth, StringTileHeight);
}

void StringTile::Update(const MouseInfo &translated_mouse)
{
   whole_tile.Update(translated_mouse);
}

void StringTile::Draw(Renderer &renderer) const
{
   const Color light  = ToColor(0xB0,0xB0,0xB0);
   const Color medium = ToColor(0x70,0x70,0x70);
   const Color dark   = ToColor(0x50,0x50,0x50);

   renderer.SetOffset(m_x, m_y);

   renderer.SetColor(whole_tile.hovering ? medium : dark);
   renderer.DrawQuad(0, 0, StringTileWidth, StringTileHeight);

   // Draw horizontal rule
   renderer.SetColor(light);
   renderer.DrawQuad(10, 30, StringTileWidth - 20, 1);

   TextWriter title(10, 10, renderer, false, 14);
   title << Text(m_title, light);

   TextWriter text(10, 46, renderer, false, 14);
   text << m_string;

   renderer.ResetOffset();
}

