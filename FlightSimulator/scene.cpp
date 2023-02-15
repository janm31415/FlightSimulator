#include "scene.h"

jtk::float4x4 perspective(float angle, float ratio, float n, float f)
  {
  jtk::float4x4 ret;
  float tan_half_angle = std::tan(angle / 2);
  ret[0] = 1 / (ratio * tan_half_angle);
  ret[1] = 0;
  ret[2] = 0;
  ret[3] = 0;
  ret[4] = 0;
  ret[5] = 1 / tan_half_angle;
  ret[6] = 0;
  ret[7] = 0;
  ret[8] = 0;
  ret[9] = 0;
  ret[10] = -(f + n) / (f - n);
  ret[11] = +1;
  ret[12] = 0;
  ret[13] = 0;
  ret[14] = -(2 * f * n) / (f - n);
  ret[15] = 0;
  return ret;
  }

camera::camera(float fov, float aspect, float n, float f) :
  m_up(0, 1, 0), m_front(0, 0, 1)
  {
  m_projection = perspective(fov, aspect, n, f);
  m_coordinate_system_inv = jtk::get_identity();
  m_coordinate_system = jtk::get_identity();
  }

jtk::float4x4 camera::get_view_matrix() const
  {
  return m_coordinate_system_inv;
  }

jtk::float4x4 camera::get_projection_matrix() const
  {
  return m_projection;
  }

void camera::look_at(const jtk::vec3<float>& target)
  {
  jtk::vec3<float> pos(m_coordinate_system[12], m_coordinate_system[13], m_coordinate_system[14]);
  m_coordinate_system_inv = jtk::look_at(pos, target, m_up);
  m_coordinate_system = jtk::invert_orthonormal(m_coordinate_system_inv);
  }