#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Object;
class Shader;

class Scene
{
public:
  void add(Object *o) { objects_.push_back(o); }
  void update(float dt, float t);
  void draw(const Shader &sh, const glm::mat4 &VP);

private:
  std::vector<Object *> objects_;
};
