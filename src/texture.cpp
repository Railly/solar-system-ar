#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "texture.hpp"
#include <iostream>

Texture::Texture(const char *path)
{
  int w, h, n;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &w, &h, &n, 0);
  if (!data)
  {
    std::cerr << "texture load failed: " << path << '\n';
    return;
  }

  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, n == 4 ? GL_RGBA : GL_RGB, w, h, 0,
               n == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
}

void Texture::bind(GLenum unit) const
{
  glActiveTexture(unit);
  glBindTexture(GL_TEXTURE_2D, id_);
}
