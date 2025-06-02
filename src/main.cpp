#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <vector>
#include <cmath>

#include "shader.hpp"
#include "ar_tracker.hpp"
#include "logger.hpp"

#include <iostream>

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

// Thick axis shader
static const char *AXIS_VSHADER = R"(
#version 410 core
layout(location=0) in vec3 aPos;
uniform mat4 MVP;
void main(){ gl_Position = MVP * vec4(aPos, 1.0); }
)";

static const char *AXIS_FSHADER = R"(
#version 410 core
uniform vec3 uColor;
out vec4 FragColor;
void main(){ FragColor = vec4(uColor, 1.0); }
)";

// Generate thick cylinder geometry for axes
struct AxisGeometry {
  GLuint VAO, VBO, EBO;
  GLsizei indexCount;
};

AxisGeometry createThickAxis(float radius, float length, int segments = 8) {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  
  // Create cylinder along Z-axis (0,0,0) to (0,0,length)
  for (int i = 0; i <= segments; i++) {
    float theta = 2.0f * M_PI * i / segments;
    float x = radius * cos(theta);
    float y = radius * sin(theta);
    
    // Bottom circle (z=0)
    vertices.insert(vertices.end(), {x, y, 0.0f});
    // Top circle (z=length)  
    vertices.insert(vertices.end(), {x, y, length});
  }
  
  // Generate indices for cylinder sides
  for (int i = 0; i < segments; i++) {
    int bottom1 = i * 2;
    int top1 = i * 2 + 1;
    int bottom2 = ((i + 1) % (segments + 1)) * 2;
    int top2 = ((i + 1) % (segments + 1)) * 2 + 1;
    
    // Two triangles per quad
    indices.insert(indices.end(), {static_cast<unsigned int>(bottom1), static_cast<unsigned int>(top1), static_cast<unsigned int>(bottom2)});
    indices.insert(indices.end(), {static_cast<unsigned int>(bottom2), static_cast<unsigned int>(top1), static_cast<unsigned int>(top2)});
  }
  
  AxisGeometry geom;
  glGenVertexArrays(1, &geom.VAO);
  glBindVertexArray(geom.VAO);
  
  glGenBuffers(1, &geom.VBO);
  glBindBuffer(GL_ARRAY_BUFFER, geom.VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
  
  glGenBuffers(1, &geom.EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  geom.indexCount = indices.size();
  return geom;
}

int main()
{
  LOG_INF("Starting AR Solar System - Step 4: ArUco Marker Detection");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "AR Solar System - Marker Detection", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  Shader bgShader(BG_VSHADER, BG_FSHADER);
  Shader axisShader(AXIS_VSHADER, AXIS_FSHADER);

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

  // Create thick axis geometry
  float axisRadius = 0.003f;  // 3mm thick axes - very visible!
  float axisLength = 0.05f;   // 5cm long
  AxisGeometry thickAxis = createThickAxis(axisRadius, axisLength, 8);
  
  LOG_INF("Created thick axes: radius=%.1fmm, length=%.1fcm", axisRadius*1000, axisLength*100);

  glEnable(GL_DEPTH_TEST);
  double last = glfwGetTime();

  ARTracker ar;

  // FPS logging
  static double fpsTimer = 0;
  static int frames = 0;
  static bool markerDetectedBefore = false;

  LOG_INF("Entering main loop - camera feed + marker detection with THICK 3D axes");

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
      LOG_INF("FPS: %d  marker: %s  frame: %s",
              frames / 2, 
              ar.markerVisible() ? "DETECTED" : "not found", 
              ar.hasValidFrame() ? "valid" : "empty");
      fpsTimer = 0;
      frames = 0;
    }

    // Log marker detection events
    if (ar.markerVisible() && !markerDetectedBefore) {
      LOG_INF("🎯 MARKER DETECTED! ID 0 - showing THICK 3D axes (X=red, Y=green, Z=blue)");
      markerDetectedBefore = true;
    } else if (!ar.markerVisible() && markerDetectedBefore) {
      LOG_INF("❌ Marker lost - point camera back at ArUco marker");
      markerDetectedBefore = false;
    }

    // Skip rendering entirely if no valid camera frame yet
    if (!ar.hasValidFrame())
    {
      LOG_DBG("No valid frame yet, continuing...");
      glfwPollEvents();
      continue;
    }

    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---- draw camera background (always) ----
    bgShader.use();
    glBindTexture(GL_TEXTURE_2D, ar.backgroundTex());
    glBindVertexArray(bgVAO);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); 
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_CULL_FACE); 
    glEnable(GL_DEPTH_TEST);

    static bool loggedCam = false;
    if (!loggedCam && ar.hasValidFrame()) {
      LOG_INF("✅ Camera feed active - point at ArUco marker ID 0");
      loggedCam = true;
    }

    // ---- draw THICK 3D axes when marker is detected ----
    if (ar.markerVisible()) {
      glm::mat4 VP = ar.proj() * ar.view();
      
      axisShader.use();
      glBindVertexArray(thickAxis.VAO);
      
      // X axis - Red (rotate Z-axis cylinder to point along +X)
      glm::mat4 xTransform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
      glm::mat4 xMVP = VP * xTransform;
      glUniform3f(glGetUniformLocation(axisShader.id(), "uColor"), 1.0f, 0.0f, 0.0f);
      glUniformMatrix4fv(glGetUniformLocation(axisShader.id(), "MVP"), 1, GL_FALSE, &xMVP[0][0]);
      glDrawElements(GL_TRIANGLES, thickAxis.indexCount, GL_UNSIGNED_INT, 0);
      
      // Y axis - Green (rotate Z-axis cylinder to point along +Y)
      glm::mat4 yTransform = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
      glm::mat4 yMVP = VP * yTransform;
      glUniform3f(glGetUniformLocation(axisShader.id(), "uColor"), 0.0f, 1.0f, 0.0f);
      glUniformMatrix4fv(glGetUniformLocation(axisShader.id(), "MVP"), 1, GL_FALSE, &yMVP[0][0]);
      glDrawElements(GL_TRIANGLES, thickAxis.indexCount, GL_UNSIGNED_INT, 0);
      
      // Z axis - Blue (cylinder already aligned with +Z)
      glm::mat4 zMVP = VP; // no rotation needed
      glUniform3f(glGetUniformLocation(axisShader.id(), "uColor"), 0.0f, 0.0f, 1.0f);
      glUniformMatrix4fv(glGetUniformLocation(axisShader.id(), "MVP"), 1, GL_FALSE, &zMVP[0][0]);
      glDrawElements(GL_TRIANGLES, thickAxis.indexCount, GL_UNSIGNED_INT, 0);
      
      // Debug: Log marker pose occasionally
      static int poseDebugCounter = 0;
      if (++poseDebugCounter % 60 == 0) { // every 2 seconds at 30fps
        glm::mat4 view = ar.view();
        glm::vec3 markerPos = glm::vec3(view[3]); // translation part
        LOG_INF("Marker ID 0 pose - position: (%.3f, %.3f, %.3f) - THICK axes visible!", 
                markerPos.x, markerPos.y, markerPos.z);
      }
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
