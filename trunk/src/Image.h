// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __IMAGE_H
#define __IMAGE_H

#include <Windows.h>
#include <string>

#include <gl\gl.h>

#include "Renderer.h"

enum ImageErrorCode
{
   Error_CannotLoadFile,
   Error_CannotLoadResource,
   Error_CannotCreateNewImage,
   Error_CannotDetermineSize,
};

class ImageError : public std::exception
{
public:
   ImageError(ImageErrorCode error) : m_error(error) { }
   ImageError(ImageErrorCode error, const std::wstring &additional_information)
      : m_error(error), m_additional(additional_information)
   { }

   std::wstring GetErrorDescription() const;

   const ImageErrorCode m_error;
   const std::wstring m_additional;

private:
   ImageError operator=(const ImageError&);
};


// Loads a bitmap from resource or file.  Easily draw portions of the
// image to a Win32 DC with color keying.  Bottom-right pixel is used
// as the color key.
class Image
{
public:
   // Convenience functions so global hinstance
   // can be avoided, at least in small part.
   static void SetGlobalModuleInstance(HINSTANCE inst) { m_global_hinstance = inst; }
   static HINSTANCE GetGlobalModuleInstance() { return m_global_hinstance; }

   // Load a bitmap from a file
   Image(const std::wstring &filename);

   // Load a bitmap from an exe or dll resource
   //
   // Consider using GetGlobalModuleInstance()
   Image(HINSTANCE module_instance, const std::wstring &resource_name);

   ~Image();

   // Convenience drawing function to draw entire image
   // at the specified destination coordinates
   void draw(Renderer &r, int x, int y) const;

   void draw(Renderer &r, int x, int y, int width, int height, int src_x, int src_y) const;

   int getWidth() const { return m_width; }
   int getHeight() const { return m_height; }

private:
   // Disable copying
   Image(const Image &);

   void Init();

   void ThrowWithLastError(ImageErrorCode code);

   HBITMAP m_image;
   GLuint m_texture;

   int m_width;
   int m_height;

   static HINSTANCE m_global_hinstance;
};

#endif