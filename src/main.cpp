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
  LOG_INF("Starting AR Solar System - Step 5: Camera Background");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "AR Solar System - Camera Background", nullptr, nullptr);
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

  // Solar system with fixed parameters (no UI controls)
  Object sun{sphere, Texture("assets/sun.jpg")};
  sun.localScale = glm::vec3(0.18f);  // Use localScale field
  sun.spinSpeed = glm::radians(15.f);  // 3x faster: 5°/s → 15°/s

  Object earth{sphere, Texture("assets/earth.jpg")};
  earth.localScale = glm::vec3(0.08f);  // Use localScale field
  earth.spinSpeed = glm::radians(90.f);  // 3x faster: 30°/s → 90°/s
  earth.orbitRadius = 0.4f;              
  earth.orbitSpeed = glm::radians(24.f);  // 3x faster: 8°/s → 24°/s
  earth.orbitAxis = glm::normalize(glm::vec3(0.1f, 0, 1)); 

  Object moon{sphere, Texture("assets/moon.jpg")};
  moon.localScale = glm::vec3(0.02f);  // Use localScale field
  moon.spinSpeed = glm::radians(60.f);  // 3x faster: 20°/s → 60°/s               
  moon.orbitRadius = 0.12f;  // Increased distance from Earth (was 0.08f - too close!)                                   
  moon.orbitSpeed = glm::radians(75.f);  // 3x faster: 25°/s → 75°/s              
  moon.orbitTarget = &earth;                                  
  moon.orbitAxis = glm::normalize(glm::vec3(0.1f, 0, 1));

  LOG_INF("Solar system created with fixed parameters");

  Scene scene;
  scene.add(&sun); 
  scene.add(&earth);
  scene.add(&moon);

  glEnable(GL_DEPTH_TEST);
  double last = glfwGetTime();

  ARTracker ar;
  static float alpha = 0.0f;   
  static const float HOVER_HEIGHT = 0.06f;  // fixed hover height
  static const float SYSTEM_SCALE = 0.3f;   // fixed system scale

  // FPS logging
  static double fpsTimer = 0;
  static int frames = 0;

  LOG_INF("Entering main loop - camera background + basic solar system");

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

    static bool loggedBg = false;
    if (!loggedBg && ar.hasValidFrame())
    {
      LOG_INF("Background camera feed rendered successfully");
      loggedBg = true;
    }

    // ---- update & draw solar system (basic rendering) ----
    if (ar.markerVisible() && alpha > 0.01f)
    {
      scene.update(dt, static_cast<float>(now));

      // Fixed transformations (no user controls)
      glm::mat4 hover = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, +HOVER_HEIGHT)); 
      glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(SYSTEM_SCALE)); 
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
      LOG_DBG("Drew solar system with fixed parameters, alpha %.2f", alpha);
    }

    glfwSwapBuffers(win);
    glfwPollEvents();

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(win, true);
  }

  LOG_INF("Shutting down");
  glfwTerminate();
  return 0;
}
