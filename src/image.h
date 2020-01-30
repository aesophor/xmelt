// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef MELTDOWN_IMAGE_H_
#define MELTDOWN_IMAGE_H_

extern "C" {
#include <X11/Xlib.h>
}

#include <memory>
#include <string>

namespace meltdown {

class Image {
 public:
  Image(XImage* ximg);
  virtual ~Image() = default;

  struct Pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
  };

  void SaveAsPng(const std::string& dest) const;

 private:
  const unsigned short width_;
  const unsigned short height_;
  std::unique_ptr<Image::Pixelp[]> pixels_;
  XImage* ximg_;
};

}  // namespace meldown

#endif  // MELTDOWN_IMAGE_H_
