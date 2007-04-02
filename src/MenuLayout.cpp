// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "MenuLayout.h"
#include "TextWriter.h"
#include "Renderer.h"

namespace Layout
{

void DrawTitle(Renderer &renderer, const std::wstring &title)
{
   TextWriter title_writer(ScreenMarginX, ScreenMarginX + TitleFontSize, renderer, false, TitleFontSize);
   title_writer << title;
}

void DrawHorizontalRule(Renderer &renderer, int state_width, int y)
{
   renderer.SetColor(0x50, 0x50, 0x50);
   renderer.DrawQuad(ScreenMarginX, y - 1, state_width - 2*ScreenMarginX, 3);
}

void DrawButton(Renderer &renderer, const ButtonState &button, const std::wstring &text, int text_x)
{
   const static Color color = ToColor(0x40,0x40,0x40);
   const static Color color_hover = ToColor(0x60,0x60,0x60);

   renderer.SetOffset(button.x, button.y);

   renderer.SetColor(button.hovering ? color_hover : color);
   renderer.DrawQuad(0, 0, ButtonWidth, ButtonHeight);

   TextWriter button_text(text_x, 8, renderer, false, ButtonFontSize);
   button_text << Text(text, ToColor(0xFF, 0xFF, 0xFF));

   renderer.ResetOffset();
}


} // End namespace Layout
