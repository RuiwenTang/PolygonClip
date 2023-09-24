#include "polygon_clip_math.hpp"
#include "polygon_clip.hpp"
#include "polygon_clip_priv.hpp"

namespace pc {

constexpr Scalar kPerturbation = static_cast<Scalar>(1.001);

bool Math::segment_intersect(Vertex *p1, Vertex *p2, Vertex *q1, Vertex *q2,
                             float &t1, float &t2) {

  auto p1_q1 = p1->point - q1->point;
  auto p2_q1 = p2->point - q1->point;
  auto q2_q1 = q2->point - q1->point;

  Point q2_q1_normal{-q2_q1.y, q2_q1.x};

  auto WEC_P1 = p1_q1.x * q2_q1_normal.x + p1_q1.y * q2_q1_normal.y;
  auto WEC_P2 = p2_q1.x * q2_q1_normal.x + p2_q1.y * q2_q1_normal.y;

  if (scalar_is_zero(WEC_P1)) {
    // need perturbation
    p1->point = p1->point * kPerturbation + p2->point * (1 - kPerturbation);

    p1_q1 = p1->point - q1->point;

    WEC_P1 = p1_q1.x * q2_q1_normal.x + p1_q1.y * q2_q1_normal.y;
    WEC_P2 = p2_q1.x * q2_q1_normal.x + p2_q1.y * q2_q1_normal.y;
  }

  if (scalar_is_zero(WEC_P2)) {
    // need perturbation
    p2->point = p2->point * kPerturbation + p1->point * (1 - kPerturbation);

    p2_q1 = p2->point - q1->point;

    WEC_P1 = p1_q1.x * q2_q1_normal.x + p1_q1.y * q2_q1_normal.y;
    WEC_P2 = p2_q1.x * q2_q1_normal.x + p2_q1.y * q2_q1_normal.y;
  }

  if (WEC_P1 * WEC_P2 >= 0.f) {
    return false;
  }

  auto q1_p1 = q1->point - p1->point;
  auto q2_p1 = q2->point - p1->point;
  auto p2_p1 = p2->point - p1->point;
  Point p2_p1_normal{-p2_p1.y, p2_p1.x};

  auto WEC_Q1 = q1_p1.x * p2_p1_normal.x + q1_p1.y * p2_p1_normal.y;
  auto WEC_Q2 = q2_p1.x * p2_p1_normal.x + q2_p1.y * p2_p1_normal.y;

  if (scalar_is_zero(WEC_Q1)) {
    // need perturbation
    q1->point = q1->point * kPerturbation + q2->point * (1 - kPerturbation);

    q1_p1 = q1->point - p1->point;

    WEC_Q1 = q1_p1.x * p2_p1_normal.x + q1_p1.y * p2_p1_normal.y;
    WEC_Q2 = q2_p1.x * p2_p1_normal.x + q2_p1.y * p2_p1_normal.y;
  }

  if (scalar_is_zero(WEC_Q2)) {
    // need perturbation
    q2->point = q2->point * kPerturbation + q1->point * (1 - kPerturbation);

    q2_p1 = q2->point - p1->point;

    WEC_Q1 = q1_p1.x * p2_p1_normal.x + q1_p1.y * p2_p1_normal.y;
    WEC_Q2 = q2_p1.x * p2_p1_normal.x + q2_p1.y * p2_p1_normal.y;
  }

  if (WEC_Q1 * WEC_Q2 >= 0.f) {
    return false;
  }

  t1 = WEC_P1 / (WEC_P1 - WEC_P2);
  t2 = WEC_Q1 / (WEC_Q1 - WEC_Q2);

  return true;
}

} // namespace pc