// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __RENDERER_H
#define __RENDERER_H

#include <Windows.h>

class Tga;

struct Color
{
   int r, g, b, a;
};

static Color ToColor(int r, int g, int b, int a = 0xFF)
{
   Color c;
   c.r = r;
   c.g = g;
   c.b = b;
   c.a = a;

   return c;
}

class Renderer
{
public:
   Renderer(HDC hdc);

   void SetOffset(int x, int y) { m_xoffset = x; m_yoffset = y; }
   void ResetOffset() { SetOffset(0,0); }

   void ForceTexture(unsigned int texture_id);

   void SetColor(Color c);
   void SetColor(int r, int g, int b, int a = 0xFF);
   void DrawQuad(int x, int y, int w, int h);

   void DrawTga(const Tga *tga, int x, int y) const;
   void DrawTga(const Tga *tga, int x, int y, int width, int height, int src_x, int src_y) const;

   void DrawStretchedTga(const Tga *tga, int x, int y, int w, int h) const;
   void DrawStretchedTga(const Tga *tga, int x, int y, int w, int h, int src_x, int src_y, int src_w, int src_h) const;

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
