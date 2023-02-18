#include "material.h"
#include <string>

#include "RenderDoos/render_engine.h"
#include "RenderDoos/types.h"

///////////////////////////////////////////////////////////////////////

static std::string get_simple_material_vertex_shader()
  {
  return std::string(R"(#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
uniform mat4 ViewProject; // columns
uniform mat4 Camera; // columns

out vec3 Normal;
out vec2 TexCoord;

void main() 
  {
  gl_Position = ViewProject*vec4(vPosition.xyz,1);
  Normal = (Camera*vec4(vNormal,0)).xyz;
  TexCoord = vTexCoord;
  }
)");
  }

static std::string get_simple_material_fragment_shader()
  {
  return std::string(R"(#version 330 core
out vec4 FragColor;
  
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D Tex0;
uniform vec3 LightDir;
uniform vec4 Color;
uniform int TextureSample;
uniform float Ambient;

void main()
  {
  float l = clamp(dot(Normal,LightDir), 0, 1.0 - Ambient) + Ambient;
  vec4 clr = (texture(Tex0, TexCoord)*TextureSample + Color*(1-TextureSample))*l;
  FragColor = clr;
  }
)");
  }

simple_material::simple_material()
  {
  vs_handle = -1;
  fs_handle = -1;
  shader_program_handle = -1;
  tex_handle = -1;
  color = 0xff0000ff;
  ambient = 0.2f;
  vp_handle = -1;
  cam_handle = -1;
  light_dir_handle = -1;
  tex_sample_handle = -1;
  ambient_handle = -1;
  color_handle = -1;
  tex0_handle = -1;
  dummy_tex_handle = -1;
  }

simple_material::~simple_material()
  {
  }

void simple_material::set_texture(int32_t handle, int32_t flags)
  {
  if (handle >= 0 && handle < MAX_TEXTURE)
    tex_handle = handle;
  else
    tex_handle = -1;
  texture_flags = flags;
  }

void simple_material::set_color(uint32_t clr)
  {
  color = clr;
  }

void simple_material::set_ambient(float a)
  {
  ambient = a;
  }

void simple_material::destroy(RenderDoos::render_engine* engine)
  {
  engine->remove_program(shader_program_handle);
  engine->remove_shader(vs_handle);
  engine->remove_shader(fs_handle);
  engine->remove_texture(tex_handle);
  engine->remove_texture(dummy_tex_handle);
  engine->remove_uniform(vp_handle);
  engine->remove_uniform(cam_handle);
  engine->remove_uniform(light_dir_handle);
  engine->remove_uniform(tex_sample_handle);
  engine->remove_uniform(ambient_handle);
  engine->remove_uniform(color_handle);
  engine->remove_uniform(tex0_handle);
  }

void simple_material::compile(RenderDoos::render_engine* engine)
  {
  using namespace RenderDoos;
  if (engine->get_renderer_type() == renderer_type::METAL)
    {
    vs_handle = engine->add_shader(nullptr, SHADER_VERTEX, "simple_material_vertex_shader");
    fs_handle = engine->add_shader(nullptr, SHADER_FRAGMENT, "simple_material_fragment_shader");
    }
  else if (engine->get_renderer_type() == renderer_type::OPENGL)
    {
    vs_handle = engine->add_shader(get_simple_material_vertex_shader().c_str(), SHADER_VERTEX, nullptr);
    fs_handle = engine->add_shader(get_simple_material_fragment_shader().c_str(), SHADER_FRAGMENT, nullptr);
    }
  dummy_tex_handle = engine->add_texture(1, 1, texture_format_rgba8, (const uint16_t*)nullptr);
  shader_program_handle = engine->add_program(vs_handle, fs_handle);
  vp_handle = engine->add_uniform("ViewProject", uniform_type::mat4, 1);
  cam_handle = engine->add_uniform("Camera", uniform_type::mat4, 1);
  light_dir_handle = engine->add_uniform("LightDir", uniform_type::vec3, 1);
  tex_sample_handle = engine->add_uniform("TextureSample", uniform_type::integer, 1);
  ambient_handle = engine->add_uniform("Ambient", uniform_type::real, 1);
  color_handle = engine->add_uniform("Color", uniform_type::vec4, 1);
  tex0_handle = engine->add_uniform("Tex0", uniform_type::sampler, 1);
  }

void simple_material::bind(RenderDoos::render_engine* engine, float* view_project, float* camera_space, float* light_dir)
  {
  using namespace RenderDoos;

  engine->bind_program(shader_program_handle);

  engine->set_uniform(vp_handle, (void*)view_project);
  engine->set_uniform(cam_handle, (void*)camera_space);
  engine->set_uniform(light_dir_handle, (void*)light_dir);
  int32_t tex_sample = tex_handle >= 0 ? 1 : 0;
  engine->set_uniform(tex_sample_handle, (void*)&tex_sample);
  engine->set_uniform(ambient_handle, (void*)&ambient);
  float col[4] = { (color & 255) / 255.f, ((color >> 8) & 255) / 255.f, ((color >> 16) & 255) / 255.f, ((color >> 24) & 255) / 255.f };
  engine->set_uniform(color_handle, (void*)col);
  int32_t tex_0 = 0;
  engine->set_uniform(tex0_handle, (void*)&tex_0);

  engine->bind_uniform(shader_program_handle, vp_handle);
  engine->bind_uniform(shader_program_handle, cam_handle);
  engine->bind_uniform(shader_program_handle, color_handle);
  engine->bind_uniform(shader_program_handle, light_dir_handle);
  engine->bind_uniform(shader_program_handle, tex_sample_handle);
  engine->bind_uniform(shader_program_handle, ambient_handle);
  engine->bind_uniform(shader_program_handle, tex0_handle);
  if (tex_handle >= 0)
    {
    const texture* tex = engine->get_texture(tex_handle);
    if (tex->format == texture_format_bgra8 || tex->format == texture_format_rgba16 || tex->format == texture_format_rgba8 || tex->format == texture_format_rgba32f)
      engine->bind_texture_to_channel(tex_handle, 0, texture_flags);
    else
      engine->bind_texture_to_channel(dummy_tex_handle, 0, texture_flags);
    }
  else
    {
    engine->bind_texture_to_channel(dummy_tex_handle, 0, texture_flags);
    }
  }