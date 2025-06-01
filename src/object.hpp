#pragma once
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include <glm/glm.hpp>

struct Object
{
  const Object *orbitTarget = nullptr;
  Mesh mesh;
  Texture tex;
  glm::mat4 model{1.0f};

  glm::vec3 axis{0, 1, 0};
  float spinSpeed = 0.0f; // rad/sec

  glm::vec3 orbitCenter{0};
  float orbitRadius = 0.0f;
  float orbitSpeed = 0.0f; // rad/sec

  void update(float dt, float t);
  void draw(const Shader &sh, const glm::mat4 &VP) const;
  glm::vec3 position() const { return glm::vec3(model[3]); }
};
