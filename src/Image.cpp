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
   case Error_CannotCreateNewImage:                message = L"Cannot create a new blank image."; break;
   case Error_CannotDetermineSize:                 message = L"Cannot determine image dimensions."; break;

   case Error_AlreadyInDrawingMode:                message = L"Cannot enter image draw mode more than once."; break;
   case Error_InvalidDrawingModeExit:              message = L"Cannot leave image draw mode before entering it."; break;

   case Error_AlreadyDrawingOn:                    message = L"Cannot enter draw-on mode more than once."; break;
   case Error_InvalidDrawingOnExit:                message = L"Cannot leave draw-on mode before entering it."; break;

   case Error_AttemptedDrawWhileNotInDrawingMode:  message = L"Cannot draw before entering image draw mode."; break;

   default:                                        message = WSTRING(L"Unknown ImageError Code (" << m_error << L")."); break;
   }

   if (m_additional.length() > 0)
   {
      message += WSTRING(L"\n\nAdditional Information: ") + m_additional;
   }

   return message;
}



Image::Image(const wstring &filename)
: m_drawing(false), m_drawing_on(false), m_transparency_enabled(false), m_image(0), m_image_mask(0)
{
   m_original_image = (HBITMAP)LoadImage(0, filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
   if (!m_original_image) ThrowWithLastError(Error_CannotLoadFile);

   GetDimensions();
   CopyOriginalToWorkingImage();
}

Image::Image(HINSTANCE module_instance, const wstring &resource_name)
   : m_drawing(false), m_drawing_on(false), m_transparency_enabled(false), m_image(0), m_image_mask(0)
{
   m_original_image = (HBITMAP)LoadImage(module_instance, resource_name.c_str(), IMAGE_BITMAP, 0, 0, 0);
   if (!m_original_image) ThrowWithLastError(Error_CannotLoadResource);

   GetDimensions();
   CopyOriginalToWorkingImage();
}

Image::Image(int width, int height, COLORREF initial_fill)
   : m_drawing(false), m_drawing_on(false), m_transparency_enabled(false), m_image(0), m_image_mask(0)
{
   HDC screen = CreateDC(L"DISPLAY", 0, 0, 0);
   m_original_image = CreateCompatibleBitmap(screen, width, height);
   DeleteDC(screen);
   if (!m_original_image) ThrowWithLastError(Error_CannotCreateNewImage);

   RECT r = { 0, 0, width, height };
   HBRUSH fill_brush = CreateSolidBrush(initial_fill);
   HDC hdc = CreateCompatibleDC(0);

   // Fill with the transparent color
   HBITMAP previous_bitmap = (HBITMAP)SelectObject(hdc, m_original_image);
   FillRect(hdc, &r, fill_brush);
   SelectObject(m_image_dc, previous_bitmap);

   DeleteObject(fill_brush);
   DeleteDC(hdc);

   GetDimensions();
   CopyOriginalToWorkingImage();
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

void Image::GetDimensions()
{
   BITMAP bm;

   if (GetObject(m_original_image, sizeof(bm), &bm) == 0) ThrowWithLastError(Error_CannotDetermineSize);
   m_width = bm.bmWidth;
   m_height = bm.bmHeight;
}

void Image::CopyOriginalToWorkingImage()
{
   if (!m_image)
   {
      HDC screen = CreateDC(L"DISPLAY", 0, 0, 0);
      m_image = CreateCompatibleBitmap(screen, m_width, m_height);
      DeleteDC (screen);
   }

   HDC dc_working = CreateCompatibleDC(0);
   HDC dc_original = CreateCompatibleDC(0);
   
   HBITMAP previous_working = (HBITMAP)SelectObject(dc_working, m_image);
   HBITMAP previous_original = (HBITMAP)SelectObject(dc_original, m_original_image);

   BitBlt(dc_working, 0, 0, m_width, m_height, dc_original, 0, 0, SRCCOPY);

   SelectObject(dc_working, previous_working);
   SelectObject(dc_original, previous_original);

   DeleteDC(dc_working);
   DeleteDC(dc_original);
}

void Image::EnableTransparency()
{
   m_transparency_enabled = true;
   BuildMask();
}

void Image::DisableTransparency()
{
   m_transparency_enabled = false;
}

void Image::BuildMask()
{
   CopyOriginalToWorkingImage();
   if (!m_image_mask) m_image_mask = CreateBitmap(m_width, m_height, 1, 1, 0);

   HDC dc_image = CreateCompatibleDC(0);
   HDC dc_mask = CreateCompatibleDC(0);

   // Select the two bitmaps into the new DCs, and grab whatever might
   // be in the new DCs, and hold until we're finished with them.
   HBITMAP old_image_bitmap = (HBITMAP)SelectObject(dc_image, m_image);
   HBITMAP old_mask_bitmap = (HBITMAP)SelectObject(dc_mask, m_image_mask);

   // Grab the alpha color from the bottom right pixel, and set it as the 
   // background color of the original graphics
   COLORREF alpha_color = GetPixel(dc_image, m_width - 1, m_height - 1);
   SetBkColor(dc_image, alpha_color);

   // Straight copy the image over to our mask bitmap
   BitBlt(dc_mask, 0, 0, m_width, m_height, dc_image, 0, 0, SRCCOPY);

   // Set the text color to white for some reason and get back what was there
   SetTextColor(dc_image, 0x00FFFFFF);

   // Change the background color of the original graphics to pure black (we don't need
   // to save what was there, because we just set it to the alpha_color ourselves)
   SetBkColor(dc_image, 0x00000000);

   // Copy the mask onto the original image
   BitBlt(dc_image, 0, 0, m_width, m_height, dc_mask, 0, 0, SRCAND);

   // Restore the text color, background color and whatever
   // might have been in the DCs when we first made them
   SelectObject(dc_image, old_image_bitmap);
   SelectObject(dc_mask, old_mask_bitmap);

   // Clean up the DCs
   DeleteDC(dc_image);
   DeleteDC(dc_mask);
}

Image::~Image()
{
   DeleteObject(m_original_image);
   DeleteObject(m_image);
   DeleteObject(m_image_mask);
}

void Image::draw(int x, int y) const
{
   draw(x, y, m_width, m_height, 0, 0);
}

void Image::draw(int x, int y, int width, int height, int src_x, int src_y) const
{
   if (!m_drawing) throw ImageError(Error_AttemptedDrawWhileNotInDrawingMode);

   // Select the mask into the new DC. Then AND the mask over to the destination.
   HBITMAP previous_bitmap = (HBITMAP)SelectObject(m_image_dc, m_image_mask);

   if (m_transparency_enabled)
   {
      BitBlt(m_cached_destination_dc, x, y, width, height, m_image_dc, src_x, src_y, SRCAND);
   }

   // Swap out the selected bitmap for the actual graphics and draw it on
   // the destination, ORing it with the mask.
   SelectObject(m_image_dc, m_image);

   DWORD raster_op = (m_transparency_enabled ? SRCPAINT : SRCCOPY);
   BitBlt(m_cached_destination_dc, x, y, width, height, m_image_dc, src_x, src_y, raster_op);

   // Reselect whatever was in the DC before this call.
   SelectObject(m_image_dc, previous_bitmap);
}

void Image::beginDrawing(Renderer &renderer) const
{
   if (m_drawing) throw ImageError(Error_AlreadyInDrawingMode);

   m_cached_destination_dc = renderer.GetHdc();
   m_image_dc = CreateCompatibleDC(0);

   m_drawing = true;
}

void Image::endDrawing() const
{
   if (!m_drawing) throw ImageError(Error_InvalidDrawingModeExit);

   DeleteDC(m_image_dc);

   m_drawing = false;
}

Renderer Image::beginDrawingOn()
{
   if (m_drawing_on) throw ImageError(Error_AlreadyDrawingOn);

   m_draw_on_dc = CreateCompatibleDC(0);
   m_previous_draw_on_obj = SelectObject(m_draw_on_dc, m_original_image);

   SetBkMode(m_draw_on_dc, TRANSPARENT);

   m_drawing_on = true;
   return Renderer(m_draw_on_dc);
}

void Image::endDrawingOn()
{
   if (!m_drawing_on) throw ImageError(Error_InvalidDrawingOnExit);

   SelectObject(m_draw_on_dc, m_previous_draw_on_obj);
   DeleteDC(m_draw_on_dc);

   m_drawing_on = false;

   CopyOriginalToWorkingImage();

   // Rebuild our transparency mask
   if (m_transparency_enabled) BuildMask();
}
