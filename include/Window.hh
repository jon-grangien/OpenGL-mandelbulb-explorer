#ifndef MANDELBULB_WINDOW_H
#define MANDELBULB_WINDOW_H

#include "types.hh"

class Window {
  unsigned int width = 640, height = 640;

  void static onClose(GLFWwindow *win);
  void static error_callback(int error, const char *description);

 public:
  GLFWwindow *window;
  bool init(FrameBufferSizeCallback resizeCallback);
};

#endif //MANDELBULB_WINDOW_H
