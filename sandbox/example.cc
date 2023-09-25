#include "example.hpp"
#include <array>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

namespace pc {
namespace example {

const char *kVertexShader = R"(
  #version 330 core
  layout(location = 0) in vec2 aPos;

  uniform mat4 uMVP;
  uniform mat4 uTransform;

  void main() {
    gl_Position = uMVP * uTransform * vec4(aPos, 0.0, 1.0);
  }
)";

const char *kFragmentShader = R"(
  #version 330 core
  uniform vec4 uColor;
  out vec4 FragColor;
  void main() {
    FragColor = uColor;
  }
)";

GLuint create_shader(const char *source, GLenum type) {
  GLuint shader = glCreateShader(type);

  GLint success;
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[1024];
    glGetShaderInfoLog(shader, 1204, nullptr, info_log);
    std::cerr << "shader compile error = " << info_log;
    exit(-4);
  }

  return shader;
}

GLuint create_shader_program(const char *vs_code, const char *fs_code) {
  GLuint program = glCreateProgram();
  GLint success;

  GLuint vs = create_shader(vs_code, GL_VERTEX_SHADER);
  GLuint fs = create_shader(fs_code, GL_FRAGMENT_SHADER);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (!success) {
    GLchar info_log[1024];
    glGetProgramInfoLog(program, 1024, nullptr, info_log);
    std::cerr << "program link error " << info_log << std::endl;
    exit(-5);
  }

  return program;
}

class GlProgram {
public:
  GlProgram() = default;
  ~GlProgram() {
    if (m_program) {
      glDeleteProgram(m_program);
      m_program = 0;
    }
  }

  void init() {
    m_program = create_shader_program(kVertexShader, kFragmentShader);

    m_mvp_location = glGetUniformLocation(m_program, "uMVP");
    m_transform_location = glGetUniformLocation(m_program, "uTransform");
    m_color_location = glGetUniformLocation(m_program, "uColor");
  }

  void bind() { glUseProgram(m_program); }

  void uploadMVP(const std::array<float, 16> &matrix) {
    glUniformMatrix4fv(m_mvp_location, 1, GL_FALSE, matrix.data());
  }

  void uploadTransform(const std::array<float, 16> &matrix) {
    glUniformMatrix4fv(m_transform_location, 1, GL_FALSE, matrix.data());
  }

  void uploadColor(const std::array<float, 4> &color) {
    glUniform4f(m_color_location, color[0], color[1], color[2], color[3]);
  }

  static std::shared_ptr<GlProgram> GetProgram() {
    static std::weak_ptr<GlProgram> g_program;

    auto program = g_program.lock();

    if (!program) {
      program = std::make_shared<GlProgram>();

      program->init();

      g_program = program;
    }

    return program;
  }

private:
  GLuint m_program = 0;
  GLint m_mvp_location = -1;
  GLint m_transform_location = -1;
  GLint m_color_location = -1;
};

enum class Orientation {
  kLinear,
  kClockWise,
  kAntiClockWise,
};

Orientation Calculateorientation(const Point &p, const Point &q,
                                 const Point &r) {
  float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (std::abs(val) <= 0.0001f) {
    return Orientation::kLinear;
  }

  return (val > 0.f) ? Orientation::kClockWise : Orientation::kAntiClockWise;
}

PolygonRender::PolygonRender(uint32_t screen_width, uint32_t screen_height,
                             float translate_x, float translate_y) {
  m_mvp = {
      2.f / screen_width,
      0.f,
      0.f,
      0.f,
      0.f,
      -2.f / screen_height,
      0.f,
      0.f,
      0.f,
      0.f,
      -1.f,
      0.f,
      -1.f,
      1.f,
      0.f,
      1.f,
  };

  m_transform = {
      1.f, 0.f, 0.f, 0.f, 0.f,         1.f,         0.f, 0.f,
      0.f, 0.f, 1.f, 0.f, translate_x, translate_y, 0.f, 1.f,
  };
}

PolygonRender::~PolygonRender() {}

void PolygonRender::init(const pc::Polygon &polygon, bool is_stroke) {
  m_program = GlProgram::GetProgram();

  m_stroke = is_stroke;

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ibo);

  if (m_stroke) {
    init_stroke(polygon);
  } else {
    init_fill(polygon);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void PolygonRender::terminate() {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ibo);
}

void PolygonRender::draw(const std::array<float, 4> &color) {
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

  m_program->bind();

  m_program->uploadMVP(m_mvp);
  m_program->uploadTransform(m_transform);
  m_program->uploadColor(color);

  if (m_stroke) {
    draw_stroke(color);
  } else {
    draw_fill(color);
  }
}

void PolygonRender::draw_stroke(const std::array<float, 4> &color) {
  glDisable(GL_STENCIL_TEST);
  for (const auto &cmd : m_cmds) {
    glDrawElements(GL_LINE_LOOP, cmd.count, GL_UNSIGNED_INT,
                   reinterpret_cast<void *>(cmd.offset));
  }
}

void PolygonRender::draw_fill(const std::array<float, 4> &color) {
  glEnable(GL_STENCIL_TEST);
  glFrontFace(GL_CW);

  for (const auto &cmd : m_cmds) {
    // draw mask
    glColorMask(0, 0, 0, 0);
    glStencilFunc(GL_ALWAYS, 0x01, 0xFF);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    glDrawElements(GL_TRIANGLES, cmd.count, GL_UNSIGNED_INT,
                   reinterpret_cast<void *>(cmd.offset));

    // color
    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glDrawElements(GL_TRIANGLES, cmd.count, GL_UNSIGNED_INT,
                   reinterpret_cast<void *>(cmd.offset));
  }

  glDisable(GL_STENCIL_TEST);
}

void PolygonRender::init_stroke(const pc::Polygon &polygon) {
  std::vector<float> stage_vertices;
  std::vector<uint32_t> stage_index;

  for (auto head : polygon.get_vertices()) {
    auto curr = head;
    uint32_t offset = stage_index.size() * sizeof(uint32_t);
    uint32_t count = 0;
    do {
      uint32_t i = stage_vertices.size() / 2;

      stage_index.emplace_back(i);

      stage_vertices.emplace_back(curr->point.x);
      stage_vertices.emplace_back(curr->point.y);

      count++;

      curr = curr->next;
    } while (curr != head);

    m_cmds.emplace_back(DrawCmd{DrawMode::kLineLoop, offset, count});
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  glBufferData(GL_ARRAY_BUFFER, stage_vertices.size() * sizeof(float),
               stage_vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, stage_index.size() * sizeof(uint32_t),
               stage_index.data(), GL_STATIC_DRAW);
}

void PolygonRender::init_fill(const pc::Polygon &polygon) {
  std::vector<float> stage_vertices;
  std::vector<uint32_t> stage_index;

  for (auto head : polygon.get_vertices()) {
    auto curr = head;

    if (curr->next->next->next == head) {
      uint32_t i = stage_vertices.size() / 2;
      stage_vertices.emplace_back(curr->point.x);
      stage_vertices.emplace_back(curr->point.y);
      uint32_t j = stage_vertices.size() / 2;
      stage_vertices.emplace_back(curr->next->point.x);
      stage_vertices.emplace_back(curr->next->point.x);
      uint32_t k = stage_vertices.size() / 2;
      stage_vertices.emplace_back(curr->next->next->point.x);
      stage_vertices.emplace_back(curr->next->next->point.y);

      uint32_t offset = stage_index.size() * sizeof(uint32_t);

      stage_index.emplace_back(i);
      stage_index.emplace_back(j);
      stage_index.emplace_back(k);

      m_cmds.emplace_back(DrawCmd{DrawMode::kTrialgles, offset, 3});

      continue;
    }

    uint32_t offset = stage_index.size() * sizeof(uint32_t);
    uint32_t count = 0;

    uint32_t i_first = stage_vertices.size() / 2;

    stage_vertices.emplace_back(curr->point.x);
    stage_vertices.emplace_back(curr->point.y);

    uint32_t i_prev = stage_vertices.size() / 2;
    curr = curr->next;

    stage_vertices.emplace_back(curr->point.x);
    stage_vertices.emplace_back(curr->point.y);

    do {

      uint32_t i_curr = stage_vertices.size() / 2;

      stage_vertices.emplace_back(curr->point.x);
      stage_vertices.emplace_back(curr->point.y);

      stage_index.emplace_back(i_first);
      stage_index.emplace_back(i_prev);
      stage_index.emplace_back(i_curr);

      i_prev = i_curr;

      curr = curr->next;
      count += 3;
    } while (curr != head);

    m_cmds.emplace_back(DrawCmd{DrawMode::kTrialgles, offset, count});
  }

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  glBufferData(GL_ARRAY_BUFFER, stage_vertices.size() * sizeof(float),
               stage_vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, stage_index.size() * sizeof(uint32_t),
               stage_index.data(), GL_STATIC_DRAW);
}

App::App(std::string title, uint32_t width, uint32_t height)
    : m_title(std::move(title)), m_width(width), m_height(height),
      m_window(nullptr) {}

void App::init() {
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  m_window =
      glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

  glfwMakeContextCurrent(m_window);
}

void App::loop(std::function<void()> func) {
  while (!glfwWindowShouldClose(m_window)) {
    func();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
  }
}

void App::terminate() {
  glfwDestroyWindow(m_window);

  glfwTerminate();
}

} // namespace example

} // namespace pc
