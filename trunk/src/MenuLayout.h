// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __MENU_LAYOUT_H
#define __MENU_LAYOUT_H

#include "GameState.h"
#include <Windows.h>

struct ButtonState
{
   ButtonState() : hovering(false), depressed(false), x(0), y(0), w(0), h(0) { }
   ButtonState(int x, int y, int w, int h) : hovering(false), depressed(false), x(x), y(y), w(w), h(h) { }

   void Update(const MouseInfo &mouse)
   {
      hovering = mouse.x > x && mouse.x < x+w && mouse.y > y && mouse.y < y+h;
      depressed = hovering && mouse.held.left;
      hit = hovering && mouse.released.left;
   }

   // Simple mouse over
   bool hovering;

   // Mouse over while (left) button is held down
   bool depressed;

   // Mouse over just as the (left) button is released
   bool hit;

   int x, y;
   int w, h;
};

// Macro to turn replace Image::draw()'s 4 parameters with one
#define BUTTON_RECT(button) ((button).x), ((button).y), ((button).w), ((button).h)

namespace Layout
{
   void DrawTitle(HDC hdc, const std::wstring &title);
   void DrawHorizontalRule(HDC hdc, int state_width, int y);
   void DrawButton(HDC hdc, const ButtonState &button, const std::wstring &text, int text_x);

   // Pixel margin forced at edges of screen
   const static int ScreenMarginX = 16;
   const static int ScreenMarginY = 70;

   const static int TitleFontSize = 20;
   const static int ScoreFontSize = 26;
   const static int ButtonFontSize = 14;
   const static int SmallFontSize = 12;

   const static int ButtonWidth = 140;
   const static int ButtonHeight = 35;

};

#endif