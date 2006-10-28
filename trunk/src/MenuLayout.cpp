// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "MenuLayout.h"
#include "Image.h"
#include "TextWriter.h"

namespace Layout
{

void DrawTitle(HDC hdc, const std::wstring &title)
{
   TextWriter title_writer(ScreenMarginX, ScreenMarginX + TitleFontSize, hdc, false, TitleFontSize);
   title_writer << title;
}

void DrawHorizontalRule(HDC hdc, int state_width, int y)
{
   HPEN pen = CreatePen(PS_SOLID, 3, RGB(0x50,0x50,0x50));
   HPEN old_pen = static_cast<HPEN>(SelectObject(hdc, pen));

   MoveToEx(hdc, ScreenMarginX, y, 0);
   LineTo(hdc, state_width - ScreenMarginX, y);

   SelectObject(hdc, old_pen);
   DeleteObject(pen);
}

void DrawButton(HDC hdc, const ButtonState &button, const std::wstring &text, int text_x)
{
   const static COLORREF color = RGB(0x40,0x40,0x40);
   const static COLORREF color_hover = RGB(0x60,0x60,0x60);

   Image button_img(ButtonWidth, ButtonHeight, button.hovering ? color_hover : color );
   HDC button_hdc = button_img.beginDrawingOn();
   TextWriter button_text(text_x, 8, button_hdc, false, ButtonFontSize);
   button_text << Text(text, RGB(255, 255, 255));
   button_img.endDrawingOn();

   button_img.beginDrawing(hdc);
   button_img.draw(button.x, button.y);
   button_img.endDrawing();
}


} // End namespace Layout
