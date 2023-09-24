
#include "polygon_clip.hpp"

int main(int argc, const char **argv) {

  std::vector<pc::Point> points{{0, 0}, {100, 0}, {100, 100}, {0, 100}};

  std::vector<pc::Point> points2{{50, 50}, {150, 50}, {150, 150}, {50, 150}};

  pc::Polygon subject;
  subject.append_vertices(points);

  pc::Polygon clipping;
  clipping.append_vertices(points2);

  pc::Polygon result = pc::Polygon::Clip(subject, clipping);

  return 0;
}
