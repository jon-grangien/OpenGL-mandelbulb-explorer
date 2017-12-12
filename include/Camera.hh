#ifndef MANDELBULB_CAMERA_H
#define MANDELBULB_CAMERA_H

#include <iostream>
#include "types.hh"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

// Constants
#define PI     3.14159265358979323846f
#define TWO_PI 6.28318530718f

class Camera {

  // Coordinates for viewer
  float defaultR = 1.3f, defaultTheta = 0.0f, defaultPhi = 0.0f;
  float r = defaultR;
  float theta = defaultTheta;
  float phi = defaultPhi;

 public:
  float x = 0.0f, y = 0.0f, z = 0.0f;
  float coordTurnStep = 0.01f;
  float coordZoomStep = 0.008f;
  bool freeControlsActive = false;
  bool constantZoom = false;

  vec3 eye;
  vec3 center;
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
  void updateCenteredViewMatrix();
  void rotateViewMatrixHorizontally(float a);
  void rotateViewMatrixVertically(float a);
  void translateViewMatrix(float v);

  vec3 getViewMatrixBackward();
  vec3 getViewMatrixForward();

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
