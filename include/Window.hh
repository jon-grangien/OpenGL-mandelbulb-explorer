#ifndef MANDELBULB_WINDOW_H
#define MANDELBULB_WINDOW_H

#include "types.hh"

class Window {
  GLFWwindow *window;
  unsigned int width = 640, height = 640;
  unsigned int nbFrames = 0;

  void static onClose(GLFWwindow *win);
  void static error_callback(int error, const char *description);

  ProcessInputFunc inputFunc;
  DisplayFunc displayFunc;
 public:
  bool logPerformance = false, logCoordinates = false;

  bool init(FrameBufferSizeCallback resizeCallback,
            ProcessInputFunc inputFunc,
            DisplayFunc dispFunc);
  void display();
  GLFWwindow *getHandle() { return window; };

};

#endif //MANDELBULB_WINDOW_H
