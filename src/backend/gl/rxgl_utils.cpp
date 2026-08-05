
#include <GL/glew.h>

#include "rxgl_stuff.h"
#include "rxgl_base.h"

#include "rx_logsutils.h"

GLuint GL_CompileShader(const char *shaderName, const char *shaderSrc, GLenum shaderType) {
  GLuint shaderId = glCreateShader(shaderType);
  if ( !shaderId ) {
    glDeleteShader(shaderId);

    char *error = GL_GetShaderError(shaderId);
    glerrlog(
      "cant create %s shader for \"%s\", error: %s",
      shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
      shaderName,
      error);

    return 0;
  }

  // set the shader source for compiling
  glShaderSource(shaderId, 1, &shaderSrc, nullptr);

  // now compile the shader
  glCompileShader(shaderId);

  // check status
  GLint ok;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &ok);
  if ( !ok ) {
    char *error = GL_GetShaderError(shaderId);
    glerrlog(
      "cant create %s shader for \"%s\", error: %s",
      shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
      shaderName,
      error);

    glDeleteShader(shaderId);

    return 0;
  }

  return shaderId;
}

GLuint GL_CreateProgram(const char *programName) {
  GLuint programId = glCreateProgram();
  if ( !programId ) {
    char *error = GL_GetProgramError(programId);
    glerrlog("cant create shader program for \"%s\", error; %s", programName, error);

    glDeleteProgram(programId);

    return 0;
  }

  return programId;
}

void GL_LinkShadersToProgram(
  const char *programName,
  GLuint programId,
  GLuint vertexShaderId, GLuint fragmentShaderId,
  unsigned int flags) {

  M_GLAttachShader(programId, vertexShaderId, fragmentShaderId);

  glLinkProgram(programId);

  GLint linkOK;
  glGetProgramiv(programId, GL_LINK_STATUS, &linkOK);
  if ( !linkOK ) {
    char *error = GL_GetProgramError(programId);
    glerrlog("cant link program \"%s\", error: %s", programName, error);

    if ( flags & DELETE_PROGRAM_WHEN_FAIL )
      glDeleteProgram(programId);

    if ( flags & DELETE_VERTEX_SHADER_WHEN_FAIL )
      glDeleteShader(vertexShaderId);

    if ( flags & DELETE_FRAGMENT_SHADER_WHEN_FAIL )
      glDeleteShader(fragmentShaderId);

    return;
  }
}

GLuint GL_CreateFullProgram(
  const char *vertShaderName, const char *fragShaderName, const char *progShaderName,
  const char *vertShaderSrc, const char *fragShaderSrc,
  unsigned int flags) {

  GLuint vertShader = GL_CompileShader(vertShaderName, vertShaderSrc, GL_VERTEX_SHADER);
  GLuint fragShader = GL_CompileShader(fragShaderName, fragShaderSrc, GL_FRAGMENT_SHADER);

  GLuint shaderProg = GL_CreateProgram(progShaderName);
  GL_LinkShadersToProgram(progShaderName, shaderProg, vertShader, fragShader, flags);

  return shaderProg;
}

char *GL_GetShaderError(GLuint id) {
  static char buffer[GL_ERROR_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  glGetShaderInfoLog(id, GL_ERROR_BUFFER_SIZE, nullptr, buffer);

  return buffer;
}

char *GL_GetProgramError(GLuint id) {
  static char buffer[GL_ERROR_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  glGetProgramInfoLog(id, GL_ERROR_BUFFER_SIZE, nullptr, buffer);

  return buffer;
}
