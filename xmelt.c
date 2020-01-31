// Copyright (c) 2020 Marco Wang <m.aesophor@gmail.com>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>  // time
#include <unistd.h>  // sleep, usleep

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef struct rect_s { int x, y, w, h; } rect_t;
typedef unsigned long pixel_t;  // 0xAARRGGBB


static void
die(const char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

static void
fullscreen_xwindow(Display* dpy, Window window, int width, int height) {
  XSetWindowBorderWidth(dpy, window, 0);
  XMoveResizeWindow(dpy, window, 0, 0, width, height);

  Atom atom_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
  Atom atom_wm_state_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

  XChangeProperty(dpy, window, atom_wm_state, XA_ATOM, 32, PropModeReplace,
                  (unsigned char*) &atom_wm_state_fullscreen, True);
}

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
  XImage *img;
  Display *dpy;
  Window root;
  XWindowAttributes attr;

  dpy = XOpenDisplay(NULL);
  if (!dpy) {
    die("XOpenDisplay failed");
  }

  root = RootWindow(dpy, 0);
  if (!root) {
    die("Can't get the root window");
  }

  XGetWindowAttributes(dpy, root, &attr);
  img = XGetImage(dpy, root, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
  if (!img) {
    die("Can't XGetImage");
  }

  srand(time(NULL));

  Window window = XCreateSimpleWindow(dpy, root, 0, 0, attr.width, attr.height, 1, 0, 0);
  XSelectInput(dpy, window, ButtonPressMask | ExposureMask);
  XMapWindow(dpy, window);
  fullscreen_xwindow(dpy, window, attr.width, attr.height);
  
  while (1) {
    XPutImage(dpy, window, DefaultGC(dpy, 0), img, 0, 0, 0, 0, attr.width, attr.height);
    melt(img);
  }

  XDestroyImage(img);
  XCloseDisplay(dpy);
  return EXIT_SUCCESS;
}
