#include "object.hpp"
#include "shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Object::update(float dt, float t)
{
  // Accumulate angles based only on dt (no time absolute dependency)
  spinAngle += spinSpeed * dt;
  orbitAngle += orbitSpeed * dt;

  // Rebuild model matrix cleanly each frame (no cumulative errors)
  model = glm::scale(glm::mat4(1.0f), localScale);

  // Apply rotation around own axis
  model = glm::rotate(model, spinAngle, axis);

  // Handle orbital movement
  if (orbitRadius > 0.0f)
  {
    glm::vec3 center = orbitTarget ? orbitTarget->position() : orbitCenter;
    
    // Create rotation matrix around the specified orbit axis
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), orbitAngle, orbitAxis);
    // Start with unit vector along X and rotate it
    glm::vec3 offset = glm::vec3(R * glm::vec4(orbitRadius, 0, 0, 1));
    
    // Set position (translation part of matrix)
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

void Object::draw(const Shader &sh, const glm::mat4 &VP, const glm::mat4 &view, const glm::mat4 &transform) const
{
  sh.use();
  
  glm::mat4 MV = view * transform * model;
  glm::mat4 MVP = VP * model;
  glm::mat3 NormalM = glm::transpose(glm::inverse(glm::mat3(MV)));
  
  sh.setMat4("MVP", MVP);
  sh.setMat4("MV", MV);
  sh.setMat3("NormalM", NormalM);
  
  tex.bind();
  mesh.draw();
}
