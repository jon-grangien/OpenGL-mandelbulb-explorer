#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void error_callback(int error, const char* description);

int main()
{
  if (!glfwInit())
    std::cout << "Error: glfw failed to init" << std::endl;

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  glm::vec4 testVec = glm::vec4(glm::vec3(0.0), 1.0);

  GLFWwindow* window = glfwCreateWindow(640, 480, "Simple window", NULL, NULL);
  if (!window)
  {
    std::cout << "Error: glfw failed create window" << std::endl;
    glfwTerminate();
  }

  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window))
  {
    // Main loop
    std::cout << testVec.length() << std::endl;
  }

  glfwDestroyWindow(window);
  glfwTerminate();
	return 0;
}

void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

