#ifndef MANDELBULB_CAMERA_H
#define MANDELBULB_CAMERA_H

#include <iostream>
#include "types.hh"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Constants
#define PI     3.14159265358979323846f
#define TWO_PI 6.28318530718f

class Camera {
  const float SPHER_COORDINATES_STEP = 0.005f;
  const float SPHER_COORDINATES_STEP_HALF = 0.0025f;
  const float SPHER_COORDINATES_STEP_DOUBLE = 0.01f;

  // Coordinates for viewer
  float defaultR = 1.3f, defaultTheta = 0.0f, defaultPhi = 0.0f;
  float r = defaultR;
  float theta = defaultTheta;
  float phi = defaultPhi;

 public:
  float x = 0.0f, y = 0.0f, z = 0.0f;
  float freeModeTurnStep = 0.0001f;
  bool freeControlsActive = false;
  bool previousFreeControlsActive = false;

  vec3 eye;
  vec3 center;
  vec3 eyeTarget;
  vec3 up;
  mat4 viewMatrix;
  mat4 projectionMatrix;

  Camera(unsigned int w, unsigned int h, float near, float far);
  ~Camera() = default;

  /**
   * Print all coordinates
   */
  void printCoordinates();

  /**
   * Recalculate view matrix with viewer
   */
  void updateViewMatrix();

  // Key press handlers
  void handleKeyPressW();
  void handleKeyPressA();
  void handleKeyPressS();
  void handleKeyPressD();
  void handleKeyPressZ();
  void handleKeyPressX();

  /**
   * Reset coordinates to initial values
   */
  void resetCoords();

  /**
   * Calculate and set cartesian coords from spherical
   */
  void sphericalToCartesian();

};

#endif //MANDELBULB_CAMERA_H
