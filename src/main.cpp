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
  LOG_INF("Starting Solar System - Step 2: Sun Texture & Rotation");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "Solar System - Sun Texture", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  Shader shader(VSHADER, FSHADER);
  Mesh sphere = Mesh::sphere();

  // Single Sun object
  Object sun{sphere, Texture("assets/sun.jpg")};
  sun.localScale = glm::vec3(1.0f);  // Normal size
  sun.spinSpeed = glm::radians(30.f);  // Visible but not too fast rotation

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.02f, 0.02f, 0.1f, 1.0f);  // Dark space background

  double last = glfwGetTime();
  
  LOG_INF("Sun initialized - textured sphere with rotation");

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - last);
    last = now;

    // Update Sun rotation
    sun.update(dt, static_cast<float>(now));

    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera setup - close view of the Sun
    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f, 0.0f, 3.0f),  // Camera closer to see texture detail
      glm::vec3(0.0f, 0.0f, 0.0f),  // Look at Sun (origin)
      glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );
    
    glm::mat4 proj = glm::perspective(
      glm::radians(45.0f), 
      static_cast<float>(w) / static_cast<float>(h), 
      0.1f, 100.0f
    );
    
    glm::mat4 VP = proj * view;

    // Render the Sun
    shader.use();
    sun.draw(shader, VP);

    glfwSwapBuffers(win);
    glfwPollEvents();

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(win, true);
  }

  LOG_INF("Shutting down");
  glfwTerminate();
  return 0;
}
