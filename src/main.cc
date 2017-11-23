#include <iostream>
#include <GL/glew.h>
#include "utils.hh"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Window.hh"
#include "types.hh"
#include <imgui.h>
#include <GLFW/glfw3.h>

// Declarations
void sphericalToCartesian(float r, float theta, float phi, float &x, float &y, float &z);
void resizeCallback(GLFWwindow *win, int w, int h);
void processInput(GLFWwindow *window);
void display();

// Constants
#define PI     3.14159265358979323846f
#define TWO_PI 6.28318530718f

float STEP_SIZE = 0.001f;
unsigned int INITIAL_WIDTH = 800;
unsigned int INITIAL_HEIGHT = 640;
float NEAR_PLANE = 0.1f;
float FAR_PLANE = 100.0f;
float COORDINATES_STEP = 0.005f;
float COORDINATES_STEP_HALF = 0.0025f;
float COORDINATES_STEP_DOUBLE = 0.01f;

// Arg variables
float maxRaySteps = 1000.0;
float baseMinDistance = 0.00001;
float minDistance = baseMinDistance;
int minDistanceFactor = 0;
float mandelIters = 1000;
float bailLimit = 2.5;
float power = 8.0;

// App state
auto windowAdapter = Window(INITIAL_WIDTH, INITIAL_HEIGHT);
bool logPerformance = false;
bool logCoordinates = false;
bool shouldUpdateCoordinates = true; // True initially to first set spherical to cartesian
int nbFrames = 0;
int displayedFrames = 0;
float displayedMS = 0;
double lastTime = (float) glfwGetTime();

GLuint shader;
GLuint vbo, vao;
GLFWwindow *window;
mat4 projectionMatrix = glm::perspective(90.0f, (GLfloat) INITIAL_WIDTH / (GLfloat) INITIAL_HEIGHT, NEAR_PLANE, FAR_PLANE);

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
    {-1.0f, -1.0f},
    {1.0f, -1.0f},
    {-1.0f, 1.0f},
    {1.0f, 1.0f}
};
mat4x2 quad = glm::make_mat4x2(&quadArray[0][0]);

mat4 inverseVP = glm::inverse(viewMatrix) * glm::inverse(projectionMatrix);

GLfloat currentTime = 0.0;
GLfloat screenRatio;
auto screenSize = vec2(0.0);

int main(int argc, char *argv[]) {

  // Handle args
  int OK = utils::handleArgs(argc, argv, logCoordinates);
  if (OK < 0) return -1;

  std::cout << "Keys:\n"
            << "Q: Quit\n"
            << "L: Reload shader files\n"
            << "WASD: Movement around center\n"
            << "Z: Zoom out\n"
            << "X: Zoom in\n"
            << "R: Reset position\n";

  auto glfwOk = windowAdapter.init(resizeCallback, processInput, display);
  auto err = glewInit();

  if (!glfwOk)
    return EXIT_FAILURE;
  if (err != GLEW_OK)
    std::cout << "Error: GLEW failed to init\n";

  window = windowAdapter.getHandle();

  glDisable(GL_DEPTH_TEST);

  shader = utils::loadShaders("../shaders/mandel_raymarch.vert", "../shaders/mandel_raymarch.frag");
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &quad[0][0], GL_STATIC_DRAW);

  // Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Enable attribute index 0 as being used
  glEnableVertexAttribArray(0);

  windowAdapter.display();
  return 0;
}

void display() {
  currentTime = (float) glfwGetTime();

  if (logCoordinates) {
    printf("\nr: %.1f, theta: %.1f, phi: %.1f and x: %.1f, y: %.1f, z: %.1f", r, theta, phi, x, y, z);
    fflush(stdout);
  }

  // Perfomance calculations
  nbFrames++;
  if (currentTime - lastTime >= 1.0) {
    displayedMS = (float) 1000.0 / float(nbFrames);
    displayedFrames = nbFrames;
    nbFrames = 0;
    lastTime += 1.0;
  }

  if (shouldUpdateCoordinates) {
    sphericalToCartesian(r, theta, phi, x, y, z);
    eye = vec3(y, x, z);
    viewMatrix = glm::lookAt(eye, center, up);
    inverseVP = glm::inverse(viewMatrix) * glm::inverse(projectionMatrix);

    shouldUpdateCoordinates = false;
  }

  ImGui::SetNextWindowSize(ImVec2(350, 180));
  ImGui::Begin("Settings");
  ImGui::Text("Graphics values");
  ImGui::SliderFloat("Max ray steps", &maxRaySteps, 1.0f, 3000.0f);
  ImGui::SliderFloat("Mandel iters", &mandelIters, 0.0f, 3000.0f);
  ImGui::SliderInt("Min dist factor", &minDistanceFactor, -5, 5);

  if (minDistanceFactor < 0) {
    minDistance = baseMinDistance / ((float)(pow(10.0, abs(minDistanceFactor))));
  } else if (minDistanceFactor > 0) {
    minDistance = baseMinDistance * ((float)(pow(10.0, minDistanceFactor)));
  } else {
    minDistance = baseMinDistance;
  }

  ImGui::SliderFloat("Power", &power, 0.0f, 32.0f);
  ImGui::Separator();
  ImGui::Value("Min dist", minDistance, "%.9f");
  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(0, windowAdapter.getHeight()), 0, ImVec2(0.0, 1.0));
  ImGui::SetNextWindowSize(ImVec2(140, 80));
  ImGui::Begin("State");
  ImGui::Value("FPS", displayedFrames);
  ImGui::Value("ms/frame", displayedMS);
  ImGui::End();

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
}

void resizeCallback(GLFWwindow *win, int w, int h) {
  //std::cout << "\nresized to " << w << ", " << h << std::endl;
  glViewport(0, 0, w, h);
  screenSize.x = (GLfloat) w;
  screenSize.y = (GLfloat) h;
  screenRatio = screenSize.x / screenSize.y;
  projectionMatrix = glm::perspective(90.0f, screenRatio, NEAR_PLANE, FAR_PLANE);
  inverseVP = glm::inverse(viewMatrix) * glm::inverse(projectionMatrix);
}

void processInput(GLFWwindow *window) {

  // Close
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Reload shader
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    shader = utils::loadShaders("../shaders/mandel_raymarch.vert", "../shaders/mandel_raymarch.frag");

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
