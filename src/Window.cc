#include <cstdio>
#include <iostream>
#include "Window.hh"

bool Window::init(FrameBufferSizeCallback resizeCallback) {
  if (!glfwInit())
    std::cout << "Error: glfw failed to init\n";

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

  window = glfwCreateWindow(width, height, "Mandelbulb explorer", nullptr, nullptr);
  if (!window) {
    std::cout << "Error: glfw failed create window\n";
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSetWindowCloseCallback(window, onClose);
  resizeCallback(window, width, height); // call it once
  return true;
}

void Window::onClose(GLFWwindow *win) {
  std::cout << "Window closed\n";
}

void Window::error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}
