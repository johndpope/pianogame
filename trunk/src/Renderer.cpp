// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "Renderer.h"

Renderer::Renderer(HDC hdc) : m_c(ToColor(0,0,0)), m_hdc(hdc), m_brush(0), m_color_changed(true)
{
}

Renderer::~Renderer()
{
   if (m_brush) DeleteObject(m_brush);
}

Renderer::Renderer(const Renderer& rhs)
{
   m_hdc = rhs.m_hdc;

   m_color_changed = true;
   m_brush = 0;
   m_c = rhs.m_c;
}

Renderer Renderer::operator=(const Renderer& rhs)
{
   if (&rhs == this) return rhs;
   return Renderer(rhs);
}

void Renderer::SetColor(Color c)
{
   SetColor(c.r, c.g, c.b);
}

void Renderer::SetColor(int r, int g, int b)
{
   m_c = ToColor(r, g, b);
   m_color_changed = true;
}

void Renderer::DrawQuad(int x, int y, int w, int h)
{
   // Do lazy brush initialization
   if (!m_brush || m_color_changed)
   {
      if (m_brush) DeleteObject(m_brush);
      m_brush = CreateSolidBrush(ToCOLORREF(m_c));

      m_color_changed = false;
   }

   RECT r = { x, y, x+w, y+h };
   FillRect(m_hdc, &r, m_brush);
}
