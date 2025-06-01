#include "mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

Mesh Mesh::sphere(int seg, int ring)
{
  std::vector<float> verts;
  std::vector<unsigned> idx;

  for (int y = 0; y <= ring; ++y)
  {
    float v = float(y) / ring, phi = v * glm::pi<float>();
    for (int x = 0; x <= seg; ++x)
    {
      float u = float(x) / seg, theta = u * 2 * glm::pi<float>();
      float sx = sin(phi) * cos(theta);
      float sy = cos(phi);
      float sz = sin(phi) * sin(theta);
      verts.insert(verts.end(), {sx, sy, sz, u, 1.0f - v});
    }
  }
  for (int y = 0; y < ring; ++y)
    for (int x = 0; x < seg; ++x)
    {
      unsigned a = y * (seg + 1) + x;
      unsigned b = a + seg + 1;
      idx.insert(idx.end(), {a, b, a + 1, b, b + 1, a + 1});
    }

  Mesh m;
  m.indexCount = static_cast<GLsizei>(idx.size());

  glGenVertexArrays(1, &m.vao);
  glBindVertexArray(m.vao);

  glGenBuffers(1, &m.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &m.ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return m;
}
