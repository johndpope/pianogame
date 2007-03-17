// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "Renderer.h"
#include "TrackProperties.h"

Renderer::~Renderer()
{
   if (m_brush) DeleteObject(m_brush);
   m_brush = 0;

   m_hdc = 0;
}

void Renderer::SetColor(Color c)
{
   SetColor(c.r, c.g, c.b);
}

void Renderer::SetColor(int r, int g, int b)
{
   if (m_brush) DeleteObject(m_brush);
   m_brush = CreateSolidBrush(RGB(r,g,b));
}

void Renderer::DrawQuad(int x, int y, int w, int h)
{
   if (!m_brush) return;

   RECT r = { x, y, x+w, y+h };
   FillRect(m_hdc, &r, m_brush);
}
