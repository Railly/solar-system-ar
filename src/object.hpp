#pragma once
#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include <glm/glm.hpp>

struct Object
{
  // Core components
  Mesh mesh;
  Texture tex;
  glm::mat4 model{1.0f};

  // Rotation properties
  glm::vec3 axis{0, 1, 0};
  float spinSpeed = 0.0f; // rad/sec
  float spinAngle = 0.0f; // accumulated rotation angle

  // Orbital properties
  glm::vec3 orbitCenter{0};
  float orbitRadius = 0.0f;
  float orbitSpeed = 0.0f; // rad/sec
  float orbitAngle = 0.0f; // accumulated orbital angle
  glm::vec3 orbitAxis{0, 1, 0}; // axis around which to orbit (default Y-axis)
  const Object *orbitTarget = nullptr;

  // Scale property (separated from model matrix)
  glm::vec3 localScale{1.0f};

  // Constructor
  Object(const Mesh& m, const Texture& t) : mesh(m), tex(t) {}

  // Methods
  void update(float dt, float t);
  void draw(const Shader &sh, const glm::mat4 &VP) const;
  void draw(const Shader &sh, const glm::mat4 &VP, const glm::mat4 &view, const glm::mat4 &transform) const; // lit version
  glm::vec3 position() const { return glm::vec3(model[3]); }
};
