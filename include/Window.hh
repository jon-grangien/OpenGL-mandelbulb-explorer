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
  Window(unsigned int w, unsigned int h) : width(w), height(h) {};
  ~Window();

  bool init(FrameBufferSizeCallback resizeCallback,
            ProcessInputFunc inputFunc,
            DisplayFunc dispFunc);

  void display();
  void setResolution(unsigned int w, unsigned int h) {
    width = w;
    height = h;
  }

  GLFWwindow *getHandle() { return window; }
  unsigned int getWidth() { return width; }
  unsigned int getHeight() { return height; }

};

#endif //MANDELBULB_WINDOW_H
