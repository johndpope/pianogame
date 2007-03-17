// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __RENDERER_H
#define __RENDERER_H

#include "windows.h"

struct Color;

class Renderer
{
public:
   Renderer(HDC hdc) : m_hdc(hdc), m_brush(0) { }
   ~Renderer();

   void SetColor(Color c);
   void SetColor(int r, int g, int b);
   void DrawQuad(int x, int y, int w, int h);

   // TODO: REMOVE!
   HDC GetHdc() { return m_hdc; }

private:
   Renderer(const Renderer&);
   Renderer operator=(const Renderer&);

   HDC m_hdc;
   HBRUSH m_brush;
};

#endif
