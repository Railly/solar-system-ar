#include "scene.hpp"
#include "object.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Scene::update(float dt, float t)
{
  for (auto *o : objects_)
    o->update(dt, t);
    
  // Tilt orbital plane 20 degrees so Earth doesn't hide behind Sun
  glm::mat4 tilt = glm::rotate(glm::mat4(1.0f),
                               glm::radians(0.f),   // 20° around X axis
                               glm::vec3(1, 0, 0));
  
  for (auto *o : objects_)
    o->model = tilt * o->model;
}

void Scene::draw(const Shader &sh, const glm::mat4 &VP)
{
  for (auto *o : objects_)
    o->draw(sh, VP);
}
