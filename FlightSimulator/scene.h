#pragma once

#include "jtk/qbvh.h"
#include "jtk/vec.h"

class camera
  {
  public:
    camera(float fov, float aspect, float n, float f);

    jtk::float4x4 get_view_matrix() const;
    jtk::float4x4 get_projection_matrix() const;

    void look_at(const jtk::vec3<float>& target);

  private:
    jtk::float4x4 m_projection;
    jtk::float4x4 m_coordinate_system, m_coordinate_system_inv;
    jtk::vec3<float> m_up, m_front;    
  };