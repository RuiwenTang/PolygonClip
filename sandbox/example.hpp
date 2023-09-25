#pragma once

#include "polygon_clip.hpp"

#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace pc {
namespace example {

class GlProgram;

class PolygonRender {
public:
  struct DrawCmd {
    uint32_t offset;
    uint32_t count;
  };

  PolygonRender(uint32_t screen_width, uint32_t screen_height,
                float translate_x, float translate_y);
  ~PolygonRender();

  void init(const pc::Polygon &polygon, bool is_stroke);

  void terminate();

  void draw(const std::array<float, 4> &color);

private:
  void init_stroke(const pc::Polygon &polygon);
  void init_fill(const pc::Polygon &polygon);

  void draw_stroke(const std::array<float, 4> &color);
  void draw_fill(const std::array<float, 4> &color);

private:
  GLuint m_vao = {};
  GLuint m_vbo = {};
  GLuint m_ibo = {};
  bool m_stroke = false;
  std::shared_ptr<GlProgram> m_program = {};
  std::vector<DrawCmd> m_cmds = {};
  std::array<float, 16> m_mvp = {};
  std::array<float, 16> m_transform = {};
};

class App {
public:
  App(std::string title, uint32_t width = 800, uint32_t height = 800);
  ~App() = default;

  void init();

  void loop(std::function<void()> func);

  void terminate();

private:
  uint32_t m_width;
  uint32_t m_height;
  std::string m_title;
  GLFWwindow *m_window = nullptr;
};

} // namespace example
} // namespace pc
