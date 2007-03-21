// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __IMAGE_H
#define __IMAGE_H

#include <Windows.h>
#include <string>

#include "Renderer.h"

enum ImageErrorCode
{
   Error_CannotLoadFile,
   Error_CannotLoadResource,
   Error_CannotCreateNewImage,
   Error_CannotDetermineSize,

   Error_AlreadyInDrawingMode,
   Error_InvalidDrawingModeExit,

   Error_AlreadyDrawingOn,
   Error_InvalidDrawingOnExit,

   Error_AttemptedDrawWhileNotInDrawingMode
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

   // Create a new solid color image of the specified size
   Image(int width, int height, Color initial_fill = ToColor(0,0,0));

   ~Image();

   // Transparency is disabled by default.  Bottom-right
   // pixel of image is used as transparent color.
   void EnableTransparency();
   void DisableTransparency();

   // Wrap any draw calls inside a matched pair of beginDrawing/endDrawing
   // calls.  Nesting is not allowed.
   void beginDrawing(Renderer &renderer) const;
   void endDrawing() const;

   // You can also draw directly on an image's surface using the following
   // matched pair of calls.  Nesting is not allowed.
   Renderer beginDrawingOn();
   void endDrawingOn();

   // Convenience drawing function to draw entire image
   // at the specified destination coordinates
   void draw(int x, int y) const;

   void draw(int x, int y, int width, int height, int src_x, int src_y) const;

   int getWidth() const { return m_width; }
   int getHeight() const { return m_height; }

private:
   // Disable copying
   Image(const Image &i);

   void GetDimensions();
   void BuildMask();
   void CopyOriginalToWorkingImage();

   void ThrowWithLastError(ImageErrorCode code);

   HBITMAP m_original_image;
   HBITMAP m_image;
   HBITMAP m_image_mask;

   bool m_transparency_enabled;

   int m_width;
   int m_height;

   // State for drawing this image onto other surfaces
   mutable bool m_drawing;
   mutable HDC m_image_dc;
   mutable HDC m_cached_destination_dc;

   // State for drawing on the surface of this image
   bool m_drawing_on;
   HDC m_draw_on_dc;
   HGDIOBJ m_previous_draw_on_obj;

   static HINSTANCE m_global_hinstance;
};

#endif