#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "object.hpp"
#include "scene.hpp"

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
in vec2 vUV; uniform sampler2D tex; out vec4 FragColor;
void main(){ FragColor = texture(tex, vUV); }
)";

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "Sun-Earth", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  Shader shader(VSHADER, FSHADER);
  Mesh sphere = Mesh::sphere();

  Object sun{sphere, Texture("assets/sun.jpg")};
  sun.spinSpeed = glm::radians(14.f); // arbitrary

  Object earth{sphere, Texture("assets/earth.jpg")};
  earth.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
  earth.spinSpeed = glm::radians(360.f / 24.f);
  earth.orbitRadius = 4.0f;
  earth.orbitSpeed = glm::radians(360.f / 365.f);

  // Mesh y textures ya existentes
  Object moon{sphere, Texture("assets/moon.jpg")};
  moon.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.073f)); // radio ~ 0.27 Tierra
  moon.spinSpeed = glm::radians(360.f / 27.3f);                // sincrónica ≈ igual a traslación
  moon.orbitRadius = 0.5f;                                     // escala de tu escena
  moon.orbitSpeed = glm::radians(360.f / 27.3f);               // 27.3 d ≈ 1 mes sideral
  moon.orbitTarget = &earth;                                   // ¡clave!

  Scene scene;
  scene.add(&sun);
  scene.add(&earth);
  scene.add(&moon);

  glEnable(GL_DEPTH_TEST);
  double last = glfwGetTime();

  ui::ImGuiLayer gui;
  gui.init(win);

  bool showUI = true;

  while (!glfwWindowShouldClose(win))
  {
    double now = glfwGetTime();
    float dt = static_cast<float>(now - last);
    last = now;
    gui.begin();

    drawOrbitalPanel(sun, earth, &showUI);

    int w, h;
    glfwGetFramebufferSize(win, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 P = glm::perspective(glm::radians(45.f), float(w) / h, 0.1f, 50.f);
    glm::mat4 V = glm::lookAt(glm::vec3(0, 2, 8), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 VP = P * V;

    scene.update(dt, static_cast<float>(now));
    scene.draw(shader, VP);

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
