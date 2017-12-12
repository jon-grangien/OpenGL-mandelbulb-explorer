#include "Camera.hh"

Camera::Camera(unsigned int w, unsigned int h, float near, float far) {
  eye = vec3(0.0f, 0.0f, 1.0f);
  center = vec3(0.0f, 0.0f, 0.0f);
  up = vec3(0.0f, 1.0f, 0.0f);
  viewMatrix = glm::lookAt(eye, center, up);
  projectionMatrix = glm::perspective(90.0f, (GLfloat) w / (GLfloat) h, near, far);
};

void Camera::printCoordinates() {
  printf("\nr: %.3f, theta: %.3f, phi: %.3f and x: %.3f, y: %.3f, z: %.3f", r, theta, phi, x, y, z);
}

void Camera::updateCenteredViewMatrix() {
  viewMatrix = glm::lookAt(eye, center, up);
}

vec3 Camera::getViewMatrixBackward() {
  return vec3(viewMatrix[1][3], viewMatrix[2][3], viewMatrix[3][3]);
}

vec3 Camera::getViewMatrixForward() {
  return -getViewMatrixBackward();
}

void Camera::rotateViewMatrixHorizontally(float a) {
  viewMatrix = glm::rotate(a, vec3(0.0, 1.0, 0.0)) * viewMatrix;
}

void Camera::rotateViewMatrixVertically(float a) {
  viewMatrix = glm::rotate(a, vec3(1.0, 0.0, 0.0)) * viewMatrix;
}

void Camera::translateViewMatrix(float v) {
  vec3 tvec = glm::normalize(getViewMatrixForward());
  tvec = vec3(tvec.x * v, tvec.y * v, tvec.z * v);
  viewMatrix = glm::translate(tvec) * viewMatrix;
}

void Camera::handleKeyPressW() {
  theta -= coordTurnStep;
  theta = std::max(0.0f, theta);
  theta = std::min(PI, theta);

  if (freeControlsActive)
    rotateViewMatrixVertically(-coordTurnStep);
}

void Camera::handleKeyPressA() {
  phi -= coordTurnStep * 2.0f;  // Could be optimized by keeping track of doubled step. Requires ImGui callbacks
  phi = std::max(0.0f, phi);
  phi = std::min(TWO_PI, phi);

  if (freeControlsActive)
    rotateViewMatrixHorizontally(-coordTurnStep);
}

void Camera::handleKeyPressS() {
  theta += coordTurnStep;
  theta = std::max(0.0f, theta);
  theta = std::min(PI, theta);

  if (freeControlsActive)
    rotateViewMatrixVertically(coordTurnStep);
}

void Camera::handleKeyPressD() {
  phi += coordTurnStep * 2.0f;
  phi = std::max(0.0f, phi);
  phi = std::min(TWO_PI, phi);

  if (freeControlsActive)
    rotateViewMatrixHorizontally(coordTurnStep);
}

void Camera::handleKeyPressZ() {
  r -= coordZoomStep;

  if (freeControlsActive)
    translateViewMatrix(-coordZoomStep);
}

void Camera::handleKeyPressX() {
  r += coordZoomStep;

  if (freeControlsActive)
    translateViewMatrix(coordZoomStep);
}

void Camera::resetCoords() {
  r = defaultR;
  theta = defaultTheta;
  phi = defaultPhi;
}

void Camera::sphericalToCartesian() {
  x = r * sinf(theta) * cosf(phi);
  y = r * sinf(theta) * sinf(phi);
  z = r * cosf(theta);
  printCoordinates();
}

