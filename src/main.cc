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
void setGuiStyle();

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
float bailLimit = 1.8;
float power = 8.0;
float noiseFactor = 1.0;
vec3 bgColor = vec3(0.69, 0.55, 0.76);
vec3 mandelColorA = vec3(0.9, 0.6, 1.0);
vec3 mandelColorB = vec3(1.0);
vec3 glowColor = vec3(0.75, 0.9, 1.0);
bool showBgGradient = true;
bool phongShading = true;

// App state
auto windowAdapter = Window(INITIAL_WIDTH, INITIAL_HEIGHT);
bool logPerformance = false;
bool logCoordinates = false;
bool weakSettings = false;
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
  int OK = utils::handleArgs(argc, argv, logCoordinates, weakSettings);
  if (OK < 0) return -1;

  if (weakSettings) {
    maxRaySteps = 200.0;
    mandelIters = 100.0;
    minDistanceFactor = 3.0;
    power = 6.0;
  }

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

  setGuiStyle();

  // Graphics settings
  ImGui::SetNextWindowSize(ImVec2(350, 180));
  ImGui::Begin("Settings");
  ImGui::Text("Graphics values");
  ImGui::SliderFloat("Max ray steps", &maxRaySteps, 5.0f, 4000.0f);
  ImGui::SliderFloat("Mandel iters", &mandelIters, 1.0f, 3000.0f);
  ImGui::SliderInt("Min dist factor", &minDistanceFactor, -5, 3);

  // Adjust the min distance by a decimal
  if (minDistanceFactor < 0) {
    minDistance = baseMinDistance / ((float)(pow(10.0, abs(minDistanceFactor))));
  } else if (minDistanceFactor > 0) {
    minDistance = baseMinDistance * ((float)(pow(10.0, minDistanceFactor)));
  } else {
    minDistance = baseMinDistance;
  }

  ImGui::SliderFloat("Bailout", &bailLimit, 1.0f, 1.81f);
  ImGui::SliderFloat("Power", &power, 1.0f, 32.0f);
  ImGui::Checkbox("Light source", &phongShading);
  ImGui::Separator();
  ImGui::Value("Min dist", minDistance, "%.9f");
  ImGui::End();

  // Color settings
  ImGui::Begin("Colors");
  //ImGui::ShowStyleEditor();
  ImGui::ColorEdit3("Bg color", (float*)&bgColor);
  ImGui::Checkbox("Bg gradient", &showBgGradient);
  ImGui::Separator();
  ImGui::ColorEdit3("Inner", (float*)&mandelColorA);
  ImGui::ColorEdit3("Outer", (float*)&mandelColorB);
  ImGui::ColorEdit3("Glow", (float*)&glowColor);
  ImGui::SliderFloat("Noise", &noiseFactor, 0.0f, 1.0f);
  ImGui::End();

  // Stats
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
  glUniform1i(glGetUniformLocation(shader, "u_phongShading"), phongShading);
  glUniform3fv(glGetUniformLocation(shader, "u_bgColor"), 1, glm::value_ptr(bgColor));
  glUniform3fv(glGetUniformLocation(shader, "u_mandelColorA"), 1, glm::value_ptr(mandelColorA));
  glUniform3fv(glGetUniformLocation(shader, "u_mandelColorB"), 1, glm::value_ptr(mandelColorB));
  glUniform3fv(glGetUniformLocation(shader, "u_glowColor"), 1, glm::value_ptr(glowColor));
  glUniform3fv(glGetUniformLocation(shader, "u_eyePos"), 1, glm::value_ptr(eye));
  glUniform1i(glGetUniformLocation(shader, "u_showBgGradient"), showBgGradient);
  glUniform1fv(glGetUniformLocation(shader, "u_noiseFactor"), 1, &noiseFactor);
}

void resizeCallback(GLFWwindow *win, int w, int h) {
  //std::cout << "\nresized to " << w << ", " << h << std::endl;
  glViewport(0, 0, w, h);
  screenSize.x = (GLfloat) w;
  screenSize.y = (GLfloat) h;
  screenRatio = screenSize.x / screenSize.y;
  windowAdapter.setResolution((unsigned int) w, (unsigned int) h);
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

void setGuiStyle() {
  ImVec4* colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
  colors[ImGuiCol_Border]                 = ImVec4(0.60f, 0.23f, 0.23f, 0.00f);
  colors[ImGuiCol_TitleBg]                = ImVec4(mandelColorA.r, mandelColorA.g, mandelColorA.b, 0.40f);
  colors[ImGuiCol_TitleBgActive]          = ImVec4(mandelColorA.r, mandelColorA.g, mandelColorA.b, 0.65f);
  colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
  colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
  colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
  colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  colors[ImGuiCol_SeparatorActive]        = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
  colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_CloseButton]            = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
  colors[ImGuiCol_CloseButtonHovered]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  colors[ImGuiCol_CloseButtonActive]      = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}
