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
#include "logger.hpp"

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
void main(){ FragColor = texture(tex, vUV); }
)";

int main()
{
  LOG_INF("Starting AR Solar System - Step 6: GUI Controls");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "AR Solar System - GUI Controls", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  Shader shader(VSHADER, FSHADER);        // basic shader for all objects
  Shader bgShader(BG_VSHADER, BG_FSHADER);
  Mesh sphere = Mesh::sphere();

  // Create background quad for AR camera feed
  GLuint bgVAO, bgVBO;
  float quad[] = {
      // pos.xy   uv
      -1.f, -1.f, 0.f, 1.f, // lower-left
      1.f, -1.f, 1.f, 1.f,  // lower-right
      -1.f, 1.f, 0.f, 0.f,  // upper-left
      1.f, 1.f, 1.f, 0.f    // upper-right
  };
  glGenVertexArrays(1, &bgVAO);
  glBindVertexArray(bgVAO);
  glGenBuffers(1, &bgVBO);
  glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Solar system with basic rendering (no complex lighting)
  Object sun{sphere, Texture("assets/sun.jpg")};
  sun.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.18f)); 
  sun.spinSpeed = glm::radians(15.f);                        

  Object earth{sphere, Texture("assets/earth.jpg")};
  earth.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.08f)); 
  earth.spinSpeed = glm::radians(360.f / 24.f);
  earth.orbitRadius = 0.4f;              
  earth.orbitSpeed = glm::radians(15.f); 
  earth.orbitAxis = glm::normalize(glm::vec3(0.1f, 0, 1)); 

  Object moon{sphere, Texture("assets/moon.jpg")};
  moon.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f)); 
  moon.spinSpeed = glm::radians(360.f / 27.3f);               
  moon.orbitRadius = 0.08f;                                   
  moon.orbitSpeed = glm::radians(360.f / 27.3f);              
  moon.orbitTarget = &earth;                                  
  moon.orbitAxis = glm::normalize(glm::vec3(0.1f, 0, 1));     

  LOG_INF("Solar system created with basic rendering");

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
  static float alpha = 0.0f;   
  static float gHover = 0.06f; 
  static float gSystemScale = 0.3f; 
  // Removed lighting controls since we're using basic rendering

  // FPS logging
  static double fpsTimer = 0;
  static int frames = 0;

  LOG_INF("Entering main loop");

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - last);
    last = now;

    ar.grabFrame(); 

    // FPS and status logging
    fpsTimer += dt;
    ++frames;
    if (fpsTimer > 2.0)
    { 
      LOG_INF("FPS: %d  alpha: %.2f  marker: %s  frame: %s",
              frames / 2, alpha, ar.markerVisible() ? "yes" : "no", ar.hasValidFrame() ? "valid" : "empty");
      fpsTimer = 0;
      frames = 0;
    }

    // Skip rendering entirely if no valid camera frame yet
    if (!ar.hasValidFrame())
    {
      LOG_DBG("No valid frame yet, continuing...");
      glfwPollEvents();
      continue;
    }

    // Smooth fade in/out based on marker visibility
    alpha = ar.markerVisible() ? std::min(alpha + dt * 4.0f, 1.0f)
                               : std::max(alpha - dt * 4.0f, 0.0f);

    gui.begin();
    // Simplified UI panel without lighting controls
    drawOrbitalPanel(sun, earth, moon, gHover, gSystemScale, gSystemScale, gSystemScale, &showUI); // reuse scale param

    // Debug feedback when no marker detected
    if (!ar.markerVisible())
    {
      ImGui::Begin("AR Status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "🎯 Point camera at ArUco marker");
      ImGui::Text("Marker ID: 0 (DICT_6X6_250)");
      ImGui::Text("Alpha: %.2f", alpha);
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
    glDisable(GL_CULL_FACE); 
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_CULL_FACE); 
    glEnable(GL_DEPTH_TEST);

    // ---- update & draw solar system (basic rendering) ----
    if (ar.markerVisible() && alpha > 0.01f)
    {
      scene.update(dt, static_cast<float>(now));

      // Move entire system above marker
      glm::mat4 hover = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, +gHover)); 
      glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(gSystemScale)); 
      glm::mat4 transform = hover * scaling;
      glm::mat4 VP = ar.proj() * ar.view() * transform;

      // Enable alpha blending for fade effect
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Draw all objects with basic shader (no lighting)
      shader.use();
      glUniform1f(glGetUniformLocation(shader.id(), "uAlpha"), alpha);
      
      sun.draw(shader, VP);
      earth.draw(shader, VP);
      moon.draw(shader, VP);

      glDisable(GL_BLEND);
      LOG_DBG("Drew solar system with basic rendering, alpha %.2f", alpha);
    }

    gui.end();
    glfwSwapBuffers(win);
    glfwPollEvents();

    if (glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS)
      showUI = !showUI;
  }

  LOG_INF("Shutting down");
  gui.shutdown();
  glfwTerminate();
  return 0;
}
