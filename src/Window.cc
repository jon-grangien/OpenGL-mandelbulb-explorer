#include <cstdio>
#include <iostream>
#include <imgui_impl_glfw_gl3.hh>
#include <imgui.h>
#include "Window.hh"

Window::~Window() {
  //ImGui_ImplGlfwGL3_Shutdown();
  //glfwDestroyWindow(window);
}

bool Window::init(FrameBufferSizeCallback resizeCallback,
                  ProcessInputFunc inputFunc,
                  DisplayFunc dispFunc) {
  this->inputFunc = inputFunc;
  this->displayFunc = dispFunc;
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

  ImGui_ImplGlfwGL3_Init(window, true);

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSetWindowCloseCallback(window, onClose);
  resizeCallback(window, width, height); // call it once
  return true;
}

void Window::display() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();
    inputFunc(window);

    displayFunc();

    ImGui::Render();
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::onClose(GLFWwindow *win) {
  std::cout << "Window closed\n";
}

void Window::error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}
