#pragma once

#include "jtk/qbvh.h"
#include "jtk/vec.h"

namespace RenderDoos
  {
  class render_engine;
  }

jtk::float4x4 perspective(float angle, float ratio, float n, float f);

class camera
  {
  public:
    camera(float fov, float aspect, float n, float f);

    jtk::float4x4 get_view_matrix() const;
    jtk::float4x4 get_projection_matrix() const;

    void look_at(const jtk::vec3<float>& target);

    void set_position(float x, float y, float z);
    void set_rotation(float rx, float ry, float rz);
    void set_coordinate_system(const jtk::float4x4& cs);
    void set_up(const jtk::vec3<float>& up);

  private:
    jtk::float4x4 m_projection;
    jtk::float4x4 m_coordinate_system, m_coordinate_system_inv;
    jtk::vec3<float> m_up, m_front;    
  };

struct mesh
  {
  mesh();
  ~mesh();

  void init_from_ply_file(RenderDoos::render_engine& engine, const std::string& filename);
  void cleanup(RenderDoos::render_engine& engine);

  std::vector<jtk::vec3<float>> vertices;
  std::vector<jtk::vec3<float>> normals; 
  std::vector<jtk::vec3<uint32_t>> triangles; 
  std::vector<jtk::vec3<jtk::vec2<float>>> uv;

  int32_t geometry_id;
  };

struct texture
  {
  texture();
  ~texture();

  void init_from_file(RenderDoos::render_engine& engine, const std::string& filename);
  void cleanup(RenderDoos::render_engine& engine);

  int32_t texture_id;
  unsigned char* im;
  };

struct cubemap
  {
  cubemap();
  ~cubemap();

  void init_from_file(RenderDoos::render_engine& engine, 
    const std::string& front,
    const std::string& back,
    const std::string& left,
    const std::string& right,
    const std::string& top,
    const std::string& bottom
    );
  void cleanup(RenderDoos::render_engine& engine);

  int32_t texture_id;
  int32_t geometry_id;
  };