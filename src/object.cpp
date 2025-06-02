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
    
    // Create rotation matrix around the specified orbit axis
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), ang, orbitAxis);
    // Start with unit vector along X and rotate it
    glm::vec3 offset = glm::vec3(R * glm::vec4(orbitRadius, 0, 0, 1));
    
    model[3] = glm::vec4(center + offset, 1.0f);
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
