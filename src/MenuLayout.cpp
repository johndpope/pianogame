// Synthesia
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "MenuLayout.h"
#include "TextWriter.h"
#include "Renderer.h"

namespace Layout
{

void DrawTitle(Renderer &renderer, const std::wstring &title)
{
   TextWriter title_writer(ScreenMarginX, ScreenMarginY - TitleFontSize - 10, renderer, false, TitleFontSize);
   title_writer << title;
}

void DrawHorizontalRule(Renderer &renderer, int state_width, int y)
{
   renderer.SetColor(0x50, 0x50, 0x50);
   renderer.DrawQuad(ScreenMarginX, y - 1, state_width - 2*ScreenMarginX, 3);
}

void DrawButton(Renderer &renderer, const ButtonState &button, const Tga *tga)
{
   const static Color color = ToColor(0xE0,0xE0,0xE0);
   const static Color color_hover = ToColor(0xFF,0xFF,0xFF);

   renderer.SetColor(button.hovering ? color_hover : color);
   renderer.DrawTga(tga, button.x, button.y);
}


} // End namespace Layout
