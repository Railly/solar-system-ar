#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cmath>

#include "logger.hpp"

#include <iostream>

int main()
{
  LOG_INF("Starting OpenGL Window - Step 1: GLFW + Glad Context");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "OpenGL Window - Animated Clear", nullptr, nullptr);
  if (!win) {
    LOG_ERR("Failed to create GLFW window");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(win);
  
  if (!gladLoadGL()) {
    LOG_ERR("Failed to initialize Glad");
    return -1;
  }

  LOG_INF("OpenGL Context: %s", glGetString(GL_VERSION));
  LOG_INF("GPU: %s", glGetString(GL_RENDERER));

  double last = glfwGetTime();
  
  LOG_INF("Window initialized - animated clear color to test GPU rendering");

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float time = static_cast<float>(now);

    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);

    // Animated clear color - cycles through rainbow
    float r = 0.5f + 0.5f * sin(time * 0.8f);
    float g = 0.5f + 0.5f * sin(time * 0.8f + 2.0f);
    float b = 0.5f + 0.5f * sin(time * 0.8f + 4.0f);
    
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(win);
    glfwPollEvents();

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(win, true);
  }

  LOG_INF("Shutting down");
  glfwTerminate();
  return 0;
}
