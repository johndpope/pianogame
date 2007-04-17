// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "StringTile.h"
#include "TextWriter.h"
#include "Renderer.h"

StringTile::StringTile(int x, int y, Tga *graphics) : m_x(x), m_y(y), m_graphics(graphics)
{
   whole_tile = ButtonState(0, 0, StringTileWidth, StringTileHeight);
}

void StringTile::Update(const MouseInfo &translated_mouse)
{
   whole_tile.Update(translated_mouse);
}

void StringTile::Draw(Renderer &renderer) const
{
   renderer.SetOffset(m_x, m_y);

   const Color hover = ToColor(0xFF,0xFF,0xFF);
   const Color no_hover = ToColor(0xE0,0xE0,0xE0);
   renderer.SetColor(whole_tile.hovering ? hover : no_hover);
   renderer.DrawTga(m_graphics, 0, 0);

   // NOTE: Title drawing disabled.  Expected to be in the texture
   //TextWriter title(10, 10, renderer, false, 14);
   //title << Text(m_title, ToColor(0xB0, 0xB0, 0xB0));

   TextWriter text(20, 46, renderer, false, 14);
   text << m_string;

   renderer.ResetOffset();
}

