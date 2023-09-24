#include "polygon_clip_priv.hpp"
#include "polygon_clip_math.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>

namespace pc {

constexpr float kFloatNearZero = 1.f / (1 << 12);

bool scalar_equal(Scalar s1, Scalar s2) {
  return std::abs(s1 - s2) <= kFloatNearZero;
}

bool scalar_less(Scalar s1, Scalar s2) { return s1 - s2 > -kFloatNearZero; }

bool scalar_is_zero(float t) { return scalar_equal(t, 0.f); }

Point operator-(const Point &p1, const Point &p2) {
  return Point(p1.x - p2.x, p1.y - p2.y);
}

Point operator+(const Point &p1, const Point &p2) {
  return Point(p1.x + p2.x, p1.y + p2.y);
}

Point operator*(const Point &p1, Scalar f) { return Point(p1.x * f, p1.y * f); }

bool operator<(const Point &p1, const Point &p2) {
  return p1.y < p2.y || (scalar_equal(p1.y, p2.y) && scalar_less(p1.x, p2.x));
}

bool operator==(const Point &p1, const Point &p2) {
  return scalar_equal(p1.x, p2.x) && scalar_equal(p1.y, p2.y);
}

PolygonIter::PolygonIter(const std::vector<Vertex *> &polygons)
    : m_polygon(polygons) {
  m_index = 0;
  if (m_polygon.empty()) {
    m_curr_head = nullptr;
    m_current = nullptr;
  } else {
    m_curr_head = m_polygon.front();
    m_current = m_curr_head;
  }
}

bool PolygonIter::has_next() {
  if (m_current == nullptr) {
    return false;
  }

  if (m_current->next == m_curr_head && m_index == m_polygon.size() - 1) {
    return false;
  }

  return true;
}

void PolygonIter::move_next() {
  if (m_current->next != m_curr_head) {
    m_current = m_current->next;
    return;
  }

  if (m_index == m_polygon.size() - 1) {
    return;
  }

  m_index++;
  m_curr_head = m_polygon[m_index];
  m_current = m_curr_head;
}

Vertex *PolygonIter::current() { return m_current; }

Polygon ClipAlgorithm::do_clip(Polygon subject, Polygon clipping) {
  Polygon result;

  ClipAlgorithm algorithm(std::move(subject), std::move(clipping));

  algorithm.process_intersection();

  if (algorithm.m_intersect_count == 0) {
    // no intersection
    // means empty result or subject is inside clipping

    return result;
  }

  algorithm.mark_vertices(MarkType::kIntersection);

  std::vector<Vertex *> intersection_points;
  for (auto &vert : algorithm.m_subject.m_vertex) {
    if (vert->intersect) {
      intersection_points.emplace_back(vert.get());
    }
  }

  for (auto vert : intersection_points) {
    if (vert->marked) {
      continue;
    }

    std::vector<Point> pts;

    vert->marked = true;

    auto start = vert;
    auto current = start;

    pts.emplace_back(current->point);

    do {
      if (current->entry_exit) {
        do {
          current = current->next;

          pts.emplace_back(current->point);
        } while (!current->intersect);
      } else {
        do {
          current = current->prev;

          pts.emplace_back(current->point);
        } while (!current->intersect);
      }
      current->marked = true;
      current = current->neighbour;
      current->marked = true;
    } while (current != start);

    result.append_vertices(pts);
  }

  return result;
}

struct VertexDist {
  Vertex *vert;
  float t;

  VertexDist(Vertex *vert, float t) : vert(vert), t(t) {}
};

struct VertDistCompiler {
  bool operator()(const VertexDist &v1, const VertexDist &v2) {
    return v1.t < v2.t;
  }
};

void ClipAlgorithm::process_intersection() {
  uint32_t intersection_count = 0;

  PolygonIter subj_iter(m_subject.get_vertices());

  while (subj_iter.has_next()) {
    auto current = subj_iter.current();

    subj_iter.move_next();

    PolygonIter clip_iter(m_subject.get_vertices());

    std::vector<VertexDist> intersect_list;

    while (clip_iter.has_next()) {
      float t1 = 0.f;
      float t2 = 0.f;

      auto clip_curr = clip_iter.current();

      if (Math::segment_intersect(current, current->next, clip_curr,
                                  clip_curr->next, t1, t2)) {
        intersection_count++;

        auto i1 = m_subject.allocate_vertex(current, current->next, t1);
        auto i2 = m_clipping.allocate_vertex(clip_curr, clip_curr->next, t2);

        i1->intersect = true;
        i2->intersect = true;

        i1->neighbour = i2;
        i2->neighbour = i1;

        // insert i2 into cliping vertices list
        i2->next = clip_curr->next;
        clip_curr->next->prev = i2;

        clip_curr->next = i2;
        i2->prev = clip_curr;

        // insert i1 into intersect list
        intersect_list.emplace_back(VertexDist(i1, t1));
      }

      clip_iter.move_next();
    }

    if (!intersect_list.empty()) {

      std::sort(intersect_list.begin(), intersect_list.end(),
                VertDistCompiler{});

      for (size_t i = 1; i < intersect_list.size(); i++) {
        auto prev = intersect_list[i - 1];
        auto next = intersect_list[i];

        prev.vert->next = next.vert;
        next.vert->prev = prev.vert;
      }

      auto head = intersect_list.front().vert;
      auto tail = intersect_list.back().vert;

      tail->next = current->next;
      current->next->prev = tail;

      current->next = head;
      head->prev = current;
    }

    subj_iter.move_next();
  }

  assert((intersection_count % 2) == 0);
}

void ClipAlgorithm::mark_vertices(MarkType type) {
  bool no_intersection = true;
  uint32_t inner_indicator = 0;

  // loop for polygon 1

  // false  : exit
  // true   : entry
  bool status = false;

  PolygonIter clip_iter(m_clipping.get_vertices());

  if (m_subject.contains(clip_iter.current()->point)) {
    status = false;
    inner_indicator = 1;
  } else {
    status = true;
    inner_indicator = 0;
  }

  while (clip_iter.has_next()) {
    auto current = clip_iter.current();

    if (current->intersect) {
      current->entry_exit = status;
      status = !status;

      if (no_intersection) {
        no_intersection = false;
      }
    }

    clip_iter.move_next();
  }

  // loop for polygon 2
  PolygonIter subj_iter(m_subject.get_vertices());

  if (m_clipping.contains(subj_iter.current()->point)) {
    status = false;
    inner_indicator = 2;
  } else {
    status = true;
  }

  while (subj_iter.has_next()) {
    auto current = subj_iter.current();

    if (current->intersect) {
      current->entry_exit = status;
      status = !status;

      if (no_intersection) {
        no_intersection = false;
      }
    }
  }

  // polygon 2 is inside polygon 1
  
}

} // namespace pc
