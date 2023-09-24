#pragma once

#include "polygon_clip.hpp"

namespace pc {

class Math {
public:
  static bool segment_intersect(Vertex *p1, Vertex *p2, Vertex *q1, Vertex *q2,
                                float &t1, float &t2);
};

} // namespace pc
