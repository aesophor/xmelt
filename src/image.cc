// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "image.h"

extern "C" {
#include <X11/Xutil.h>
#include <png.h>
}

#include <iostream>
#include <fstream>
#include <boost/scope_exit.hpp>

namespace meltdown {

Image::Image(XImage* ximg)
    : width_(ximg->width),
      height_(ximg->height),
      pixels_(std::make_unique<Image::Pixel[]>(width_ * height_)),
      ximg_(ximg) {

  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      unsigned long xpixel = XGetPixel(ximg_, x, y);

      Image::Pixel& pixel = pixels_[y * x + x];
      pixel.r = (xpixel & ximg->red_mask) >> 16;
      pixel.g = (xpixel & ximg->green_mask) >> 8;
      pixel.b = (xpixel & ximg->blue_mask);
    }
  }
}


void Image::SaveAsPng(const std::string& dest) const {
  FILE* fout = nullptr;
  png_structp png_ptr = nullptr;
  png_infop png_info_ptr = nullptr;
  png_bytep png_row = nullptr;


  // Open destination file where the image will be saved.
  fout = fopen(dest.c_str(), "wb");
  if (!fout) {
    std::cerr << "Could not open file for writing." << std::endl;
    return;
  }
  
  BOOST_SCOPE_EXIT(&fout) {
    fclose(fout);
  } BOOST_SCOPE_EXIT_END


  // Initialize png_ptr.
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png_info_ptr) {
    std::cerr << "png_structp alloc failed" << std::endl;
    return;
  }

  BOOST_SCOPE_EXIT(&png_ptr) {
    png_destroy_write_struct(&png_ptr, nullptr);
  } BOOST_SCOPE_EXIT_END


  // Initialize png_info_ptr.
  png_info_ptr = png_create_info_struct(png_ptr);
  if (!png_info_ptr) {
    std::cerr << "png_structp alloc failed" << std::endl;
    return;
  }

  BOOST_SCOPE_EXIT(&png_ptr, &png_info_ptr) {
    png_free_data(png_ptr, png_info_ptr, PNG_FREE_ALL, -1);
  } BOOST_SCOPE_EXIT_END


  // Set up exception handling.
  if (setjmp(png_jmpbuf(png_ptr))) {
    std::cerr << "failed to create the png image!" << std::endl;
    return;
  }

  BOOST_SCOPE_EXIT(png_row) {
    free(png_row);
  } BOOST_SCOPE_EXIT_END


  // Actually write shit to the fucking png file.
  png_init_io(png_ptr, fout);

  // Write header (8 bit color depth)
  png_set_IHDR(png_ptr, png_info_ptr, width_, height_,
      8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_png(png_ptr, png_info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
}

}  // namespace meltdown
