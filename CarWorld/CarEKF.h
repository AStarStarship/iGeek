// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_EKF_H
#define CRABS_CARWORLD_EKF_H
#include "CarPhysics.h"
#include "BicycleModel.h"
#include <math.h>
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
/** 4x4 symmetric matrix (row-major). Used for EKF covariance. */
struct TMat4
{
  FPD m[16] = {};
  FPD& At(ISC r, ISC c) { return m[r * 4 + c]; }
  const FPD& At(ISC r, ISC c) const { return m[r * 4 + c]; }
  FPD AtC(ISC r, ISC c) const { return m[r * 4 + c]; }
};
/** Extended Kalman Filter for car pose estimation (Course 2 w2).
 * State: [x, y, heading, speed]. Process model: kinematic bicycle.
 * Measurements: GPS (x, y), odometry (speed), IMU (yaw_rate).
 * The pilot consumes the EKF estimate, not the ground truth. */
class ACarEKF
{
 public:
  enum { Dim = 4 };
  ACarEKF() { Reset(0, 0, 0, 0); }
  void Reset(FPD x, FPD y, FPD heading, FPD speed) {
    state_[0] = x; state_[1] = y; state_[2] = heading; state_[3] = speed;
    cov_ = {};
    cov_.At(0, 0) = 1.0; cov_.At(1, 1) = 1.0;
    cov_.At(2, 2) = 0.1; cov_.At(3, 3) = 1.0;
    q_pos_ = 0.1; q_vel_ = 0.01; q_head_ = 0.01;
    r_gps_ = 1.0; r_odom_ = 0.01; r_imu_ = 0.01;
    wheelbase_ = 2.7;
  }
  /** Predict: propagate state + covariance through the bicycle model. */
  void Predict(FPD steer, FPD accel, FPD dt) {
    if (dt <= 0 || dt > 0.1) return;
    FPD v = state_[3], psi = state_[2];
    FPD ch = cos(psi), sh = sin(psi);
    FPD k = v / wheelbase_ * tan(TBikeClamp(steer, -1.2, 1.2));
    FPD px = state_[0] + v * ch * dt;
    FPD py = state_[1] + v * sh * dt;
    FPD pp = TBikeAngle(psi + k * dt);
    FPD pv = TBikeMax(0, v + accel * dt);
    // Jacobian F = df/dx.
    TMat4 f = {};
    f.At(0, 0) = 1; f.At(0, 2) = -v * sh * dt; f.At(0, 3) = ch * dt;
    f.At(1, 0) = 1; f.At(1, 2) = v * ch * dt;  f.At(1, 3) = sh * dt;
    f.At(2, 2) = 1; f.At(2, 3) = dt / wheelbase_ * tan(TBikeClamp(steer, -1.2, 1.2));
    f.At(3, 3) = 1;
    // P- = F P F^T + Q.
    TMat4 fp = MatMul(f, cov_);
    TMat4 fpt = MatMul(fp, MatTranspose(f));
    TMat4 q = {};
    q.At(0, 0) = q_pos_ * dt; q.At(1, 1) = q_pos_ * dt;
    q.At(2, 2) = q_head_ * dt; q.At(3, 3) = q_vel_ * dt;
    cov_ = MatAdd(fpt, q);
    state_[0] = px; state_[1] = py; state_[2] = pp; state_[3] = pv;
  }
  /** Update with GPS (x, y) measurement. */
  void UpdateGps(FPD meas_x, FPD meas_y) {
    TMat4 h = {}; h.At(0, 0) = 1; h.At(1, 1) = 1;
    TMat4 r = {}; r.At(0, 0) = r_gps_; r.At(1, 1) = r_gps_;
    FPD z[2] = {meas_x - state_[0], meas_y - state_[1]};
    Update2(h, r, z);
  }
  /** Update with odometry (speed) measurement. */
  void UpdateOdometry(FPD meas_speed) {
    TMat4 h = {}; h.At(0, 3) = 1;
    TMat4 r = {}; r.At(0, 0) = r_odom_;
    FPD z[1] = {meas_speed - state_[3]};
    Update1(h, r, z[0]);
  }
  const FPD* State() const { return state_; }
  FPD X() const { return state_[0]; }
  FPD Y() const { return state_[1]; }
  FPD Heading() const { return state_[2]; }
  FPD Speed() const { return state_[3]; }
  const TMat4& Covariance() const { return cov_; }
  void SetWheelbase(FPD lb) { wheelbase_ = lb > 0 ? lb : 2.7; }
 private:
  FPD state_[Dim];
  TMat4 cov_;
  FPD q_pos_, q_vel_, q_head_;
  FPD r_gps_, r_odom_, r_imu_;
  FPD wheelbase_;
  static TMat4 MatMul(const TMat4& a, const TMat4& b) {
    TMat4 c = {};
    for (ISC i = 0; i < 4; ++i)
      for (ISC j = 0; j < 4; ++j)
        for (ISC k = 0; k < 4; ++k) c.At(i, j) += a.At(i, k) * b.At(k, j);
    return c;
  }
  static TMat4 MatTranspose(const TMat4& a) {
    TMat4 t = {};
    for (ISC i = 0; i < 4; ++i)
      for (ISC j = 0; j < 4; ++j) t.At(i, j) = a.At(j, i);
    return t;
  }
  static TMat4 MatAdd(const TMat4& a, const TMat4& b) {
    TMat4 c = {};
    for (ISC i = 0; i < 16; ++i) c.m[i] = a.m[i] + b.m[i];
    return c;
  }
  void Symmetrize() {
    for (ISC i = 0; i < 4; ++i)
      for (ISC j = i; j < 4; ++j) {
        FPD v = 0.5 * (cov_.At(i, j) + cov_.At(j, i));
        cov_.At(i, j) = v; cov_.At(j, i) = v;
      }
  }
  /** 1-D Kalman update. H is 1x4 (row 0 of h). */
  void Update1(const TMat4& h, const TMat4& r, FPD z) {
    // hp_row[j] = sum_k h(0,k) * P(k,j).
    FPD hp_row[4] = {};
    for (ISC j = 0; j < 4; ++j)
      for (ISC k = 0; k < 4; ++k)
        hp_row[j] += h.At(0, k) * cov_.At(k, j);
    // S = H P H^T + R (scalar).
    FPD s = r.At(0, 0);
    for (ISC j = 0; j < 4; ++j)
      for (ISC k = 0; k < 4; ++k)
        s += h.At(0, j) * cov_.At(j, k) * h.At(0, k);
    if (s < 1e-12) return;
    FPD inv_s = 1.0 / s;
    FPD k_gain[4];
    for (ISC j = 0; j < 4; ++j) k_gain[j] = hp_row[j] * inv_s;
    for (ISC j = 0; j < 4; ++j) state_[j] += k_gain[j] * z;
    // P = (I - K H) P.
    TMat4 i_kh = {};
    for (ISC j = 0; j < 4; ++j) {
      i_kh.At(j, j) = 1.0;
      for (ISC k = 0; k < 4; ++k)
        i_kh.At(j, k) -= k_gain[j] * h.At(0, k);
    }
    cov_ = MatMul(i_kh, cov_);
    Symmetrize();
  }
  /** 2-D Kalman update. H is 2x4 (rows 0,1 of h). */
  void Update2(const TMat4& h, const TMat4& r, const FPD* z) {
    // S = H P H^T + R (2x2).
    FPD s[2][2] = {};
    for (ISC i = 0; i < 2; ++i)
      for (ISC j = 0; j < 2; ++j)
        for (ISC a = 0; a < 4; ++a)
          for (ISC b = 0; b < 4; ++b)
            s[i][j] += h.At(i, a) * cov_.At(a, b) * h.At(j, b);
    s[0][0] += r.At(0, 0); s[1][1] += r.At(1, 1);
    FPD det = s[0][0] * s[1][1] - s[0][1] * s[1][0];
    if (fabs(det) < 1e-12) return;
    FPD inv_det = 1.0 / det;
    FPD s_inv[2][2] = {{s[1][1] * inv_det, -s[0][1] * inv_det},
                       {-s[1][0] * inv_det, s[0][0] * inv_det}};
    // K = P H^T S^-1 (4x2).
    FPD k_gain[4][2] = {};
    for (ISC j = 0; j < 4; ++j)
      for (ISC i = 0; i < 2; ++i)
        for (ISC a = 0; a < 4; ++a)
          for (ISC b = 0; b < 2; ++b)
            k_gain[j][i] += cov_.At(j, a) * h.At(b, a) * s_inv[b][i];
    for (ISC j = 0; j < 4; ++j)
      for (ISC i = 0; i < 2; ++i)
        state_[j] += k_gain[j][i] * z[i];
    // P = (I - K H) P.
    TMat4 i_kh = {};
    for (ISC j = 0; j < 4; ++j) {
      i_kh.At(j, j) = 1.0;
      for (ISC i = 0; i < 2; ++i)
        for (ISC a = 0; a < 4; ++a)
          i_kh.At(j, a) -= k_gain[j][i] * h.At(i, a);
    }
    cov_ = MatMul(i_kh, cov_);
    Symmetrize();
  }
};
} //< namespace iGeek
#endif
#endif
