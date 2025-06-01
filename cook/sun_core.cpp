#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

static const char *VSHADER = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
uniform mat4 MVP;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

static const char *FSHADER = R"(
#version 410 core
in vec2 vUV;
uniform sampler2D tex;
out vec4 fragColor;
void main() {
    fragColor = texture(tex, vUV);
}
)";

GLuint compile(GLenum type, const char *src)
{
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok)
  {
    char log[512];
    glGetShaderInfoLog(s, 512, nullptr, log);
    std::cerr << log << '\n';
  }
  return s;
}

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "Rotating Sun", nullptr, nullptr);
  if (!win)
    return -1;
  glfwMakeContextCurrent(win);
  gladLoadGL();

  GLuint vs = compile(GL_VERTEX_SHADER, VSHADER);
  GLuint fs = compile(GL_FRAGMENT_SHADER, FSHADER);
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);

  // ---- generate sphere geometry ----
  const int SEG = 64, RING = 64;
  std::vector<float> verts;
  std::vector<unsigned> idx;
  for (int y = 0; y <= RING; ++y)
  {
    float v = float(y) / RING, phi = v * glm::pi<float>();
    for (int x = 0; x <= SEG; ++x)
    {
      float u = float(x) / SEG, theta = u * 2 * glm::pi<float>();
      float sx = sin(phi) * cos(theta);
      float sy = cos(phi);
      float sz = sin(phi) * sin(theta);
      verts.insert(verts.end(), {sx, sy, sz, u, 1 - v});
    }
  }
  for (int y = 0; y < RING; ++y)
    for (int x = 0; x < SEG; ++x)
    {
      unsigned a = y * (SEG + 1) + x;
      unsigned b = a + SEG + 1;
      idx.insert(idx.end(), {a, b, a + 1, b, b + 1, a + 1});
    }

  GLuint vao, vbo, ebo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // ---- load texture ----
  int w, h, n;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load("sun.jpg", &w, &h, &n, 0);
  if (!data)
  {
    std::cerr << "texture load failed\n";
    return -1;
  }

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, n == 4 ? GL_RGBA : GL_RGB, w, h, 0,
               n == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);

  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(win))
  {
    float t = glfwGetTime();
    int ww, hh;
    glfwGetFramebufferSize(win, &ww, &hh);
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 P = glm::perspective(glm::radians(45.f), float(ww) / hh, 0.1f, 10.f);
    glm::mat4 V = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 M = glm::rotate(glm::mat4(1.0f), t, glm::vec3(0, 1, 0));
    glm::mat4 MVP = P * V * M;

    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "MVP"), 1, GL_FALSE, &MVP[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(prog, "tex"), 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, idx.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
