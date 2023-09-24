#include "polygon_clip.hpp"
#include "polygon_clip_priv.hpp"

namespace pc {

Polygon::Polygon(const Polygon &other) {

  for (auto v : other.m_sub_polygons) {
    auto p = v;
    std::vector<Point> points{};
    points.emplace_back(p->point);

    p = p->next;

    while (p != v) {
      points.emplace_back(p->point);
      p = p->next;
    }

    if (points.size() < 3) {
      continue;
    }

    this->append_vertices(points);
  }
}

void Polygon::append_vertices(const std::vector<Point> &points) {
  if (points.size() < 3) {
    // not a closed path
    return;
  }

  auto head = allocate_vertex(points.front());

  auto prev = head;
  for (size_t i = 1; i < points.size(); i++) {
    auto next = allocate_vertex(points[i]);
    prev->next = next;
    next->prev = prev;

    prev = next;
  }

  prev->next = head;
  head->prev = prev;

  m_sub_polygons.emplace_back(head);
}

Vertex *Polygon::allocate_vertex(const Point &p) {
  if (!m_left_top) {
    m_left_top = p;
  } else {
    if (m_left_top->x >= p.x)
      m_left_top->x = p.x;

    if (m_left_top->y >= p.y)
      m_left_top->y = p.y;
  }

  if (!m_right_bottom) {
    m_right_bottom = p;
  } else {
    if (m_right_bottom->x < p.x)
      m_right_bottom->x = p.x;

    if (m_right_bottom->y < p.y)
      m_right_bottom->y = p.y;
  }

  m_vertex.emplace_back(std::make_unique<Vertex>(p));

  return m_vertex.back().get();
}

Polygon Polygon::Clip(const Polygon &subject, const Polygon &clipping) {
  return ClipAlgorithm::do_clip(Polygon(subject), Polygon(clipping));
}

} // namespace pc
