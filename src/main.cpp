#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "object.hpp"
#include "scene.hpp"
#include "ar_tracker.hpp"

#include "imgui_layer.hpp"
#include "ui_panel.hpp"

#include <iostream>

static const char *VSHADER = R"(
#version 410 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
uniform mat4 MVP;
out vec2 vUV;
void main(){ vUV=aUV; gl_Position=MVP*vec4(aPos,1.0); }
)";

static const char *FSHADER = R"(
#version 410 core
in vec2 vUV; 
uniform sampler2D tex; 
uniform float uAlpha;
out vec4 FragColor;
void main(){ 
  FragColor = texture(tex, vUV);
  FragColor.a *= uAlpha;
}
)";

// Background shaders (for AR camera feed)
static const char *BG_VSHADER = R"(
#version 410 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
out vec2 vUV;
void main(){ vUV=aUV; gl_Position=vec4(aPos,0.0,1.0); }
)";

static const char *BG_FSHADER = R"(
#version 410 core
in vec2 vUV; uniform sampler2D tex; out vec4 FragColor;
void main(){ FragColor = texture(tex, vec2(vUV.x, 1.0-vUV.y)); }
)";

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "AR Solar System", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  Shader shader(VSHADER, FSHADER);
  Shader bgShader(BG_VSHADER, BG_FSHADER);
  Mesh sphere = Mesh::sphere();

  // Create background quad for AR camera feed
  GLuint bgVAO, bgVBO;
  float quad[16] = {
      // pos      // uv
      -1, -1, 0, 0, 1, -1, 1, 0,
      1, 1, 1, 1, -1, 1, 0, 1};
  glGenVertexArrays(1, &bgVAO);
  glBindVertexArray(bgVAO);
  glGenBuffers(1, &bgVBO);
  glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  Object sun{sphere, Texture("assets/sun.jpg")};
  sun.spinSpeed = glm::radians(14.f); // arbitrary

  Object earth{sphere, Texture("assets/earth.jpg")};
  earth.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
  earth.spinSpeed = glm::radians(360.f / 24.f);
  earth.orbitRadius = 4.0f;
  earth.orbitSpeed = glm::radians(360.f / 365.f);

  Object moon{sphere, Texture("assets/moon.jpg")};
  moon.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.073f)); // radius ~ 0.27 Earth
  moon.spinSpeed = glm::radians(360.f / 27.3f);                // synchronous ≈ equal to translation
  moon.orbitRadius = 0.5f;                                     // scale of your scene
  moon.orbitSpeed = glm::radians(360.f / 27.3f);               // 27.3 d ≈ 1 sidereal month
  moon.orbitTarget = &earth;                                   // key!

  Scene scene;
  scene.add(&sun);
  scene.add(&earth);
  scene.add(&moon);

  glEnable(GL_DEPTH_TEST);
  double last = glfwGetTime();

  ui::ImGuiLayer gui;
  gui.init(win);

  ARTracker ar;
  bool showUI = true;
  static float alpha = 0.0f;  // for smooth fade in/out

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - last);
    last = now;
    
    ar.grabFrame(); // updates V, P + bg texture

    // Smooth fade in/out based on marker visibility
    alpha = ar.markerVisible() ? std::min(alpha + dt * 4.0f, 1.0f)
                               : std::max(alpha - dt * 4.0f, 0.0f);

    gui.begin();
    drawOrbitalPanel(sun, earth, moon, &showUI);
    
    // Debug feedback when no marker detected
    if (!ar.markerVisible()) {
      ImGui::Begin("AR Status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "🎯 Point camera at ArUco marker");
      ImGui::Text("Marker ID: 0 (DICT_6X6_250)");
      ImGui::End();
    }

    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---- draw background quad (always) ----
    bgShader.use();
    glBindTexture(GL_TEXTURE_2D, ar.backgroundTex());
    glBindVertexArray(bgVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_DEPTH_TEST);

    // ---- update & draw solar system (with fade effect) ----
    if (alpha > 0.01f) {  // only draw if visible enough
      scene.update(dt, static_cast<float>(now));
      
      // Enable alpha blending for fade effect
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      shader.use();
      glUniform1f(glGetUniformLocation(shader.id(), "uAlpha"), alpha);
      scene.draw(shader, ar.proj() * ar.view());
      
      glDisable(GL_BLEND);
    }

    gui.end();
    glfwSwapBuffers(win);
    glfwPollEvents();

    if (glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS)
      showUI = !showUI;
  }

  gui.shutdown();
  glfwTerminate();
  return 0;
}
