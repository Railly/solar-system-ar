#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "logger.hpp"

#include <iostream>

int main()
{
  LOG_INF("Starting AR Solar System - Step 0: Bootstrap & Build");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Para macOS

  GLFWwindow *win = glfwCreateWindow(800, 600, "AR Solar System - Bootstrap", nullptr, nullptr);
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

  glfwSwapInterval(1);  // V-Sync
  glClearColor(0.12f, 0.12f, 0.15f, 1.0f);  // Fondo gris oscuro

  LOG_INF("OpenGL Context: %s", glGetString(GL_VERSION));
  LOG_INF("GPU: %s", glGetString(GL_RENDERER));
  LOG_INF("Window created - basic gray background, press ESC to exit");

  while (!glfwWindowShouldClose(win))
  {
    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);

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
