#pragma once
#include <GLFW/glfw3.h>

namespace ui
{

  class ImGuiLayer
  {
  public:
    void init(GLFWwindow *win); // call once after OpenGL is ready
    void begin();               // call every frame BEFORE you render 3-D
    void end();                 // call every frame AFTER you render 3-D
    void shutdown();            // call once on exit
  };

} // namespace ui
