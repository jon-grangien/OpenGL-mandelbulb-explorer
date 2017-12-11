#include "Camera.hh"

Camera::Camera(unsigned int w, unsigned int h, float near, float far) {
  eye = vec3(0.0f, 0.0f, 1.0f);
  center = vec3(0.0f, 0.0f, 0.0f);
  eyeTarget = center;
  up = vec3(0.0f, 1.0f, 0.0f);
  viewMatrix = glm::lookAt(eye, eyeTarget, up);
  projectionMatrix = glm::perspective(90.0f, (GLfloat) w / (GLfloat) h, near, far);
};

void Camera::printCoordinates() {
  printf("\nr: %.1f, theta: %.1f, phi: %.1f and x: %.1f, y: %.1f, z: %.1f", r, theta, phi, x, y, z);
}

void Camera::updateViewMatrix() {
  viewMatrix = glm::lookAt(eye, eyeTarget, up);
}

void Camera::handleKeyPressW() {
  theta -= SPHER_COORDINATES_STEP;
  theta = std::max(0.0f, theta);
  theta = std::min(PI, theta);

  if (freeControlsActive)
    eyeTarget.y += freeModeTurnStep;
}

void Camera::handleKeyPressA() {
  phi -= SPHER_COORDINATES_STEP_DOUBLE;
  phi = std::max(0.0f, phi);
  phi = std::min(TWO_PI, phi);

  if (freeControlsActive)
    eyeTarget.x -= freeModeTurnStep;
}

void Camera::handleKeyPressS() {
  theta += SPHER_COORDINATES_STEP;
  theta = std::max(0.0f, theta);
  theta = std::min(PI, theta);

  if (freeControlsActive)
    eyeTarget.y -= freeModeTurnStep;
}

void Camera::handleKeyPressD() {
  phi += SPHER_COORDINATES_STEP_DOUBLE;
  phi = std::max(0.0f, phi);
  phi = std::min(TWO_PI, phi);

  if (freeControlsActive)
    eyeTarget.x += freeModeTurnStep;
}

void Camera::handleKeyPressZ() {
  r -= SPHER_COORDINATES_STEP_HALF;

  if (freeControlsActive)
    eye += freeModeTurnStep * glm::normalize(eyeTarget + eye);
}

void Camera::handleKeyPressX() {
  r += SPHER_COORDINATES_STEP_HALF;

  if (freeControlsActive)
    eye -= freeModeTurnStep * glm::normalize(eyeTarget + eye);
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
}
