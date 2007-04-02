// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "Image.h"
#include "Renderer.h"
#include "string_util.h"

using namespace std;

HINSTANCE Image::m_global_hinstance = 0;

std::wstring ImageError::GetErrorDescription() const
{
   wstring message;

   switch (m_error)
   {
   case Error_CannotLoadFile:                      message = L"Cannot load image from file."; break;
   case Error_CannotLoadResource:                  message = L"Cannot load image from resource."; break;
   case Error_CannotDetermineSize:                 message = L"Cannot determine image dimensions."; break;

   default:                                        message = WSTRING(L"Unknown ImageError Code (" << m_error << L")."); break;
   }

   if (m_additional.length() > 0)
   {
      message += WSTRING(L"\n\nAdditional Information: ") + m_additional;
   }

   return message;
}



Image::Image(const wstring &filename)
: m_image(0), m_texture(0)
{
   m_image = (HBITMAP)LoadImage(0, filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
   if (!m_image) ThrowWithLastError(Error_CannotLoadFile);

   Init();
}

Image::Image(HINSTANCE module_instance, const wstring &resource_name)
   : m_image(0), m_texture(0)
{
   m_image = (HBITMAP)LoadImage(module_instance, resource_name.c_str(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
   if (!m_image) ThrowWithLastError(Error_CannotLoadResource);

   Init();
}

void Image::ThrowWithLastError(ImageErrorCode code)
{
   LPWSTR raw_message = 0;
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      0, GetLastError(), 0, (LPWSTR) &raw_message, 0, 0);

   std::wstring error_message = raw_message;
   LocalFree(raw_message);

   throw ImageError(code, error_message);
}

void Image::Init()
{
   BITMAP bm;

   if (GetObject(m_image, sizeof(bm), &bm) == 0) ThrowWithLastError(Error_CannotDetermineSize);
   m_width = bm.bmWidth;
   m_height = bm.bmHeight;

   glGenTextures(1, &m_texture);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glBindTexture(GL_TEXTURE_2D, m_texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, 3, bm.bmWidth, bm.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bm.bmBits);
}

Image::~Image()
{
   DeleteObject(m_image);
   glDeleteTextures(1, &m_texture);
}

void Image::draw(Renderer &r, int x, int y) const
{
   draw(r, x, y, m_width, m_height, 0, 0);
}

void Image::draw(Renderer &r, int in_x, int in_y, int width, int height, int src_x, int src_y) const
{
   const int x = in_x + r.GetXoffset();
   const int y = in_y + r.GetYoffset();

   const double tx = static_cast<double>(src_x) / static_cast<double>(m_width);
   const double ty = -static_cast<double>(src_y) / static_cast<double>(m_height);
   const double tw = static_cast<double>(width) / static_cast<double>(m_width);
   const double th = -static_cast<double>(height)/ static_cast<double>(m_height);

   glBindTexture(GL_TEXTURE_2D, m_texture);

   glBegin(GL_QUADS);
   glTexCoord2d(   tx,    ty); glVertex3i(      x,        y, 0);
   glTexCoord2d(   tx, ty+th); glVertex3i(      x, y+height, 0);
   glTexCoord2d(tx+tw, ty+th); glVertex3i(x+width, y+height, 0);
   glTexCoord2d(tx+tw,    ty); glVertex3i(x+width,        y, 0);
   glEnd();

   glBindTexture(GL_TEXTURE_2D, 0);
}



void Image::drawTga(Renderer &r, const Tga *tga, int x, int y)
{
   drawTga(r, tga, x, y, (int)tga->GetWidth(), (int)tga->GetHeight(), 0, 0);
}

void Image::drawTga(Renderer &r, const Tga *tga, int in_x, int in_y, int width, int height, int src_x, int src_y)
{
   const int x = in_x + r.GetXoffset();
   const int y = in_y + r.GetYoffset();

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
