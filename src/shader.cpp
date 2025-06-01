#include "shader.hpp"
#include <iostream>

GLuint Shader::compile(GLenum type, const char *src)
{
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok)
  {
    char log[512];
    glGetShaderInfoLog(s, 512, nullptr, log);
    std::cerr << log << '\n';
  }
  return s;
}

Shader::Shader(const char *vsSrc, const char *fsSrc)
{
  GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
  GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);
  id_ = glCreateProgram();
  glAttachShader(id_, vs);
  glAttachShader(id_, fs);
  glLinkProgram(id_);
  glDeleteShader(vs);
  glDeleteShader(fs);
}

void Shader::setMat4(const char *n, const glm::mat4 &m) const
{
  glUniformMatrix4fv(glGetUniformLocation(id_, n), 1, GL_FALSE, &m[0][0]);
}
