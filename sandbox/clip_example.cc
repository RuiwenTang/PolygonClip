
#include "polygon_clip.hpp"

#include <iostream>

int main(int argc, const char **argv) {

  std::vector<pc::Point> points{{0, 0}, {100, 0}, {100, 100}, {0, 100}};

  std::vector<pc::Point> points2{{50, 50}, {150, 50}, {150, 150}, {50, 150}};

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

  return 0;
}
