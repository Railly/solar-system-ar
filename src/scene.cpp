#include "scene.hpp"
#include "object.hpp"

void Scene::update(float dt, float t)
{
  for (auto *o : objects_)
    o->update(dt, t);
}
void Scene::draw(const Shader &sh, const glm::mat4 &VP)
{
  for (auto *o : objects_)
    o->draw(sh, VP);
}
