#include "object.hpp"
#include "shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Object::update(float dt, float t)
{
  // Accumulate angles based on dt only
  spinAngle += spinSpeed * dt;
  orbitAngle += orbitSpeed * dt;

  // Start with base scale matrix (clean every frame)
  glm::mat4 M = glm::scale(glm::mat4(1.0f), localScale);

  // Apply self-rotation
  M = glm::rotate(M, spinAngle, axis);

  // Handle orbital motion
  if (orbitRadius > 0.0f)
  {
    glm::vec3 center = orbitTarget ? orbitTarget->position() : orbitCenter;
    
    // Create rotation matrix around the specified orbit axis
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), orbitAngle, orbitAxis);
    // Start with unit vector along X and rotate it
    glm::vec3 offset = glm::vec3(R * glm::vec4(orbitRadius, 0, 0, 1));
    
    // Set translation
    M[3] = glm::vec4(center + offset, 1.0f);
  }

  model = M;
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
