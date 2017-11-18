#include <iostream>
#include "utils.hh"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "types.hh"
#include <GLFW/glfw3.h>

// Declarations
bool initGlfw();
void onClose(GLFWwindow* win);
void resize(GLFWwindow* win, GLsizei w, GLsizei h);
void sphericalToCartesian(float r, float theta, float phi, float &x, float &y, float &z);
void processInput(GLFWwindow *window);
void error_callback(int error, const char* description);

// Constants
#define PI   3.14159265358979323846f
#define PI_2 1.57079632679f
#define TWO_PI 6.28318530718f

float STEP_SIZE = 0.001f;
int WIN_WIDTH = 640;
int WIN_HEIGHT = WIN_WIDTH;
float NEAR_PLANE = 0.1f;
float FAR_PLANE = 100.0f;
float COORDINATES_STEP = 0.005f;
float COORDINATES_STEP_HALF = 0.0025f;
float COORDINATES_STEP_DOUBLE = 0.01f;

// Arg variables
float maxRaySteps = 700.0;
float minDistance = 0.0001;
float mandelIters = 15;
float bailLimit = 2.5;
float power = 8.0;

// App state
bool logPerformance = false;
bool logCoordinates = false;
bool shouldUpdateCoordinates = true; // True initially to first set spherical to cartesian

GLuint shader;
GLuint vbo, vao;
GLFWwindow* window;
mat4 projectionMatrix = glm::perspective(90.0f, (GLfloat) WIN_WIDTH / (GLfloat) WIN_HEIGHT, NEAR_PLANE, FAR_PLANE);

// Coordinates for eye pos
float defaultR = 1.3f, defaultTheta = 0.0f, defaultPhi = 0.0f;
float r = defaultR;
float theta = defaultTheta;
float phi = defaultPhi;
float x = 0.0f, y = 0.0f, z = 0.0f;

// View matrix set up with glm
vec3 eye = vec3(0.0f, 0.0f, 1.0f);
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

mat4 inverseVP = glm::inverse(viewMatrix) * glm::inverse(projectionMatrix);

GLfloat currentTime = 0.0;
GLfloat screenRatio;
auto screenSize = vec2(0.0);

int main(int argc,  char* argv[]) {

  // Handle args
  int graphicsSetting = 1;
  int OK = utils::handleArgs(argc, argv, graphicsSetting, logPerformance, logCoordinates);
  if (OK < 0) return -1;

  switch(graphicsSetting) {
    case 0:
      maxRaySteps = 500.0;
      minDistance = 0.01;
      mandelIters = 5;
      bailLimit = 2.5;
      power = 4.0;
      break;
    case 1:
    default:
      break;
    case 2:
      maxRaySteps = 1300.0;
      minDistance = 0.0001;
      mandelIters = 30;
      bailLimit = 2.5;
      power = 8.0;
      break;
  }

  std::cout << "Keys:\n";
  std::cout << "Q: Quit\n";
  std::cout << "L: Reload shader files\n";
  std::cout << "WASD: Movement around center\n";
  std::cout << "Z: Zoom out\n";
  std::cout << "X: Zoom in\n";
  std::cout << "R: Reset position\n";

  auto glfwOk = initGlfw();
  auto err = glewInit();

  if (!glfwOk)
    return EXIT_FAILURE;
  if (err != GLEW_OK)
    std::cout << "Error: GLEW failed to init\n";

  glDisable(GL_DEPTH_TEST);

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

  double lastTime = (float) glfwGetTime();
  int nbFrames = 0;

  while (!glfwWindowShouldClose(window)) {
    processInput(window);
    currentTime = (float) glfwGetTime();

    if (logPerformance && logCoordinates) {
      nbFrames++;
      if (currentTime - lastTime >= 1.0) {
        printf("\r%.1f ms/frame, %i FPS, r: %.1f, theta: %.1f, phi: %.1f",
               1000.0 / double(nbFrames),
               nbFrames,
               r,
               theta,
               phi);
        fflush(stdout);
        nbFrames = 0;
        lastTime += 1.0;
      }
    } else if (logPerformance) {
      nbFrames++;

      if (currentTime - lastTime >= 1.0) {
        printf("\r%.1f ms/frame, %i FPS", 1000.0 / double(nbFrames), nbFrames);
        fflush(stdout);
        nbFrames = 0;
        lastTime += 1.0;
      }
    } else if (logCoordinates) {
      printf("\nr: %.1f, theta: %.1f, phi: %.1f and x: %.1f, y: %.1f, z: %.1f", r, theta, phi, x, y, z);
      fflush(stdout);
    }

    if (shouldUpdateCoordinates) {
      sphericalToCartesian(r, theta, phi, x, y, z);
      eye = vec3(x, y, z);
      viewMatrix = glm::lookAt(eye, center, up);
      inverseVP = glm::inverse(viewMatrix) * glm::inverse(projectionMatrix);

      shouldUpdateCoordinates = false;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_inverseVP"), 1, GL_FALSE, glm::value_ptr(inverseVP));
    glUniform1fv(glGetUniformLocation(shader, "u_nearPlane"), 1, &NEAR_PLANE);
    glUniform1fv(glGetUniformLocation(shader, "u_farPlane"), 1, &FAR_PLANE);
    glUniform1fv(glGetUniformLocation(shader, "u_time"), 1, &currentTime);
    glUniform1fv(glGetUniformLocation(shader, "u_screenRatio"), 1, &screenRatio);
    glUniform2fv(glGetUniformLocation(shader, "u_screenSize"), 1, glm::value_ptr(screenSize));
    glUniform1fv(glGetUniformLocation(shader, "u_stepSize"), 1, &STEP_SIZE);

    // Mandel setup
    glUniform1fv(glGetUniformLocation(shader, "u_maxRaySteps"), 1, &maxRaySteps);
    glUniform1fv(glGetUniformLocation(shader, "u_minDistance"), 1, &minDistance);
    glUniform1fv(glGetUniformLocation(shader, "u_mandelIters"), 1, &mandelIters);
    glUniform1fv(glGetUniformLocation(shader, "u_bailLimit"), 1, &bailLimit);
    glUniform1fv(glGetUniformLocation(shader, "u_power"), 1, &power);

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

  window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Mandelbulb explorer", nullptr, nullptr);
  if (!window) {
    std::cout << "Error: glfw failed create window\n";
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resize);
  glfwSetWindowCloseCallback(window, onClose);
  resize(window, WIN_WIDTH, WIN_HEIGHT);
  return true;
}

void resize(GLFWwindow* win, int w, int h) {
  //std::cout << "\nresized to " << w << ", " << h << std::endl;
  glViewport(0, 0, w, h);
  screenSize.x = (GLfloat) w;
  screenSize.y = (GLfloat) h;
  screenRatio = screenSize.x / screenSize.y;
  projectionMatrix = glm::perspective(90.0f, screenRatio, NEAR_PLANE, FAR_PLANE);
  inverseVP = glm::inverse(viewMatrix) * glm::inverse(projectionMatrix);
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
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    shader = utils::loadShaders("../shaders/mandel_raymarch.vert" , "../shaders/mandel_raymarch.frag");

  // Movement
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    theta -= COORDINATES_STEP;
    theta = std::max(0.0f, theta);
    theta = std::min(PI, theta);
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    phi -= COORDINATES_STEP_DOUBLE;
    phi = std::max(0.0f, phi);
    phi = std::min(TWO_PI, phi);
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    theta += COORDINATES_STEP;
    theta = std::max(0.0f, theta);
    theta = std::min(PI, theta);
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    phi += COORDINATES_STEP_DOUBLE;
    phi = std::max(0.0f, phi);
    phi = std::min(TWO_PI, phi);
  }

  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    r -= COORDINATES_STEP_HALF;
  }

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    r += COORDINATES_STEP_HALF;
  }

  // Reset camera
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    r = defaultR;
    theta = defaultTheta;
    phi = defaultPhi;
  }
}

void sphericalToCartesian(float r, float theta, float phi, float &x, float &y, float &z) {
  x = r * sinf(theta) * cosf(phi);
  y = r * sinf(theta) * sinf(phi);
  z = r * cosf(theta);
}

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

