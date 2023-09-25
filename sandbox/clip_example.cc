
#include "example.hpp"
#include "polygon_clip.hpp"

#include <iostream>

int main(int argc, const char **argv) {

  std::vector<pc::Point> points{
      {100, 50}, {10, 79}, {65, 2}, {65, 98}, {10, 21},
  };

  std::vector<pc::Point> points2{
      {98, 63}, {4, 68}, {77, 8}, {52, 100}, {19, 12},
  };

  pc::Polygon subject;
  subject.append_vertices(points);

  pc::Polygon clipping;
  clipping.append_vertices(points2);

  pc::Polygon result = pc::Polygon::Clip(subject, clipping);

  std::cout << " -------------clip result-------------- " << std::endl;

  for (auto head : result.get_vertices()) {
    auto curr = head;
    while (curr->next && curr->next != head) {
      std::cout << "E {" << curr->point.x << "," << curr->point.y << "} -> {"
                << curr->next->point.x << "," << curr->next->point.y << "}"
                << std::endl;
      curr = curr->next;
    }
  }

  pc::example::App app("clip-example");

  app.init();

  pc::example::PolygonRender p1_render(800, 800, 50.f, 50.f);
  p1_render.init(subject, true);

  pc::example::PolygonRender p2_render(800, 800, 50.f, 50.f);
  p2_render.init(clipping, true);

  pc::example::PolygonRender res_render(800, 800, 50.f, 50.f);
  res_render.init(result, false);

  app.loop([&p1_render, &p2_render, &res_render]() {
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    p1_render.draw({1.f, 0.f, 0.f, 1.f});
    p2_render.draw({0.f, 1.f, 0.f, 1.f});

    res_render.draw({0.f, 0.f, 1.f, 1.f});
  });

  p1_render.terminate();
  p2_render.terminate();
  res_render.terminate();

  app.terminate();

  return 0;
}
