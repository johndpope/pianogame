// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "Renderer.h"
#include "Tga.h"

#include <gl\gl.h>

Renderer::Renderer(HDC hdc) : m_hdc(hdc), m_xoffset(0), m_yoffset(0)
{
}

void Renderer::SetColor(Color c)
{
   SetColor(c.r, c.g, c.b, c.a);
}

void Renderer::SetColor(int r, int g, int b, int a)
{
   glColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

void Renderer::DrawQuad(int x, int y, int w, int h)
{
   glBegin(GL_QUADS);
   glVertex3i(   x + m_xoffset,   y + m_yoffset, 0);
   glVertex3i( x+w + m_xoffset,   y + m_yoffset, 0);
   glVertex3i( x+w + m_xoffset, y+h + m_yoffset, 0);
   glVertex3i(   x + m_xoffset, y+h + m_yoffset, 0);
   glEnd();
}

void Renderer::DrawTga(const Tga *tga, int x, int y) const
{
   DrawTga(tga, x, y, (int)tga->GetWidth(), (int)tga->GetHeight(), 0, 0);
}

void Renderer::DrawTga(const Tga *tga, int in_x, int in_y, int width, int height, int src_x, int src_y) const
{
   const int x = in_x + GetXoffset();
   const int y = in_y + GetYoffset();

   const double tx = static_cast<double>(src_x) / static_cast<double>(tga->GetWidth());
   const double ty = -static_cast<double>(src_y) / static_cast<double>(tga->GetHeight());
   const double tw = static_cast<double>(width) / static_cast<double>(tga->GetWidth());
   const double th = -static_cast<double>(height)/ static_cast<double>(tga->GetHeight());

   glBindTexture(GL_TEXTURE_2D, tga->GetId());

   glBegin(GL_QUADS);
   glTexCoord2d(   tx,    ty); glVertex3i(      x,        y, 0);
   glTexCoord2d(   tx, ty+th); glVertex3i(      x, y+height, 0);
   glTexCoord2d(tx+tw, ty+th); glVertex3i(x+width, y+height, 0);
   glTexCoord2d(tx+tw,    ty); glVertex3i(x+width,        y, 0);
   glEnd();

   glBindTexture(GL_TEXTURE_2D, 0);
}
