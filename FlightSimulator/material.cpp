#include "material.h"
#include <string>

#include "gl_shaders.h"

#include "RenderDoos/render_engine.h"
#include "RenderDoos/types.h"


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
  vp_handle = engine->add_uniform("Projection", uniform_type::mat4, 1);
  cam_handle = engine->add_uniform("Camera", uniform_type::mat4, 1);
  light_dir_handle = engine->add_uniform("LightDir", uniform_type::vec3, 1);
  tex_sample_handle = engine->add_uniform("TextureSample", uniform_type::integer, 1);
  ambient_handle = engine->add_uniform("Ambient", uniform_type::real, 1);
  color_handle = engine->add_uniform("Color", uniform_type::vec4, 1);
  tex0_handle = engine->add_uniform("Tex0", uniform_type::sampler, 1);
  }

void simple_material::bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir)
  {
  using namespace RenderDoos;

  engine->bind_program(shader_program_handle);

  engine->set_uniform(vp_handle, (void*)projection);
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

cubemap_material::cubemap_material()
  {
  vs_handle = -1;
  fs_handle = -1;
  shader_program_handle = -1;
  tex_handle = -1;
  projection_handle = -1;
  cam_handle = -1;
  tex0_handle = -1;
  }

cubemap_material::~cubemap_material()
  {
  }

void cubemap_material::set_cubemap(int32_t handle, int32_t flags)
  {
  if (handle >= 0 && handle < MAX_TEXTURE)
    tex_handle = handle;
  else
    tex_handle = -1;
  texture_flags = flags;
  }

void cubemap_material::destroy(RenderDoos::render_engine* engine)
  {
  engine->remove_program(shader_program_handle);
  engine->remove_shader(vs_handle);
  engine->remove_shader(fs_handle);
  engine->remove_texture(tex_handle);
  engine->remove_uniform(projection_handle);
  engine->remove_uniform(cam_handle);
  engine->remove_uniform(tex0_handle);
  }

void cubemap_material::compile(RenderDoos::render_engine* engine)
  {
  using namespace RenderDoos;
  if (engine->get_renderer_type() == renderer_type::METAL)
    {
    vs_handle = engine->add_shader(nullptr, SHADER_VERTEX, "cubemap_material_vertex_shader");
    fs_handle = engine->add_shader(nullptr, SHADER_FRAGMENT, "cubemap_material_fragment_shader");
    }
  else if (engine->get_renderer_type() == renderer_type::OPENGL)
    {
    vs_handle = engine->add_shader(get_cubemap_material_vertex_shader().c_str(), SHADER_VERTEX, nullptr);
    fs_handle = engine->add_shader(get_cubemap_material_fragment_shader().c_str(), SHADER_FRAGMENT, nullptr);
    }
  shader_program_handle = engine->add_program(vs_handle, fs_handle);
  projection_handle = engine->add_uniform("Projection", uniform_type::mat4, 1);
  cam_handle = engine->add_uniform("Camera", uniform_type::mat4, 1);
  tex0_handle = engine->add_uniform("environmentMap", uniform_type::sampler, 1);
  }

void cubemap_material::bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* /*light_dir*/)
  {
  engine->bind_program(shader_program_handle);

  engine->set_uniform(projection_handle, (void*)projection);
  engine->set_uniform(cam_handle, (void*)camera_space);
  int32_t tex_0 = 0;
  engine->set_uniform(tex0_handle, (void*)&tex_0);

  engine->bind_uniform(shader_program_handle, projection_handle);
  engine->bind_uniform(shader_program_handle, cam_handle);
  engine->bind_uniform(shader_program_handle, tex0_handle);
  engine->bind_texture_to_channel(tex_handle, 0, texture_flags);
  }

terrain_material::terrain_material()
  {
  vs_handle = -1;
  fs_handle = -1;
  shader_program_handle = -1;
  proj_handle = -1;
  cam_handle = -1;
  res_handle = -1;
  texture_heightmap = -1;
  texture_normalmap = -1;
  texture_colormap = -1;
  heightmap_handle = -1;
  normalmap_handle = -1;
  colormap_handle = -1;
  res_w = 800;
  res_h = 450;
  }

terrain_material::~terrain_material()
  {
  }

void terrain_material::destroy(RenderDoos::render_engine* engine)
  {
  engine->remove_shader(vs_handle);
  engine->remove_shader(fs_handle);
  engine->remove_program(shader_program_handle);
  engine->remove_uniform(proj_handle);
  engine->remove_uniform(cam_handle);
  engine->remove_uniform(res_handle);
  engine->remove_uniform(heightmap_handle);
  engine->remove_uniform(normalmap_handle);
  engine->remove_uniform(colormap_handle);
  }

void terrain_material::set_texture_heightmap(int32_t id)
  {
  texture_heightmap = id;
  }

void terrain_material::set_texture_normalmap(int32_t id)
  {
  texture_normalmap = id;
  }

void terrain_material::set_texture_colormap(int32_t id)
  {
  texture_colormap = id;
  }

void terrain_material::set_resolution(uint32_t w, uint32_t h)
  {
  res_w = w;
  res_h = h;
  }

void terrain_material::compile(RenderDoos::render_engine* engine)
  {
  if (engine->get_renderer_type() == RenderDoos::renderer_type::METAL)
    {
    vs_handle = engine->add_shader(nullptr, SHADER_VERTEX, "terrain_material_vertex_shader");
    fs_handle = engine->add_shader(nullptr, SHADER_FRAGMENT, "terrain_material_fragment_shader");
    }
  else if (engine->get_renderer_type() == RenderDoos::renderer_type::OPENGL)
    {
    vs_handle = engine->add_shader(get_terrain_material_vertex_shader().c_str(), SHADER_VERTEX, nullptr);
    fs_handle = engine->add_shader(get_terrain_material_fragment_shader().c_str(), SHADER_FRAGMENT, nullptr);
    }
  shader_program_handle = engine->add_program(vs_handle, fs_handle);
  proj_handle = engine->add_uniform("Projection", RenderDoos::uniform_type::mat4, 1);
  cam_handle = engine->add_uniform("Camera", RenderDoos::uniform_type::mat4, 1);
  res_handle = engine->add_uniform("iResolution", RenderDoos::uniform_type::vec3, 1);
  heightmap_handle = engine->add_uniform("Heightmap", RenderDoos::uniform_type::sampler, 1);
  normalmap_handle = engine->add_uniform("Normalmap", RenderDoos::uniform_type::sampler, 1);
  colormap_handle = engine->add_uniform("Colormap", RenderDoos::uniform_type::sampler, 1);
  }

void terrain_material::bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* /*light_dir*/)
  {
  engine->bind_program(shader_program_handle);
  engine->set_uniform(proj_handle, (void*)projection);
  engine->set_uniform(cam_handle, (void*)camera_space);
  float res[3] = { (float)res_w, (float)res_h, 1.f };
  engine->set_uniform(res_handle, (void*)res);
  int32_t tex = 0;
  engine->set_uniform(heightmap_handle, (void*)&tex);
  tex = 1;
  engine->set_uniform(normalmap_handle, (void*)&tex);
  tex = 2;
  engine->set_uniform(colormap_handle, (void*)&tex);

  engine->bind_texture_to_channel(texture_heightmap, 0, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);
  engine->bind_texture_to_channel(texture_normalmap, 1, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);
  engine->bind_texture_to_channel(texture_colormap, 2, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);

  engine->bind_uniform(shader_program_handle, proj_handle);
  engine->bind_uniform(shader_program_handle, cam_handle);
  engine->bind_uniform(shader_program_handle, res_handle);
  engine->bind_uniform(shader_program_handle, heightmap_handle);
  engine->bind_uniform(shader_program_handle, normalmap_handle);
  engine->bind_uniform(shader_program_handle, colormap_handle);
  }


blit_material::blit_material()
  {
  vs_handle = -1;
  fs_handle = -1;
  shader_program_handle = -1;
  fg_tex_handle = -1;
  bg_tex_handle = -1;
  vp_handle = -1;
  cam_handle = -1;
  tex0_handle = -1;
  tex1_handle = -1;
  }

blit_material::~blit_material()
  {
  }

void blit_material::set_textures(int32_t foreground_texture, int32_t background_texture, int32_t flags)
  {
  if (foreground_texture >= 0 && foreground_texture < MAX_TEXTURE)
    fg_tex_handle = foreground_texture;
  else
    fg_tex_handle = -1;
  if (background_texture >= 0 && background_texture < MAX_TEXTURE)
    bg_tex_handle = background_texture;
  else
    bg_tex_handle = -1;
  texture_flags = flags;
  }

void blit_material::destroy(RenderDoos::render_engine* engine)
  {
  engine->remove_program(shader_program_handle);
  engine->remove_shader(vs_handle);
  engine->remove_shader(fs_handle);
  engine->remove_texture(fg_tex_handle);
  engine->remove_texture(bg_tex_handle);
  engine->remove_uniform(vp_handle);
  engine->remove_uniform(cam_handle);
  engine->remove_uniform(tex0_handle);
  }

void blit_material::compile(RenderDoos::render_engine* engine)
  {
  using namespace RenderDoos;
  if (engine->get_renderer_type() == renderer_type::METAL)
    {
    vs_handle = engine->add_shader(nullptr, SHADER_VERTEX, "blit_material_vertex_shader");
    fs_handle = engine->add_shader(nullptr, SHADER_FRAGMENT, "blit_material_fragment_shader");
    }
  else if (engine->get_renderer_type() == renderer_type::OPENGL)
    {
    vs_handle = engine->add_shader(get_blit_material_vertex_shader().c_str(), SHADER_VERTEX, nullptr);
    fs_handle = engine->add_shader(get_blit_material_fragment_shader().c_str(), SHADER_FRAGMENT, nullptr);
    }
  shader_program_handle = engine->add_program(vs_handle, fs_handle);
  vp_handle = engine->add_uniform("Projection", uniform_type::mat4, 1);
  cam_handle = engine->add_uniform("Camera", uniform_type::mat4, 1);
  tex0_handle = engine->add_uniform("Tex0", uniform_type::sampler, 1);
  tex1_handle = engine->add_uniform("Tex1", uniform_type::sampler, 1);
  }

void blit_material::bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* /*light_dir*/)
  {
  using namespace RenderDoos;

  engine->bind_program(shader_program_handle);

  engine->set_uniform(vp_handle, (void*)projection);
  engine->set_uniform(cam_handle, (void*)camera_space);  
  int32_t tex = 0;
  engine->set_uniform(tex0_handle, (void*)&tex);
  tex = 1;
  engine->set_uniform(tex1_handle, (void*)&tex);
  engine->bind_uniform(shader_program_handle, vp_handle);
  engine->bind_uniform(shader_program_handle, cam_handle);
  engine->bind_uniform(shader_program_handle, tex0_handle);
  engine->bind_uniform(shader_program_handle, tex1_handle);
  engine->bind_texture_to_channel(fg_tex_handle, 0, texture_flags);
  engine->bind_texture_to_channel(bg_tex_handle, 1, texture_flags);
  }