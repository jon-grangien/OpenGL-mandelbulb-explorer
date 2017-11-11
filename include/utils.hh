#include <cstdio>
#include <cstring>
#include <string>

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

char* readFile(char * file) {
  FILE *fptr;
  long length;
  char *buf;

  if (file == nullptr)
    return nullptr;

  fptr = fopen(file, "rb");

  if (!fptr) /* Return NULL on failure */
    return nullptr;

  fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
  length = ftell(fptr); /* Find out how many bytes into the file we are */
  buf = (char*) malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
  memset(buf, 0, sizeof(char)*(length + 1)); /* Clean the buffer - suggested for safety by Mateusz 2016 */
  fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
  fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
  fclose(fptr); /* Close the file */
  buf[length] = 0; /* Null terminator */

  return buf; /* Return the buffer */
}

void printShaderInfoLog(GLuint obj, const char *fn)
{
  GLint infologLength = 0;
  GLint charsWritten  = 0;
  char *infoLog;

  glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 2)
  {
    fprintf(stderr, "[From %s:]\n", fn);
    infoLog = (char *)malloc(infologLength);
    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
    fprintf(stderr, "%s\n",infoLog);
    free(infoLog);
  }
}


void printProgramInfoLog(GLuint obj, const char *vfn, const char *ffn,
                         const char *gfn, const char *tcfn, const char *tefn)
{
  GLint infologLength = 0;
  GLint charsWritten  = 0;
  char *infoLog;

  glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

  if (infologLength > 2)
  {
    if (gfn == NULL)
      fprintf(stderr, "[From %s+%s:]\n", vfn, ffn);
    else
    if (tcfn == NULL || tefn == NULL)
      fprintf(stderr, "[From %s+%s+%s:]\n", vfn, ffn, gfn);
    else
      fprintf(stderr, "[From %s+%s+%s+%s+%s:]\n", vfn, ffn, gfn, tcfn, tefn);
    infoLog = (char *)malloc(infologLength);
    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
    fprintf(stderr, "%s\n",infoLog);
    free(infoLog);
  }
}

GLuint compileShaders(const char *vs, const char *fs, const char *gs, const char *tcs, const char *tes,
                      const char *vfn, const char *ffn, const char *gfn, const char *tcfn, const char *tefn) {
  GLuint v,f,g,tc,te,p;

  v = glCreateShader(GL_VERTEX_SHADER);
  f = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(v, 1, &vs, NULL);
  glShaderSource(f, 1, &fs, NULL);
  glCompileShader(v);
  glCompileShader(f);
  if (gs != NULL)
  {
    g = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(g, 1, &gs, NULL);
    glCompileShader(g);
  }
#ifdef GL_TESS_CONTROL_SHADER
  if (tcs != NULL)
  {
    tc = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tc, 1, &tcs, NULL);
    glCompileShader(tc);
  }
  if (tes != NULL)
  {
    te = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(te, 1, &tes, NULL);
    glCompileShader(te);
  }
#endif
  p = glCreateProgram();
  glAttachShader(p,v);
  glAttachShader(p,f);
  if (gs != NULL)
    glAttachShader(p,g);
  if (tcs != NULL)
    glAttachShader(p,tc);
  if (tes != NULL)
    glAttachShader(p,te);
  glLinkProgram(p);
  glUseProgram(p);

  printShaderInfoLog(v, vfn);
  printShaderInfoLog(f, ffn);
  if (gs != NULL)	printShaderInfoLog(g, gfn);
  if (tcs != NULL)	printShaderInfoLog(tc, tcfn);
  if (tes != NULL)	printShaderInfoLog(te, tefn);

  printProgramInfoLog(p, vfn, ffn, gfn, tcfn, tefn);

  return p;
}


GLuint loadShadersGT(const char *vertFileName, const char *fragFileName, const char *geomFileName,
                       const char *tcFileName, const char *teFileName) {
  char *vs, *fs, *gs, *tcs, *tes;
  GLuint p = 0;

  vs = readFile((char *)vertFileName);
  fs = readFile((char *)fragFileName);
  gs = readFile((char *)geomFileName);
  tcs = readFile((char *)tcFileName);
  tes = readFile((char *)teFileName);
  if (vs==nullptr) {
    //fprintf(stderr, "Failed to read %s from disk.\n", vertFileName);
    perror("Failed to read vertex from disk.\n");
  }
  if (fs==nullptr)
    fprintf(stderr, "Failed to read %s from disk.\n", fragFileName);
  if ((gs==nullptr) && (geomFileName != nullptr))
    fprintf(stderr, "Failed to read %s from disk.\n", geomFileName);
  if ((tcs==nullptr) && (tcFileName != nullptr))
    fprintf(stderr, "Failed to read %s from disk.\n", tcFileName);
  if ((tes==nullptr) && (teFileName != nullptr))
    fprintf(stderr, "Failed to read %s from disk.\n", teFileName);
  if ((vs!=nullptr)&&(fs!=nullptr))
    p = compileShaders(vs, fs, gs, tcs, tes, vertFileName, fragFileName, geomFileName, tcFileName, teFileName);
  if (vs != nullptr) free(vs);
  if (fs != nullptr) free(fs);
  if (gs != nullptr) free(gs);
  if (tcs != nullptr) free(tcs);
  if (tes != nullptr) free(tes);
  return p;
}

GLuint loadShaders(const char *vertFileName, const char *fragFileName) {
  return loadShadersGT(vertFileName, fragFileName, nullptr, nullptr, nullptr);
}

}

#endif //MANDELBULB_UTILS_H
