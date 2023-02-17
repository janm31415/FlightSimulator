#pragma once

#include <stdint.h>

namespace RenderDoos
  {
  class render_engine;
  }

class material
  {
  public:
    virtual ~material() {}

    virtual void compile(RenderDoos::render_engine* engine) = 0;
    virtual void bind(RenderDoos::render_engine* engine, float* view_project, float* camera_space, float* light_dir) = 0;
    virtual void destroy(RenderDoos::render_engine* engine) = 0;
  };

class simple_material : public material
  {
  public:
    simple_material();
    virtual ~simple_material();

    void set_texture(int32_t handle, int32_t flags);
    void set_color(uint32_t clr);
    void set_ambient(float a);

    virtual void compile(RenderDoos::render_engine* engine);
    virtual void bind(RenderDoos::render_engine* engine, float* view_project, float* camera_space, float* light_dir);
    virtual void destroy(RenderDoos::render_engine* engine);

  private:
    int32_t vs_handle, fs_handle;
    int32_t shader_program_handle;
    int32_t tex_handle, dummy_tex_handle;
    uint32_t color; // if no texture is set
    float ambient;
    int32_t texture_flags;
    int32_t vp_handle, cam_handle, light_dir_handle, tex_sample_handle, ambient_handle, color_handle, tex0_handle; // uniforms
  };