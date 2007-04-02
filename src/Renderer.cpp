// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "Renderer.h"

#include <gl\gl.h>

Renderer::Renderer(HDC hdc) : m_hdc(hdc), m_xoffset(0), m_yoffset(0)
{
}

void Renderer::SetColor(Color c)
{
   SetColor(c.r, c.g, c.b);
}

void Renderer::SetColor(int r, int g, int b)
{
   glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
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
