#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <vector>
#include <cmath>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "object.hpp" 
#include "scene.hpp"
#include "logger.hpp"

#include <iostream>

// Simple texture shader 
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
out vec4 FragColor;
void main(){ FragColor = texture(tex, vUV); }
)";

int main()
{
  LOG_INF("Starting Solar System - Step 3: Hierarchical Orbits");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "Solar System - Hierarchical Orbits", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  Shader shader(VSHADER, FSHADER);
  Mesh sphere = Mesh::sphere();

  // Solar system objects
  Object sun{sphere, Texture("assets/sun.jpg")};
  sun.localScale = glm::vec3(0.6f);  // Large sun
  sun.spinSpeed = glm::radians(5.f);  // Very slow rotation

  Object earth{sphere, Texture("assets/earth.jpg")};  
  earth.localScale = glm::vec3(0.25f);  // Medium earth
  earth.spinSpeed = glm::radians(30.f);  // Moderate rotation
  earth.orbitRadius = 1.5f;  // Distance from sun
  earth.orbitSpeed = glm::radians(8.f);  // Slower orbital speed
  earth.orbitAxis = glm::normalize(glm::vec3(0.1f, 0, 1)); // Slightly tilted orbit

  Object moon{sphere, Texture("assets/moon.jpg")};
  moon.localScale = glm::vec3(0.08f);  // Small moon  
  moon.spinSpeed = glm::radians(20.f);  // Moderate rotation
  moon.orbitRadius = 0.6f;  // Much farther from earth - no collision!
  moon.orbitSpeed = glm::radians(25.f);  // Faster than Earth but not crazy
  moon.orbitTarget = &earth;  // *** CRITICAL: Moon orbits Earth, not Sun ***
  moon.orbitAxis = glm::normalize(glm::vec3(0.1f, 0, 1)); // Same inclination

  Scene scene;
  scene.add(&sun);
  scene.add(&earth);
  scene.add(&moon);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.02f, 0.02f, 0.1f, 1.0f);  // Dark space background

  double last = glfwGetTime();
  
  LOG_INF("Solar system initialized - Sun, Earth (orbiting Sun), Moon (orbiting Earth)");

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - last);
    last = now;

    // Time scale factor to slow everything down
    const float TIME_SCALE = 0.3f;  // 30% speed - much more observable
    
    // Update orbital mechanics with time scaling
    scene.update(dt * TIME_SCALE, static_cast<float>(now) * TIME_SCALE);

    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera setup - view from a fixed perspective
    glm::mat4 view = glm::lookAt(
      glm::vec3(3.0f, 2.0f, 3.0f),  // Camera position
      glm::vec3(0.0f, 0.0f, 0.0f),  // Look at origin (sun)
      glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );
    
    glm::mat4 proj = glm::perspective(
      glm::radians(45.0f), 
      static_cast<float>(w) / static_cast<float>(h), 
      0.1f, 100.0f
    );
    
    glm::mat4 VP = proj * view;

    // Render all objects
    shader.use();
    sun.draw(shader, VP);
    earth.draw(shader, VP);
    moon.draw(shader, VP);

    glfwSwapBuffers(win);
    glfwPollEvents();

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(win, true);
  }

  LOG_INF("Shutting down");
  glfwTerminate();
  return 0;
}
