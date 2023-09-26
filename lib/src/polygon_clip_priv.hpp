#pragma once

#include "polygon_clip.hpp"

#include <tuple>

namespace pc {

Point operator-(const Point &p1, const Point &p2);

Point operator+(const Point &p1, const Point &p2);

Point operator*(const Point &p, Scalar f);

bool operator<(const Point &p1, const Point &p2);

bool operator==(const Point &p1, const Point &p2);

bool scalar_is_zero(float t);

class PolygonIter {
public:
  PolygonIter(const std::vector<Vertex *> &polygons);
  ~PolygonIter() = default;

  bool has_next();

  void move_next();

  Vertex *current();

private:
  size_t m_index;
  Vertex *m_curr_head;
  Vertex *m_current;
  bool m_loop_end = false;
  const std::vector<Vertex *> &m_polygon;
};

class ClipAlgorithm {
  enum class MarkType {
    kIntersection,
    kUnion,
    kDifference,
  };

public:
  /**
   * Calculate the intersect area.
   * The subject and clipping are copyied during claculation to make sure the
   * origin data not changed
   *
   * @subject  copy of the subject polygon
   * @clipping copy of the clipping polygon
   *
   * @return   intersect polygon
   */
  static Polygon do_clip(Polygon subject, Polygon clipping);

  /**
   * Calculate the union area between two polygons
   *
   * @subject copy of the first polygon
   * @clipping copy of the second polygon
   *
   * @return union result
   */
  static Polygon do_union(Polygon subject, Polygon clipping);

private:
  ClipAlgorithm(Polygon subject, Polygon clipping)
      : m_subject(std::move(subject)), m_clipping(std::move(clipping)) {}
  ~ClipAlgorithm() = default;

  void process_intersection();

  /**
   * Mark polygon intersectons
   *
   * {
   *   no_intersection: true if there is no intersection points
   *   inner_indicator: 1 means clipping inside subject, 2 means subject
   *                        inside clipping
   * }
   * @return std::tuple<bool, uint32_t> { no_intersection, inner_indicator }
   */
  std::tuple<bool, uint32_t> mark_vertices();

private:
  Polygon m_subject;
  Polygon m_clipping;

  uint32_t m_intersect_count = 0;
};

} // namespace pc
