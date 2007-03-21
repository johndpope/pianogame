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

#define ToRGB(x) ( RGB((x).r, (x).g, (x).b) )

class Renderer
{
public:
   Renderer(HDC hdc);
   ~Renderer();

   Renderer(const Renderer& rhs);
   Renderer operator=(const Renderer& rhs);

   void SetColor(Color c);
   void SetColor(int r, int g, int b);
   void DrawQuad(int x, int y, int w, int h);

   // TODO: REMOVE!
   HDC GetHdc() { return m_hdc; }

private:

   Color m_c;
   HDC m_hdc;
   HBRUSH m_brush;
};

#endif
