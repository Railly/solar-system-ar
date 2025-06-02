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

// Lit shaders for planets (with lighting)
static const char *LIT_VSHADER = R"(
#version 410 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec3 aNrm;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 NormalM;

out vec2 vUV;
out vec3 vNormal;
out vec3 vViewPos;

void main() {
    vUV = aUV;
    vNormal = NormalM * aNrm;
    vViewPos = vec3(MV * vec4(aPos, 1.0));
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

static const char *LIT_FSHADER = R"(
#version 410 core
in vec2 vUV;
in vec3 vNormal;
in vec3 vViewPos;

uniform sampler2D tex;
uniform vec3 lightPosVS;
uniform vec3 lightColor;
uniform float uAlpha;

out vec4 FragColor;

void main() {
    vec3 N = normalize(-vNormal);  // Flip normal to point outward
    vec3 L = normalize(lightPosVS - vViewPos);
    vec3 V = normalize(-vViewPos);
    vec3 R = reflect(-L, N);
    
    // Lighting calculations
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(R, V), 0.0), 32.0);
    
    // Add hemisphere lighting for better fill (simulates sky light)
    vec3 skyDir = vec3(0, 1, 0); // up direction in view space
    float hemisphere = 0.25 * max(dot(N, skyDir), 0.0);
    
    vec3 albedo = texture(tex, vUV).rgb;
    vec3 ambient = 0.15 * albedo;
    vec3 diffuse = diff * albedo * lightColor;
    vec3 specular = spec * 0.3 * lightColor;
    vec3 fill = hemisphere * albedo * lightColor * 0.4; // subtle fill light
    
    vec3 color = ambient + diffuse + specular + fill;
    FragColor = vec4(color, uAlpha);
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
  LOG_INF("Starting AR Solar System");

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

  Shader shader(VSHADER, FSHADER);        // unlit shader for Sun
  Shader litShader(LIT_VSHADER, LIT_FSHADER); // lit shader for planets
  Shader bgShader(BG_VSHADER, BG_FSHADER);
  Mesh sphere = Mesh::sphere();

  // Create background quad for AR camera feed (correct vertex order for TRIANGLE_STRIP)
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

  // Visible solar system scales (all in marker units)
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

  LOG_INF("Solar system created - Sun:%.3f Earth:%.3f Moon:%.3f", 0.18f, 0.08f, 0.02f);

  Scene scene;
  scene.add(&sun); // put them back so scene.update() moves them
  scene.add(&earth);
  scene.add(&moon);

  glEnable(GL_DEPTH_TEST);
  double last = glfwGetTime();

  ui::ImGuiLayer gui;
  gui.init(win);

  ARTracker ar;
  bool showUI = true;
  static float alpha = 0.0f;   // for smooth fade in/out
  static float gHover = 0.06f; // hover height above marker (closer to tablet for demo)
  static float gSystemScale = 0.3f; // smaller system by default for demo
  static float gLightIntensity = 0.8f; // sun light intensity (reduced from 1.0 for realism)
  static float gLightWarmth = 0.95f; // light warmth (yellow vs white)

  // FPS logging
  static double fpsTimer = 0;
  static int frames = 0;

  LOG_INF("Entering main loop");

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - last);
    last = now;

    ar.grabFrame(); // updates V, P + bg texture

    // FPS and status logging
    fpsTimer += dt;
    ++frames;
    if (fpsTimer > 2.0)
    { // every 2 seconds
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
    drawOrbitalPanel(sun, earth, moon, gHover, gSystemScale, gLightIntensity, gLightWarmth, &showUI);

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
    glDisable(GL_CULL_FACE); // ensure quad draws regardless of winding
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_CULL_FACE); // re-enable for 3D objects
    glEnable(GL_DEPTH_TEST);

    static bool loggedBg = false;
    if (!loggedBg && ar.hasValidFrame())
    {
      LOG_INF("Background quad rendered successfully");
      loggedBg = true;
    }

    // ---- update & draw solar system (only when marker visible and alpha > 0) ----
    if (ar.markerVisible() && alpha > 0.01f)
    {
      // Debug: Check if sun is in front of camera
      static int debugCounter = 0;
      if (++debugCounter % 60 == 0)
      { // every 2 seconds at 30fps
        glm::vec4 sunViewPos = ar.view() * glm::vec4(0, 0, 0, 1);
        glm::vec4 earthViewPos = ar.view() * earth.model * glm::vec4(0, 0, 0, 1);
        glm::vec4 offsetPos = gHover * glm::vec4(0, 0, 0, 1); // origin after offset
        LOG_INF("Sun in view space: (%.3f, %.3f, %.3f)", sunViewPos.x, sunViewPos.y, sunViewPos.z);
        LOG_INF("Earth in view space: (%.3f, %.3f, %.3f)", earthViewPos.x, earthViewPos.y, earthViewPos.z);
        LOG_INF("Hover offset: (%.2f, %.2f, %.2f) - Z should be +%.2f",
                offsetPos.x, offsetPos.y, offsetPos.z, gHover);
      }

      scene.update(dt, static_cast<float>(now));

      // Move entire system above marker along its +Z axis (away from tablet surface)
      glm::mat4 hover = glm::translate(glm::mat4(1.0f),
                                       glm::vec3(0, 0, +gHover)); // POSITIVE = above tablet
      glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(gSystemScale)); // global scale
      glm::mat4 transform = hover * scaling;
      glm::mat4 VP = ar.proj() * ar.view() * transform;

      // Calculate Sun's actual center position in view space for lighting
      glm::vec3 sunPosVS = glm::vec3(ar.view() * transform * sun.model * glm::vec4(0, 0, 0, 1));

      // Debug: Log lighting positions occasionally
      static int lightDebugCounter = 0;
      if (++lightDebugCounter % 120 == 0) { // every 4 seconds at 30fps
        glm::vec3 earthPosWorld = glm::vec3(transform * earth.model * glm::vec4(0, 0, 0, 1));
        glm::vec3 sunPosWorld = glm::vec3(transform * sun.model * glm::vec4(0, 0, 0, 1));
        glm::vec3 earthPosVS = glm::vec3(ar.view() * glm::vec4(earthPosWorld, 1));
        
        LOG_INF("LIGHTING DEBUG:");
        LOG_INF("  Sun world: (%.2f, %.2f, %.2f)", sunPosWorld.x, sunPosWorld.y, sunPosWorld.z);
        LOG_INF("  Earth world: (%.2f, %.2f, %.2f)", earthPosWorld.x, earthPosWorld.y, earthPosWorld.z);
        LOG_INF("  Sun view: (%.2f, %.2f, %.2f)", sunPosVS.x, sunPosVS.y, sunPosVS.z);
        LOG_INF("  Earth view: (%.2f, %.2f, %.2f)", earthPosVS.x, earthPosVS.y, earthPosVS.z);
        
        glm::vec3 lightDir = glm::normalize(earthPosWorld - sunPosWorld);
        LOG_INF("  Light direction to Earth: (%.2f, %.2f, %.2f)", lightDir.x, lightDir.y, lightDir.z);
      }

      // Enable alpha blending for fade effect
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // 1) Draw planets with lighting
      litShader.use();
      glUniform1f(glGetUniformLocation(litShader.id(), "uAlpha"), alpha);
      glUniform3fv(glGetUniformLocation(litShader.id(), "lightPosVS"), 1, &sunPosVS[0]);
      glUniform3f(glGetUniformLocation(litShader.id(), "lightColor"), 
                  1.0f * gLightIntensity, gLightWarmth * gLightIntensity, 0.8f * gLightIntensity); // warm sunlight
      
      earth.draw(litShader, VP, ar.view(), transform);
      moon.draw(litShader, VP, ar.view(), transform);

      // 2) Draw Sun last with unlit shader (emissive)
      glDepthMask(GL_FALSE);
      shader.use();
      glUniform1f(glGetUniformLocation(shader.id(), "uAlpha"), alpha);
      sun.draw(shader, VP);
      glDepthMask(GL_TRUE);

      glDisable(GL_BLEND);
      LOG_DBG("Drew solar system with alpha %.2f", alpha);
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
