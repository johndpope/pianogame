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
class Text;
class TextWriter;

struct Color
{
   int r, g, b, a;
};

class Renderer
{
public:

   static Color ToColor(int r, int g, int b, int a = 0xFF);

   Renderer(Context context);

   void SwapBuffers();

   // 0 will disable vsync, 1 will enable.  (In Windows, >1 will skip frames.)
   void SetVSyncInterval(int interval = 1);

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

private:

   // NOTE: These are used externally by the friend
   // class TextWriter (along with the context)
   int m_xoffset;
   int m_yoffset;

   Context m_context;
   
   friend class Text;
   friend class TextWriter;
};

#endif
