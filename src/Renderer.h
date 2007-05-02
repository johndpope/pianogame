// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __RENDERER_H
#define __RENDERER_H

#include "os_graphics.h"

#ifdef WIN32
typedef HDC Context;
#else
typedef AGLContext Context;
#endif

class Tga;

struct Color
{
   int r, g, b, a;
};

class Renderer
{
public:

   static void SetVSyncInterval(int interval = 1);
   static Color ToColor(int r, int g, int b, int a = 0xFF);

   Renderer(Context context);

   void SwapBuffers();

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
   Context GetContext() { return m_context; }

   // TODO: REMOVE!
   int GetXoffset() const { return m_xoffset; }
   int GetYoffset() const { return m_yoffset; }

private:
   int m_xoffset;
   int m_yoffset;

   Context m_context;
};

#endif
