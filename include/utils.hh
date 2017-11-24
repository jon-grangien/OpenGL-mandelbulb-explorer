#include <cstdio>
#include <cstring>
#include <string>
#include "types.hh"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifndef MANDELBULB_UTILS_H
#define MANDELBULB_UTILS_H

namespace utils {

unsigned char *readFile(char *fileName) {
  FILE *file = fopen(fileName, "r");
  if (file == nullptr) {
    std::cout << "Error: Cannot open file. " << fileName << std::endl;
    return nullptr;
  }

  // File length
  long numBytes;
  long savedPos = ftell(file); // Store pos
  fseek(file, 0, SEEK_END);    // go to end
  numBytes = ftell(file);      // Index of last byte in file
  fseek(file, savedPos, SEEK_SET); // Get back to pos
  long bytesInFile = numBytes;

  auto *buffer = (unsigned char *) malloc(bytesInFile + 1);
  auto bytesRead = (int) fread(buffer, 1, bytesInFile, file);
  buffer[bytesRead] = 0; // Terminate the string with 0
  fclose(file);

  return buffer;

}

void printShaderInfoLog(GLuint obj, const char *fn) {
  GLint infologLength = 0;
  GLint charsWritten = 0;
  char *infoLog;

  glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 2) {
    fprintf(stderr, "[From %s:]\n", fn);
    infoLog = (char *) malloc(infologLength);
    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
    fprintf(stderr, "%s\n", infoLog);
    free(infoLog);
  }
}

void printProgramInfoLog(GLuint obj, const char *vfn, const char *ffn,
                         const char *gfn, const char *tcfn, const char *tefn) {
  GLint infologLength = 0;
  GLint charsWritten = 0;
  char *infoLog;

  glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 2) {
    if (gfn == nullptr)
      fprintf(stderr, "[From %s+%s:]\n", vfn, ffn);
    else if (tcfn == nullptr || tefn == nullptr)
      fprintf(stderr, "[From %s+%s+%s:]\n", vfn, ffn, gfn);
    else
      fprintf(stderr, "[From %s+%s+%s+%s+%s:]\n", vfn, ffn, gfn, tcfn, tefn);
    infoLog = (char *) malloc(infologLength);
    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
    fprintf(stderr, "%s\n", infoLog);
    free(infoLog);
  }
}

GLuint compileShaders(const unsigned char *vertAssembly,
                      const unsigned char *fragAssembly,
                      const char *vertName,
                      const char *fragName) {
  GLuint vertShader, fragShader, pID;
  const char *vertShaderStrings[1], *fragShaderStrings[1];
  int vertSuccess, fragSuccess, shadersLinked;
  char infoLog[4096];

  vertShader = glCreateShader(GL_VERTEX_SHADER);
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  vertShaderStrings[0] = (char *) vertAssembly;
  fragShaderStrings[0] = (char *) fragAssembly;

  glShaderSource(vertShader, 1, vertShaderStrings, nullptr);
  glShaderSource(fragShader, 1, fragShaderStrings, nullptr);
  glCompileShader(vertShader);
  glCompileShader(fragShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &vertSuccess);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragSuccess);

  if (!vertSuccess) {
    glGetShaderInfoLog(vertShader, sizeof(infoLog), nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  if (!fragSuccess) {
    glGetShaderInfoLog(fragShader, sizeof(infoLog), nullptr, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  pID = glCreateProgram();
  glAttachShader(pID, vertShader);
  glAttachShader(pID, fragShader);
  glBindAttribLocation(pID, 0, "in_Position");
  glLinkProgram(pID);
  glGetProgramiv(pID, GL_LINK_STATUS, &shadersLinked);

  if (shadersLinked == GL_FALSE) {
    glGetProgramInfoLog(pID, sizeof(infoLog), nullptr, infoLog);
    std::cout << "Program object linking error'n" << infoLog << std::endl;
  }

  //glUseProgram(p);
  glDetachShader(pID, vertShader);
  glDetachShader(pID, fragShader);
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  printShaderInfoLog(vertShader, vertName);
  printShaderInfoLog(fragShader, fragName);
  return pID;
}

GLuint loadShaders(const char *vertFileName, const char *fragFileName) {
  unsigned char *vs, *fs;
  GLuint program = 0;

  vs = readFile((char *) vertFileName);
  fs = readFile((char *) fragFileName);

  if (vs == nullptr) {
    perror("Failed to read vertex from disk.\n");
  }
  if (fs == nullptr)
    fprintf(stderr, "Failed to read %s from disk.\n", fragFileName);

  if ((vs != nullptr) && (fs != nullptr)) {
    program = compileShaders(vs, fs, vertFileName, fragFileName);
  }

  if (vs != nullptr) free(vs);
  if (fs != nullptr) free(fs);
  return program;
}

void showUsage() {
  std::cerr << "Usage: ./mandelbulb -c\n"
            << "Options:\n"
            << "\t-h,--help\t\tShow this message\n"
            << "\t-w,--weak \t\tLower settings for weak computer i.e. shitty Intel HD graphics laptop\n"
            << "\t-c,--coordinates \tLog coordinates in console every frame \n\n"
            << "Controls:\n"
            << "\tQ \tQuit the program\n"
            << "\tL \tReload shaders\n"
            << "\tWASD \tMovement around center\n"
            << "\tZ \tZoom in\n"
            << "\tX \tZoom out\n"
            << "\tR \tReset position\n"
            << std::endl;
}

int handleArgs(int c, char *argv[], bool &logCoordinates, bool &weakSettings) {
  for (int i = 1; i < c; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      showUsage();
      return -1;
    } else if (arg == "-w" || arg == "--weak") {
      weakSettings = true;
    } else if (arg == "-c" || arg == "--coordinates") {
      logCoordinates = true;
    }
  }
  return 0;
}

}

#endif //MANDELBULB_UTILS_H
