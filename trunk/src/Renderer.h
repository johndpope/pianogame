// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __RENDERER_H
#define __RENDERER_H

#include <Windows.h>

struct Color
{
   int r, g, b;
};

static Color ToColor(int r, int g, int b)
{
   Color c;
   c.r = r;
   c.g = g;
   c.b = b;

   return c;
}

// TODO: Remove
#define ToCOLORREF(x) ( RGB((x).r, (x).g, (x).b) )

class Renderer
{
public:
   Renderer(HDC hdc);

   void SetOffset(int x, int y) { m_xoffset = x; m_yoffset = y; }
   void ResetOffset() { SetOffset(0,0); }

   void SetColor(Color c);
   void SetColor(int r, int g, int b);
   void DrawQuad(int x, int y, int w, int h);

   // TODO: REMOVE!
   HDC GetHdc() { return m_hdc; }

   // TODO: REMOVE!
   int GetXoffset() const { return m_xoffset; }
   int GetYoffset() const { return m_yoffset; }

private:
   int m_xoffset;
   int m_yoffset;

   HDC m_hdc;
};

#endif
