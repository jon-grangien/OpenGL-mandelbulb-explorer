#ifndef MANDELBULB_TYPES_H
#define MANDELBULB_TYPES_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

typedef glm::mat4 mat4;
typedef glm::mat4x2 mat4x2;

typedef glm::vec3 vec3;
typedef glm::vec2 vec2;

typedef void (* FrameBufferSizeCallback)(GLFWwindow *win, int w, int h);
typedef void (* ProcessInputFunc)(GLFWwindow *win);
typedef void (* DisplayFunc)();

#endif //MANDELBULB_TYPES_H
