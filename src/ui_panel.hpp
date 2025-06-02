#pragma once
#include "object.hpp"
#include <imgui.h>

inline void drawOrbitalPanel(Object &sun, Object &earth, Object &moon, float &hover, float &systemScale, 
                            float &lightIntensity, float &lightWarmth, bool *show = nullptr)
{
  if (show && !*show)
    return;
  ImGui::Begin("Orbital Control", show);
  
  ImGui::SeparatorText("System Settings");
  ImGui::SliderFloat("Height above tablet", &hover, 0.02f, 0.20f, "%.2f units");
  ImGui::SameLine();
  ImGui::TextDisabled("(away from surface)");
  
  ImGui::SliderFloat("System Scale", &systemScale, 0.1f, 1.0f, "%.2f×", ImGuiSliderFlags_Logarithmic);
  ImGui::SameLine();
  ImGui::TextDisabled("(smaller/larger)");
  
  ImGui::SeparatorText("Lighting");
  ImGui::SliderFloat("Sun intensity", &lightIntensity, 0.2f, 2.0f, "%.2f×");
  ImGui::SameLine();
  ImGui::TextDisabled("(light brightness)");
  
  ImGui::SliderFloat("Light warmth", &lightWarmth, 0.5f, 1.0f, "%.2f");
  ImGui::SameLine();
  ImGui::TextDisabled("(yellow/white)");

  static bool showLightDebug = false;
  ImGui::Checkbox("Show light debug", &showLightDebug);
  if (showLightDebug) {
    ImGui::Text("Light follows Sun's exact center position");
    ImGui::Text("including all transforms and scaling");
  }

  ImGui::SeparatorText("Sun");
  float sunDeg = glm::degrees(sun.spinSpeed);
  if (ImGui::SliderFloat("Sun spin (deg/s)", &sunDeg, 0.0f, 60.0f))
    sun.spinSpeed = glm::radians(sunDeg);
  
  ImGui::SeparatorText("Earth");
  float earthSpinDeg = glm::degrees(earth.spinSpeed);
  if (ImGui::SliderFloat("Earth spin (deg/s)", &earthSpinDeg, 0.0f, 180.0f))
    earth.spinSpeed = glm::radians(earthSpinDeg);
    
  float earthOrbitDeg = glm::degrees(earth.orbitSpeed);
  if (ImGui::SliderFloat("Earth orbit (deg/s)", &earthOrbitDeg, 0.0f, 60.0f))
    earth.orbitSpeed = glm::radians(earthOrbitDeg);
    
  ImGui::SliderFloat("Earth radius", &earth.orbitRadius, 0.05f, 4.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
  
  // Orbit axis control
  static const char* axes[] = {"X", "Y", "Z"};
  int earthAxis = earth.orbitAxis == glm::vec3(1,0,0) ? 0 : 
                  earth.orbitAxis == glm::vec3(0,1,0) ? 1 : 2;
  if (ImGui::Combo("Earth orbit axis", &earthAxis, axes, 3)) {
    earth.orbitAxis = earthAxis == 0 ? glm::vec3(1,0,0) :
                      earthAxis == 1 ? glm::vec3(0,1,0) : glm::vec3(0,0,1);
  }
  
  ImGui::SeparatorText("Moon");
  float moonSpinDeg = glm::degrees(moon.spinSpeed);
  if (ImGui::SliderFloat("Moon spin (deg/s)", &moonSpinDeg, 0.0f, 120.0f))
    moon.spinSpeed = glm::radians(moonSpinDeg);
    
  float moonOrbitDeg = glm::degrees(moon.orbitSpeed);
  if (ImGui::SliderFloat("Moon orbit (deg/s)", &moonOrbitDeg, 0.0f, 150.0f))
    moon.orbitSpeed = glm::radians(moonOrbitDeg);
    
  ImGui::SliderFloat("Moon radius", &moon.orbitRadius, 0.05f, 0.5f, "%.2f", ImGuiSliderFlags_Logarithmic);
  
  // Moon orbit axis control
  int moonAxis = moon.orbitAxis == glm::vec3(1,0,0) ? 0 : 
                 moon.orbitAxis == glm::vec3(0,1,0) ? 1 : 2;
  if (ImGui::Combo("Moon orbit axis", &moonAxis, axes, 3)) {
    moon.orbitAxis = moonAxis == 0 ? glm::vec3(1,0,0) :
                     moonAxis == 1 ? glm::vec3(0,1,0) : glm::vec3(0,0,1);
  }
  
  ImGui::End();
}
