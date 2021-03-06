#include <iostream>
#include <GL/glew.h>
#include "utils.hh"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Window.hh"
#include "types.hh"
#include "Camera.hh"
#include <imgui.h>
#include <GLFW/glfw3.h>

// Declarations
void resizeCallback(GLFWwindow *win, int w, int h);
void processInput(GLFWwindow *window);
void display();
void renderGui();
void setGuiStyle();

float STEP_SIZE = 0.001f;
unsigned int INITIAL_WIDTH = 800;
unsigned int INITIAL_HEIGHT = 640;

// Correct setup with artifacts
//float NEAR_PLANE = -0.1f;
//float FAR_PLANE = -100.0f;

// Incorrect setup with less artifacts
float NEAR_PLANE = 0.1f;
float FAR_PLANE = 100.0f;

float FOV = 50.0f;

struct FractalUniforms {

  // Renderer
  float maxRaySteps = 1000.0;
  float baseMinDistance = 0.00001;
  float minDistance = baseMinDistance;
  int minDistanceFactor = 0;
  int fractalIters = 100;
  float bailLimit = 5.0;

  // Mandelbulb
  float power = 8.0;
  int derivativeBias = 1;
  bool julia = false;
  vec3 juliaC = vec3(0.86, 0.23, -0.5);

  // Box
  int boxFoldFactor = 1;
  float boxFoldingLimit = 1.0;

  // Sphere
  int sphereFoldFactor = 1;
  float sphereMinRadius = 0.01;
  float sphereFixedRadius = 2.0;
  bool sphereMinTimeVariance = false;

  // Mandelbox
  int mandelBoxFactor = 1;
  float mandelBoxScale = 1.2;

  // Tetra
  int tetraFactor = 1;
  float tetraScale = 1.0;

  float fudgeFactor = 1.0;
  float noiseFactor = 0.5;
  vec3 bgColor = vec3(0.8, 0.85, 1.0);
  vec3 glowColor = vec3(0.75, 0.9, 1.0);
  float glowFactor = 1.0;
  bool showBgGradient = true;

  vec4 orbitStrength = vec4(-1.0, -1.8, -1.4, 1.3);
  vec3 otColor0 = vec3(0.3, 0.5, 0.2);
  vec3 otColor1 = vec3(0.6, 0.2, 0.5);
  vec3 otColor2 = vec3(0.25, 0.7, 0.9);
  vec3 otColor3 = vec3(0.2, 0.45, 0.25);
  vec3 otColorBase = vec3(0.3, 0.6, 0.3);
  float otBaseStrength = 0.5;
  float otDist0to1 = 0.3;
  float otDist1to2 = 1.0;
  float otDist2to3 = 0.4;
  float otDist3to0 = 0.2;
  float otCycleIntensity = 5.0;
  float otPaletteOffset = 0.0;

  int shadowRayMinStepsTaken = 5;
  vec3 lightPos = vec3(3.0, 3.0, 10.0);
  float shadowBrightness = 0.2f;
  bool lightSource = true;
  float phongShadingMixFactor = 1.0;
  float ambientIntensity = 1.0;
  float diffuseIntensity = 1.0;
  float specularIntensity = 1.0;
  float shininess = 32.0;
  bool gammaCorrection = false;
};

// App state
struct AppState {
  bool mandelbulbOn = true;
  bool boxFoldingOn = false;
  bool sphereFoldingOn = false;
  bool mandelBoxOn = false;
  bool recursiveTetraOn = false;

  bool lowOtCycleIntensity = false;

  bool logCoordinates = false;
  bool weakSettings = false;
  int nbFrames = 0;
  int displayedFrames = 0;
  float displayedMS = 0;
  double lastTime = (float) glfwGetTime();

  bool showGui = true;
  float timeSinceLastGuiToggle = 0.0f;
};

bool shouldUpdateCoordinates = true; // True initially to first set spherical to cartesian

GLuint shader;
GLuint vbo, vao;
GLFWwindow *window;

const GLfloat quadArray[4][2] = {
    {-1.0f, -1.0f},
    {1.0f, -1.0f},
    {-1.0f, 1.0f},
    {1.0f, 1.0f}
};
mat4x2 quad = glm::make_mat4x2(&quadArray[0][0]);
mat4 inverseVP;

GLfloat currentTime = 0.0;
auto screenSize = vec2(INITIAL_WIDTH, INITIAL_HEIGHT);
GLfloat screenRatio = screenSize.x / screenSize.y;

auto windowAdapter = Window(INITIAL_WIDTH, INITIAL_HEIGHT);
auto cam = Camera(INITIAL_WIDTH, INITIAL_HEIGHT, NEAR_PLANE, FAR_PLANE);
FractalUniforms u;
AppState state;

int main(int argc, char *argv[]) {

  // Handle args
  int OK = utils::handleArgs(argc, argv, state.logCoordinates, state.weakSettings);
  if (OK < 0) return -1;

  if (state.weakSettings) {
    u.maxRaySteps = 200.0;
    u.fractalIters = 20;
    u.minDistanceFactor = 3;
    u.power = 6.0;
  }

  utils::printInstructions();

  auto glfwOk = windowAdapter.init(resizeCallback, processInput, display);
  auto err = glewInit();

  if (!glfwOk)
    return EXIT_FAILURE;
  if (err != GLEW_OK)
    std::cout << "Error: GLEW failed to init\n";

  inverseVP = glm::inverse(cam.viewMatrix) * glm::inverse(cam.projectionMatrix);
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

  if (state.logCoordinates) {
    cam.printCoordinates();
    fflush(stdout);
  }

  // Perfomance calculations
  state.nbFrames++;
  if (currentTime - state.lastTime >= 1.0) {
    state.displayedMS = (float) 1000.0 / float(state.nbFrames);
    state.displayedFrames = state.nbFrames;
    state.nbFrames = 0;
    state.lastTime += 1.0;
  }

  if (shouldUpdateCoordinates) {

    // Calculate centered view matrix every frame for locked spherical coord controls
    if (!cam.freeControlsActive) {
      cam.sphericalToCartesian();
      cam.eye = vec3(cam.y, cam.x, cam.z);
      cam.updateCenteredViewMatrix();
    }

    inverseVP = glm::inverse(cam.projectionMatrix * cam.viewMatrix);

    shouldUpdateCoordinates = false;
  }

  if (state.showGui) {
    setGuiStyle();
    renderGui();
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

  // Renderer
  glUniform1fv(glGetUniformLocation(shader, "u_maxRaySteps"), 1, &u.maxRaySteps);
  glUniform1fv(glGetUniformLocation(shader, "u_minDistance"), 1, &u.minDistance);
  glUniform1i(glGetUniformLocation(shader, "u_fractalIters"), u.fractalIters);
  glUniform1fv(glGetUniformLocation(shader, "u_bailLimit"), 1, &u.bailLimit);

  // Fractals
  glUniform1i(glGetUniformLocation(shader, "u_mandelbulbOn"), state.mandelbulbOn);
  glUniform1i(glGetUniformLocation(shader, "u_derivativeBias"), u.derivativeBias);
  glUniform1fv(glGetUniformLocation(shader, "u_power"), 1, &u.power);
  glUniform1i(glGetUniformLocation(shader, "u_julia"), u.julia);
  glUniform3fv(glGetUniformLocation(shader, "u_juliaC"), 1, glm::value_ptr(u.juliaC));

  glUniform1i(glGetUniformLocation(shader, "u_boxFoldFactor"), state.boxFoldingOn ? u.boxFoldFactor : 0);
  glUniform1fv(glGetUniformLocation(shader, "u_boxFoldingLimit"), 1, &u.boxFoldingLimit);

  glUniform1i(glGetUniformLocation(shader, "u_sphereFoldFactor"), state.sphereFoldingOn ? u.sphereFoldFactor : 0);
  glUniform1fv(glGetUniformLocation(shader, "u_sphereMinRadius"), 1, &u.sphereMinRadius);
  glUniform1fv(glGetUniformLocation(shader, "u_sphereFixedRadius"), 1, &u.sphereFixedRadius);
  glUniform1i(glGetUniformLocation(shader, "u_sphereMinTimeVariance"), u.sphereMinTimeVariance);

  glUniform1i(glGetUniformLocation(shader, "u_mandelBoxOn"), state.mandelBoxOn);
  glUniform1fv(glGetUniformLocation(shader, "u_mandelBoxScale"), 1, &u.mandelBoxScale);

  glUniform1i(glGetUniformLocation(shader, "u_tetraFactor"), state.recursiveTetraOn ? u.tetraFactor : 0);
  glUniform1fv(glGetUniformLocation(shader, "u_tetraScale"), 1, &u.tetraScale);

  // Graphics
  glUniform4fv(glGetUniformLocation(shader, "u_orbitStrength"), 1, glm::value_ptr(u.orbitStrength));
  glUniform3fv(glGetUniformLocation(shader, "u_color0"), 1, glm::value_ptr(u.otColor0));
  glUniform3fv(glGetUniformLocation(shader, "u_color1"), 1, glm::value_ptr(u.otColor1));
  glUniform3fv(glGetUniformLocation(shader, "u_color2"), 1, glm::value_ptr(u.otColor2));
  glUniform3fv(glGetUniformLocation(shader, "u_color3"), 1, glm::value_ptr(u.otColor3));
  glUniform3fv(glGetUniformLocation(shader, "u_colorBase"), 1, glm::value_ptr(u.otColorBase));
  glUniform1fv(glGetUniformLocation(shader, "u_baseColorStrength"), 1, &u.otBaseStrength);
  glUniform1fv(glGetUniformLocation(shader, "u_otDist0to1"), 1, &u.otDist0to1);
  glUniform1fv(glGetUniformLocation(shader, "u_otDist1to2"), 1, &u.otDist1to2);
  glUniform1fv(glGetUniformLocation(shader, "u_otDist2to3"), 1, &u.otDist2to3);
  glUniform1fv(glGetUniformLocation(shader, "u_otDist3to0"), 1, &u.otDist3to0);
  glUniform1fv(glGetUniformLocation(shader, "u_otCycleIntensity"), 1, &u.otCycleIntensity);
  glUniform1fv(glGetUniformLocation(shader, "u_otPaletteOffset"), 1, &u.otPaletteOffset);

  glUniform1i(glGetUniformLocation(shader, "u_shadowRayMinStepsTaken"), u.shadowRayMinStepsTaken);
  glUniform1i(glGetUniformLocation(shader, "u_lightSource"), u.lightSource);
  glUniform1fv(glGetUniformLocation(shader, "u_phongShadingMixFactor"), 1, &u.phongShadingMixFactor);
  glUniform3fv(glGetUniformLocation(shader, "u_lightPos"), 1, glm::value_ptr(u.lightPos));
  glUniform1fv(glGetUniformLocation(shader, "u_shadowBrightness"), 1, &u.shadowBrightness);
  glUniform3fv(glGetUniformLocation(shader, "u_bgColor"), 1, glm::value_ptr(u.bgColor));
  glUniform3fv(glGetUniformLocation(shader, "u_glowColor"), 1, glm::value_ptr(u.glowColor));
  glUniform1fv(glGetUniformLocation(shader, "u_glowFactor"), 1, &u.glowFactor);
  glUniform3fv(glGetUniformLocation(shader, "u_eyePos"), 1, glm::value_ptr(cam.eye));
  glUniform1i(glGetUniformLocation(shader, "u_showBgGradient"), u.showBgGradient);
  glUniform1fv(glGetUniformLocation(shader, "u_noiseFactor"), 1, &u.noiseFactor);
  glUniform1fv(glGetUniformLocation(shader, "u_fudgeFactor"), 1, &u.fudgeFactor);
  glUniform1fv(glGetUniformLocation(shader, "u_ambientIntensity"), 1, &u.ambientIntensity);
  glUniform1fv(glGetUniformLocation(shader, "u_diffuseIntensity"), 1, &u.diffuseIntensity);
  glUniform1fv(glGetUniformLocation(shader, "u_specularIntensity"), 1, &u.specularIntensity);
  glUniform1fv(glGetUniformLocation(shader, "u_shininess"), 1, &u.shininess);
  glUniform1i(glGetUniformLocation(shader, "u_gammaCorrection"), u.gammaCorrection);
}

void renderGui() {

  // Graphics settings
  //ImGui::SetNextWindowSize(ImVec2(350, 280));
  ImGui::Begin("Fractal values and graphics");
  ImGui::Text("Renderer");
  ImGui::SliderFloat("Max ray steps", &u.maxRaySteps, 5.0f, 4000.0f);
  ImGui::SliderInt("Mandel iters", &u.fractalIters, 1, 80);
  ImGui::SliderInt("Min dist factor", &u.minDistanceFactor, -5, 3);

  // Adjust the min distance by a decimal
  if (u.minDistanceFactor < 0) {
    u.minDistance = u.baseMinDistance / ((float)(pow(10.0, abs(u.minDistanceFactor))));
  } else if (u.minDistanceFactor > 0) {
    u.minDistance = u.baseMinDistance * ((float)(pow(10.0, u.minDistanceFactor)));
  } else {
    u.minDistance = u.baseMinDistance;
  }

  ImGui::Value("Min dist", u.minDistance, "%.9f");
  ImGui::SliderFloat("Bailout", &u.bailLimit, 1.0f, 10.0f);
  ImGui::SliderFloat("\"Fudge\"", &u.fudgeFactor, 0.0f, 1.0f);

  ImGui::Separator();
  ImGui::Text("Fractal values");
  ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 0.5), "Combine formulas into the fractal");

  ImGui::Text("Mandelbulb");
  ImGui::Checkbox("Mix Mandelbulb", &state.mandelbulbOn);
  if (state.mandelbulbOn) {
    ImGui::SliderFloat("Power", &u.power, 1.0f, 32.0f);
    ImGui::SliderInt("Derivative bias", &u.derivativeBias, 0, 10);
    ImGui::Checkbox("Julia", &u.julia);
    if (u.julia) {
      ImGui::SliderFloat("JuliaC X", &u.juliaC.x, -2.0f, 2.0f);
      ImGui::SliderFloat("JuliaC Y", &u.juliaC.y, -2.0f, 2.0f);
      ImGui::SliderFloat("JuliaC Z", &u.juliaC.z, -2.0f, 2.0f);
    }
  }

  ImGui::Text("Box folding");
  ImGui::Checkbox("Mix box folding", &state.boxFoldingOn);
  if (state.boxFoldingOn) {
    ImGui::SliderInt("Box fold mult", &u.boxFoldFactor, 0, 5);
    ImGui::SliderFloat("Fold limit", &u.boxFoldingLimit, 0.0f, 10.0f);
  }

  ImGui::Text("Sphere folding");
  ImGui::Checkbox("Mix sphere folding", &state.sphereFoldingOn);
  if (state.sphereFoldingOn) {
    ImGui::SliderInt("Sphere fold mult", &u.sphereFoldFactor, 0, 5);
    ImGui::SliderFloat("Min radius", &u.sphereMinRadius, 0.0000001f, 1.0f, "%.8f");
    ImGui::SliderFloat("Fixed radius", &u.sphereFixedRadius, 0.0f, 4.0f, "%.2f");
    ImGui::Checkbox("Beat", &u.sphereMinTimeVariance);
  }

  ImGui::Text("Mandelbox");
  ImGui::Checkbox("Mix Mandelbox", &state.mandelBoxOn);
  if (state.mandelBoxOn) {
    ImGui::SliderFloat("Scale", &u.mandelBoxScale, 0.01f, 5.0f, "%.3f");
    ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 0.5), "Note: Below is same as above if active");
    ImGui::SliderFloat("Sphere min r", &u.sphereMinRadius, 0.0000001f, 1.0f, "%.8f");
    ImGui::SliderFloat("Sphere fixed r", &u.sphereFixedRadius, 0.0f, 4.0f, "%.2f");
    ImGui::SliderFloat("Box fold limit", &u.boxFoldingLimit, 0.0f, 10.0f);
  }

  ImGui::Text("Recursive Tetra");
  ImGui::Checkbox("Mix rec tetra", &state.recursiveTetraOn);
  if (state.recursiveTetraOn) {
    ImGui::SliderInt("Tetra mult", &u.tetraFactor, 0, 5);
    ImGui::SliderFloat("Tetra scale", &u.tetraScale, 0.1f, 2.0f, "%.2f");
  }

  ImGui::Separator();
  ImGui::Text("Graphics");
  ImGui::Checkbox("Light source", &u.lightSource);
  ImGui::Separator();
  ImGui::Text("Controls");
  ImGui::Checkbox("FREE MODE", &cam.freeControlsActive);
  ImGui::Checkbox("Auto trip", &cam.constantZoom);
  ImGui::SliderFloat("Turn step", &cam.coordTurnStep, 0.001, 0.01, "%.4f");
  ImGui::SliderFloat("Zoom step", &cam.coordZoomStep, 0.0000001, 0.01, "%.7f");
  ImGui::End();

  // Color settings
  ImGui::Begin("Colors");
  //ImGui::ShowStyleEditor();
  ImGui::ColorEdit3("Bg color", (float*)&u.bgColor);
  ImGui::Checkbox("Bg gradient", &u.showBgGradient);
  ImGui::Separator();
  ImGui::Text("Orbit trap palette");
  ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 0.5), "Place colors w/ palette offset");
  ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 0.5), "Reduce cycle intesity if noisy");
  ImGui::ColorEdit3("Color 0", (float*)&u.otColor0);
  ImGui::ColorEdit3("Color 1", (float*)&u.otColor1);
  ImGui::ColorEdit3("Color 2", (float*)&u.otColor2);
  ImGui::ColorEdit3("Color 3", (float*)&u.otColor3);
  ImGui::SliderFloat("Dist 0 > 1", &u.otDist0to1, 0.0f, 3.0f);
  ImGui::SliderFloat("Dist 1 > 2", &u.otDist1to2, 0.0f, 3.0f);
  ImGui::SliderFloat("Dist 2 > 3", &u.otDist2to3, 0.0f, 3.0f);
  ImGui::SliderFloat("Dist 3 > 0", &u.otDist3to0, 0.0f, 3.0f);
  ImGui::ColorEdit3("Base Color", (float*)&u.otColorBase);
  ImGui::SliderFloat("Base color strength", &u.otBaseStrength, 0.0f, 1.0f);
  ImGui::Checkbox("Very low cycle intensity (e.g. for mandelbox)", &state.lowOtCycleIntensity);
  u.otCycleIntensity = (u.otCycleIntensity > 0.0101f && state.lowOtCycleIntensity) ? 0.01f : u.otCycleIntensity;
  ImGui::SliderFloat("Cycle intensity", &u.otCycleIntensity, 0.00f, state.lowOtCycleIntensity ? 0.01f : 6.0f, "%.5f");
  ImGui::SliderFloat("Palette offset", &u.otPaletteOffset, 0.0f, 100.0f);
  ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 0.5), "Centered values give more of a blend");
  ImGui::SliderFloat("Orbit strength X", &u.orbitStrength.x, -3.0f, 3.0f);
  ImGui::SliderFloat("Orbit strength Y", &u.orbitStrength.y, -3.0f, 3.0f);
  ImGui::SliderFloat("Orbit strength Z", &u.orbitStrength.z, -3.0f, 3.0f);
  ImGui::SliderFloat("Orbit strength R", &u.orbitStrength.w, -3.0f, 3.0f);
  ImGui::Separator();
  ImGui::ColorEdit3("Glow color", (float*)&u.glowColor);
  ImGui::SliderFloat("Glow strength", &u.glowFactor, 0.0f, 1.0f);
  ImGui::SliderFloat("Noise", &u.noiseFactor, 0.0f, 1.0f);

  if (u.lightSource) {
    ImGui::Separator();
    ImGui::Text("Light src position");
    ImGui::SliderFloat("Pos x", &u.lightPos.x, -20.0f, 20.0f);
    ImGui::SliderFloat("Pos y", &u.lightPos.y, -20.0f, 20.0f);
    ImGui::SliderFloat("Pos z", &u.lightPos.z, -20.0f, 20.0f);
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.0, 0.0, 0.0, 0.5), "More steps ignored may reduce noise");
    ImGui::SliderFloat("Shadow brighness", &u.shadowBrightness, 0.0f, 0.5f);
    ImGui::SliderInt("Steps ignored", &u.shadowRayMinStepsTaken, 0, 20);
    ImGui::Separator();
    ImGui::Text("Blinn-phong shading");
    ImGui::SliderFloat("Mix-in factor", &u.phongShadingMixFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Ambient", &u.ambientIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &u.diffuseIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Specular", &u.specularIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Shininess", &u.shininess, 0.0f, 64.0f);
    ImGui::Checkbox("Gamma correction 2.2", &u.gammaCorrection);
  }
  ImGui::End();

  // Debug
  //ImGui::Begin("Debug");
  //ImGui::SliderFloat("Near plane", &NEAR_PLANE, -1.0f, 2.0f);
  //ImGui::SliderFloat("Far plane", &FAR_PLANE, -2.0f, 2.0f);
  //ImGui::End();
  //NEAR_PLANE = std::abs(NEAR_PLANE - 0.001f) < 0.001f ? 0.001f : NEAR_PLANE;
  //FAR_PLANE = std::abs(FAR_PLANE - 0.001f) < 0.001f ? 0.001f : FAR_PLANE;
  //cam.projectionMatrix = glm::perspective(glm::radians(FOV), screenRatio, NEAR_PLANE, FAR_PLANE);
  //cam.printCoordinates();

  // Stats
  ImGui::SetNextWindowPos(ImVec2(0, windowAdapter.getHeight()), 0, ImVec2(0.0, 1.0));
  ImGui::SetNextWindowSize(ImVec2(140, 80));
  ImGui::Begin("State");
  ImGui::Value("FPS", state.displayedFrames);
  ImGui::Value("ms/frame", state.displayedMS);
  ImGui::End();
}

void resizeCallback(GLFWwindow *win, int w, int h) {
  //std::cout << "\nresized to " << w << ", " << h << std::endl;
  glViewport(0, 0, w, h);
  screenSize.x = (GLfloat) w;
  screenSize.y = (GLfloat) h;
  screenRatio = screenSize.x / screenSize.y;
  windowAdapter.setResolution((unsigned int) w, (unsigned int) h);
  cam.projectionMatrix = glm::perspective(glm::radians(FOV), screenRatio, NEAR_PLANE, FAR_PLANE);
  inverseVP = glm::inverse(cam.projectionMatrix * cam.viewMatrix);
}

// Process input keys concurrently of each other
// This is harder to do with a normal key callback function
void processInput(GLFWwindow *window) {

  // Close
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {

    // Disable toggle within small time window to avoid flickering in high fps
    if (std::abs(currentTime - state.timeSinceLastGuiToggle) > 0.15) {
      state.showGui = !state.showGui;
      state.timeSinceLastGuiToggle = currentTime;
    }
  }

  // Reload shader
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    shader = utils::loadShaders("../shaders/mandel_raymarch.vert", "../shaders/mandel_raymarch.frag");

  // Movement
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    cam.handleKeyPressW();
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    cam.handleKeyPressA();
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    cam.handleKeyPressS();
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    cam.handleKeyPressD();
  }

  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS || cam.constantZoom) {
    shouldUpdateCoordinates = true;
    cam.handleKeyPressZ();
  }

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    cam.handleKeyPressX();
  }

  // Reset camera
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    shouldUpdateCoordinates = true;
    cam.resetCoords();
  }
}

void setGuiStyle() {
  ImVec4* colors = ImGui::GetStyle().Colors;
  vec3 blendedColor = glm::normalize(u.otColor0 + u.otColor1 + u.otColor2 + u.otColor3 + u.otBaseStrength * u.otColorBase);


  colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
  colors[ImGuiCol_Border]                 = ImVec4(0.60f, 0.23f, 0.23f, 0.00f);
  colors[ImGuiCol_TitleBg]                = ImVec4(blendedColor.r, blendedColor.g, blendedColor.b, 0.40f);
  colors[ImGuiCol_TitleBgActive]          = ImVec4(blendedColor.r, blendedColor.g, blendedColor.b, 0.65f);
  colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(blendedColor.r, blendedColor.g, blendedColor.b, 0.25f);
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
