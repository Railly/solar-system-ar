#include "object.hpp"
#include "shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Object::update(float dt, float t)
{
  // auto-rotación
  model = glm::rotate(model, spinSpeed * dt, axis);

  // órbita
  if (orbitRadius > 0.0f)
  {
    glm::vec3 center = orbitTarget ? orbitTarget->position() : orbitCenter;
    float ang = orbitSpeed * t;
    glm::vec3 pos = center + orbitRadius * glm::vec3(cos(ang), 0, sin(ang));
    model[3] = glm::vec4(pos, 1.0f);
  }
}

void Object::draw(const Shader &sh, const glm::mat4 &VP) const
{
  sh.use();
  glm::mat4 MVP = VP * model;
  sh.setMat4("MVP", MVP);
  tex.bind();
  mesh.draw();
}
