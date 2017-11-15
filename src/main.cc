#include <iostream>
#include <utils.hh>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <types.hh>
#include <GLFW/glfw3.h>

// Declarations
bool initGlfw();
void onClose(GLFWwindow* win);
void resize(GLFWwindow* win, GLsizei w, GLsizei h);
void processInput(GLFWwindow *window);
void error_callback(int error, const char* description);

float STEP_SIZE = 0.001f;

GLuint shader;
GLuint vbo, vao;
GLFWwindow* window;
mat4 projectionMatrix;

// View matrix set up with glm
vec3 eye = vec3(0.0f, 0.0f, -2.0f);
vec3 center = vec3(0.0f, 0.0f, 0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);
mat4 viewMatrix = glm::lookAt(eye, center, up);

const GLfloat quadArray[4][2] = {
  { -1.0f, -1.0f  },
  {  1.0f, -1.0f  },
  { -1.0f, 1.0f  },
  { 1.0f,  1.0f  }
};
mat4x2 quad = glm::make_mat4x2(&quadArray[0][0]);
mat4 modelViewMatrix = quad * viewMatrix;

GLfloat currentTime = 0.0;
GLfloat screenRatio;
auto screenSize = vec2(0.0);

int main() {
  auto glfwOk = initGlfw();
  auto err = glewInit();

  if (!glfwOk)
    return EXIT_FAILURE;
  if (err != GLEW_OK)
    std::cout << "Error: GLEW failed to init\n";

  std::cout << "Q: Quit\n";
  std::cout << "R: Reload shader files\n";

  glEnable(GL_DEPTH_TEST);

  shader = utils::loadShaders("../shaders/mandel_raymarch.vert" , "../shaders/mandel_raymarch.frag");
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &quad[0][0], GL_STATIC_DRAW);

  // Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Enable attribute index 0 as being used
  glEnableVertexAttribArray(0);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);
    currentTime = (float) glfwGetTime();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
    glUniform1fv(glGetUniformLocation(shader, "time"), 1, &currentTime);
    glUniform1fv(glGetUniformLocation(shader, "screenRatio"), 1, &screenRatio);
    glUniform1fv(glGetUniformLocation(shader, "screenSize"), 1, glm::value_ptr(screenSize));
    glUniform1fv(glGetUniformLocation(shader, "stepSize"), 1, &STEP_SIZE);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

bool initGlfw() {
  if (!glfwInit())
    std::cout << "Error: glfw failed to init\n";

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

  window = glfwCreateWindow(640, 480, "Mandelbulb explorer", nullptr, nullptr);
  if (!window) {
    std::cout << "Error: glfw failed create window\n";
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetWindowCloseCallback(window, onClose);
  return true;
}

void resize(GLFWwindow* win, int w, int h) {
  std::cout << "resized to " << w << ", " << h << std::endl;
  glViewport(0, 0, w, h);
  screenSize.x = (GLfloat) w;
  screenSize.y = (GLfloat) h;
  screenRatio = screenSize.x / screenSize.y;
  projectionMatrix = glm::perspective(90.0f, screenRatio, 0.1f, 100.f);
}

void onClose(GLFWwindow* win) {
  std::cout << "Window closed\n";
}

void processInput(GLFWwindow *window) {

  // Close
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Reload shader
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    shader = utils::loadShaders("../shaders/mandel_raymarch.vert" , "../shaders/mandel_raymarch.frag");
}

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

