#pragma once
#include <glad/glad.h>
#include <vector>

struct Mesh
{
  GLuint vao{}, vbo{}, ebo{};
  GLsizei indexCount{0};

  // Constructors
  Mesh() = default;
  Mesh(const Mesh&) = default;
  Mesh& operator=(const Mesh&) = default;
  Mesh(Mesh&&) = default;
  Mesh& operator=(Mesh&&) = default;

  static Mesh sphere(int seg = 64, int ring = 64);
  void draw() const
  {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  }
};
