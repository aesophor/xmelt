// Copyright (c) 2020 Marco Wang <m.aesophor@gmail.com>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef struct rect_s { int x, y, w, h; } rect_t;
typedef unsigned long pixel_t;  // 0xAARRGGBB


static int
rand_between(int lo, int hi) {
  return rand() % (hi + 1 - lo) + lo;
}

static void
pixelcpy(XImage *img, rect_t src, rect_t dest) {
  pixel_t *buf = (pixel_t *) malloc(src.w * src.h * sizeof(pixel_t));

  for (int y = 0; y < src.h; y++) {
    for (int x = 0; x < src.w; x++) {
      int real_x = src.x + x;
      int real_y = src.y + y;

      if (real_x < 0 || real_x >= img->width || real_y < 0 || real_y >= img->height) {
        continue;
      }
      buf[y * src.w + x] = XGetPixel(img, real_x, real_y);
    }
  }

  for (int y = 0; y < dest.h; y++) {
    for (int x = 0; x < dest.w; x++) {
      int real_x = dest.x + x;
      int real_y = dest.y + y;

      if (real_x < 0 || real_x >= img->width || real_y < 0 || real_y >= img->height) {
        continue;
      }
      XPutPixel(img, real_x, real_y, buf[y * dest.w + x]);
    }
  }

  free(buf);
}

static void
melt(XImage *img) {
  rect_t src = {
    .x = rand_between(0, img->width),
    .y = rand_between(0, img->height),
    .w = rand_between(10, 50),
    .h = rand_between(5, 100)
  };

  rect_t dest = {
    .x = src.x,
    .y = src.y + rand_between(5, 20),  // create a random y offset
    .w = src.w,
    .h = src.h
  };

  pixelcpy(img, src, dest);
}


int
main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;
  XImage *img = NULL;
  Display *dpy = NULL;
  Window root = None;
  Window window = None;
  Atom atom_wm_state = None; 
  Atom atom_wm_state_fullscreen = None;
  XWindowAttributes attr;
 
  dpy = XOpenDisplay(NULL);
  if (!dpy) {
    ret = EXIT_FAILURE;
    perror("XOpenDisplay failed");
    goto end;
  }

  root = RootWindow(dpy, 0);
  if (!root) {
    ret = EXIT_FAILURE;
    perror("Can't get the root window");
    goto end;
  }

  XGetWindowAttributes(dpy, root, &attr);
  img = XGetImage(dpy, root, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
  if (!img) {
    ret = EXIT_FAILURE;
    perror("Can't XGetImage");
    goto end;
  }

  window = XCreateSimpleWindow(dpy, root, 0, 0, attr.width, attr.height, 1, 0, 0);
  XMapWindow(dpy, window);

  // Make our window fullscreen
  atom_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False); 
  atom_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
  XSetWindowBorderWidth(dpy, window, 0);
  XMoveResizeWindow(dpy, window, 0, 0, attr.width, attr.height);
  XChangeProperty(dpy, window, atom_wm_state, XA_ATOM, 32, PropModeReplace,
                  (unsigned char*) &atom_wm_state_fullscreen, True);

  // Enter melting loop.
  srand(time(NULL));
  while (1) {
    XPutImage(dpy, window, DefaultGC(dpy, 0), img, 0, 0, 0, 0, attr.width, attr.height);
    melt(img);
  }

end:
  if (img) {
    XDestroyImage(img);
  }
  if (dpy) {
    XCloseDisplay(dpy);
  }
  return ret;
}
