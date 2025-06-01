#pragma once
#include <glad/glad.h>

class Texture
{
public:
  explicit Texture(const char *path);
  void bind(GLenum unit = GL_TEXTURE0) const;

private:
  GLuint id_{};
};
