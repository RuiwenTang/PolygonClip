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

Polygon::Polygon(const Polygon &p1, const Polygon &p2) {

  for (auto v : p1.m_sub_polygons) {
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

  for (auto v : p2.m_sub_polygons) {
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

bool Polygon::contains(const Point &p) const {
  bool contains = false;
  int32_t winding_num = 0;

  PolygonIter iter(m_sub_polygons);

  while (iter.has_next()) {
    auto curr = iter.current();
    auto next = curr->next;

    if (((next->point.y > p.y) != (curr->point.y > p.y)) &&
        (p.x < (curr->point.x - next->point.x) * (p.y - next->point.y) /
                       (curr->point.y - next->point.y) +
                   next->point.x)) {
      contains = !contains;

      if (curr->point == next->point ||
          scalar_is_zero(curr->point.x - next->point.x)) {
        iter.move_next();
        continue;
      }

      if (curr->point < next->point) {
        winding_num += 1;
      } else {
        winding_num -= 1;
      }
    }

    iter.move_next();
  }

  return contains;
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

Vertex *Polygon::allocate_vertex(Vertex *p1, Vertex *p2, float t) {
  // point between p1 and p2
  auto p = p1->point * (1.f - t) + p2->point * t;
  return allocate_vertex(p);
}

Polygon Polygon::Clip(const Polygon &subject, const Polygon &clipping) {
  return ClipAlgorithm::do_clip(Polygon(subject), Polygon(clipping));
}

Polygon Polygon::Union(const Polygon &subject, const Polygon &clipping) {
  return ClipAlgorithm::do_union(Polygon(subject), Polygon(clipping));
}

} // namespace pc
