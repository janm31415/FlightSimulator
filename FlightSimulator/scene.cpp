#include "scene.h"
#include "jtk/ply.h"

#include "RenderDoos/render_engine.h"
#include "RenderDoos/types.h"

#include "stb/stb_image.h"

#include "physics.h"

jtk::float4x4 perspective(float angle, float ratio, float n, float f)
  {
  jtk::float4x4 ret;
  const float tan_half_angle = std::tan(angle / 2);
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
  ret[11] = -1;
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

void camera::set_position(float x, float y, float z)
  {
  m_coordinate_system[12] = x;
  m_coordinate_system[13] = y;
  m_coordinate_system[14] = z;
  m_coordinate_system_inv = jtk::invert_orthonormal(m_coordinate_system);
  }

void camera::set_rotation(float rx, float ry, float rz)
  {  
  jtk::float4x4 rotx = jtk::make_rotation(physics::ORIGIN, physics::X_AXIS, rx);
  jtk::float4x4 roty = jtk::make_rotation(physics::ORIGIN, physics::Y_AXIS, ry);
  jtk::float4x4 rotz = jtk::make_rotation(physics::ORIGIN, physics::Z_AXIS, rz);
  jtk::float4x4 rot = jtk::matrix_matrix_multiply(jtk::matrix_matrix_multiply(rotz, roty), rotx);
  m_coordinate_system[0] = rot[0];
  m_coordinate_system[1] = rot[1];
  m_coordinate_system[2] = rot[2];
  m_coordinate_system[4] = rot[4];
  m_coordinate_system[5] = rot[5];
  m_coordinate_system[6] = rot[6];
  m_coordinate_system[8] = rot[8];
  m_coordinate_system[9] = rot[9];
  m_coordinate_system[10] = rot[10];
  m_coordinate_system_inv = jtk::invert_orthonormal(m_coordinate_system);
  }

mesh::mesh() : geometry_id(-1)
  {
  }

mesh::~mesh()
  {
  }

void mesh::cleanup(RenderDoos::render_engine& engine)
  {
  if (geometry_id >= 0)
    {
    engine.remove_geometry(geometry_id);
    }
  }

void mesh::init_from_ply_file(RenderDoos::render_engine& engine, const std::string& filename)
  {
  std::vector<uint32_t> clrs;
  if (read_ply(filename.c_str(), vertices, normals, clrs, triangles, uv))
    {
    std::vector<jtk::vec2<float>> uv_per_vertex(vertices.size(), jtk::vec2<float>(-1));
    for (uint32_t i = 0; i < triangles.size(); ++i)
      {
      for (uint32_t j = 0; j < 3; ++j)
        {
        uv_per_vertex[triangles[i][j]] = uv[i][j];
        }
      }
    geometry_id = engine.add_geometry(VERTEX_STANDARD);
    RenderDoos::vertex_standard* vp;
    uint32_t* ip;
    engine.geometry_begin(geometry_id, vertices.size(), triangles.size()*3, (float**)&vp, (void**)&ip);
    for (uint32_t i = 0; i < vertices.size(); ++i)
      {
      vp->x = vertices[i].x;
      vp->y = vertices[i].y;
      vp->z = vertices[i].z;
      vp->nx = normals[i].x;
      vp->ny = normals[i].y;
      vp->nz = normals[i].z;
      vp->u = uv_per_vertex[i].x;
      vp->v = uv_per_vertex[i].y;
      ++vp;
      }
    for (uint32_t i = 0; i < triangles.size(); ++i)
      {
      *ip++ = triangles[i][0];
      *ip++ = triangles[i][1];
      *ip++ = triangles[i][2];
      }
    engine.geometry_end(geometry_id);
    }
  }

texture::texture() : texture_id(-1), im(nullptr)
  {
  }

texture::~texture()
  {
  if (im)
    stbi_image_free(im);
  }

void texture::init_from_file(RenderDoos::render_engine& engine, const std::string& filename)
  {
  int w, h, nr_of_channels;
  im = stbi_load(filename.c_str(), &w, &h, &nr_of_channels, 4);
  texture_id = engine.add_texture(w, h, RenderDoos::texture_format_rgba8, im);
  }

void texture::cleanup(RenderDoos::render_engine& engine)
  {
  engine.remove_texture(texture_id);
  }