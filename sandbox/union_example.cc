
#include "example.hpp"
#include "polygon_clip.hpp"

int main(int argc, const char **argv) {

  pc::example::App app("union-example");

  app.init();

  std::vector<pc::Point> points1{
      {100.f, 50.f},
      {50.f, 100.f},
      {100.f, 150.f},
      {150.f, 100.f},
  };

  std::vector<pc::Point> points2{
      {150.f, 50.f},
      {100.f, 100.f},
      {150.f, 150.f},
      {200.f, 100.f},
  };

  pc::Polygon p1;
  p1.append_vertices(points1);

  pc::Polygon p2;
  p2.append_vertices(points2);

  auto union_res = pc::Polygon::Union(p1, p2);

  pc::example::PolygonRender p1_render(800, 800, 0.f, 0.f);
  pc::example::PolygonRender p2_render(800, 800, 0.f, 0.f);
  pc::example::PolygonRender union_render(800, 800, 0.f, 200.f);

  p1_render.init(p1, true);
  p2_render.init(p2, true);
  union_render.init(union_res, true);

  app.loop([&p1_render, &p2_render, &union_render]() {
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClearStencil(0);

    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    p1_render.draw({1.f, 0.f, 0.f, 1.f});
    p2_render.draw({0.f, 1.f, 0.f, 1.f});

    union_render.draw({1.f, 0.f, 1.f, 1.f});
  });

  p1_render.terminate();
  p2_render.terminate();
  union_render.terminate();

  app.terminate();

  return 0;
}
