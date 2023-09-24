#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

namespace pc {

using Scalar = float;

struct Point {
  Scalar x = {};
  Scalar y = {};

  Point() = default;
  Point(Scalar x, Scalar y) : x(x), y(y) {}

  Point(const Point &) = default;
  Point &operator=(const Point &) = default;
};

struct Vertex {
  Point point = {};
  // double linked list in polygon
  Vertex *prev = nullptr;
  Vertex *next = nullptr;
  // state during sweep and mark
  bool intersect = false;
  bool entry_exit = false;
  bool marked = false;
  // pointer to neighbour in subject or clip polygon
  Vertex *neighbour = nullptr;

  Vertex() = default;

  Vertex(Point point) : point(std::move(point)) {}

  Vertex(const Vertex &) = default;
  Vertex &operator=(const Vertex &) = default;
};

class Polygon {
  friend class ClipAlgorithm;

public:
  Polygon() = default;
  ~Polygon() = default;

  // copy is depth clone
  Polygon(const Polygon &other);

  /**
   * Append a closed shape into this polygon
   *
   */
  void append_vertices(const std::vector<Point> &points);

  const std::vector<Vertex *> &get_vertices() const { return m_sub_polygons; }

  /**
   * Doing clip operation on subject, and output the subpolygon inside clipping
   *
   * @subject   polygon need to be clipped
   * @clipping  clip boundary for this clip operator
   */
  static Polygon Clip(const Polygon &subject, const Polygon &clipping);

private:
  Vertex *allocate_vertex(const Point &p);

  Vertex *allocate_vertex(Vertex *p1, Vertex *p2, float t);

private:
  // polygon lists
  // a complex polygon may contains many sub closed polygon
  std::vector<Vertex *> m_sub_polygons = {};
  // just a list to store all allocated vertices
  std::vector<std::unique_ptr<Vertex>> m_vertex = {};

  std::optional<Point> m_left_top = {};
  std::optional<Point> m_right_bottom = {};
};

} // namespace pc
