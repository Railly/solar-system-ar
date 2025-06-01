#pragma once
#include "object.hpp"
#include <imgui.h>

inline void drawOrbitalPanel(Object &sun, Object &earth, bool *show = nullptr)
{
  if (show && !*show)
    return;
  ImGui::Begin("Orbital Control", show);
  ImGui::SliderFloat("Sun spin", &sun.spinSpeed, 0.0f, glm::radians(60.f));
  ImGui::SliderFloat("Earth spin", &earth.spinSpeed, 0.0f, glm::radians(720.f));
  ImGui::SliderFloat("Earth orbit", &earth.orbitSpeed, 0.0f, glm::radians(720.f));
  ImGui::SliderFloat("Earth radius", &earth.orbitRadius, 1.0f, 10.0f);
  ImGui::End();
}
