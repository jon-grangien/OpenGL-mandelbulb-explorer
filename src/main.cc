#include <iostream>
#include <utils.hh>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <types.hh>
#include <GLFW/glfw3.h>

// Declarations
void error_callback(int error, const char* description);
void initGlfw();
void resize(GLFWwindow* win, GLsizei w, GLsizei h);

GLuint shader;
GLFWwindow* window;
mat4 projectionMatrix;

// View matrix set up with glm
// Eye, center, up
mat4 viewMatrix = glm::lookAt(
    vec3(-5.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f)
);

int main() {
  initGlfw();
  auto err = glewInit();
  if (err != GLEW_OK)
    std::cout << "Error: GLEW failed to init\n";

  shader = utils::loadShaders("../shaders/mandel_raymarch.vert" , "../shaders/mandel_raymarch.frag");

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_TRUE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelViewMatrix"), 1, GL_TRUE, glm::value_ptr(viewMatrix));

    // Need position, normal(?), texcoord
    glUseProgram(shader);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void initGlfw() {
  if (!glfwInit())
    std::cout << "Error: glfw failed to init\n";

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(640, 480, "Mandelbulb explorer", nullptr, nullptr);
  if (!window) {
    std::cout << "Error: glfw failed create window\n";
    glfwTerminate();
  }

  glfwSetWindowSizeCallback(window, resize);
  glfwMakeContextCurrent(window);
}

void resize(GLFWwindow* win, int w, int h) {
  glViewport(0, 0, w, h);
  GLfloat ratio = (GLfloat) w / (GLfloat) h;
  projectionMatrix = glm::perspective(90.0f, ratio, 1.0f, 1000.f);
}

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

