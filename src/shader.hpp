#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
  Shader(const char *vertSrc, const char *fragSrc);
  void use() const { glUseProgram(id_); }
  void setMat4(const char *n, const glm::mat4 &m) const;
  void setMat3(const char *n, const glm::mat3 &m) const;
  GLuint id() const { return id_; }

private:
  GLuint id_;
  static GLuint compile(GLenum type, const char *src);
};
