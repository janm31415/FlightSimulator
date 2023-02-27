#pragma once

#include <stdint.h>

#include "ft2build.h"
#include FT_FREETYPE_H

namespace RenderDoos
  {
  class render_engine;
  }

class material
  {
  public:
    virtual ~material() {}

    virtual void compile(RenderDoos::render_engine* engine) = 0;
    virtual void bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir) = 0;
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
    virtual void bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir);
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

class cubemap_material : public material
  {
  public:
    cubemap_material();
    virtual ~cubemap_material();

    void set_cubemap(int32_t handle, int32_t flags);

    virtual void compile(RenderDoos::render_engine* engine);
    virtual void bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir);
    virtual void destroy(RenderDoos::render_engine* engine);

  private:
    int32_t vs_handle, fs_handle;
    int32_t shader_program_handle;
    int32_t tex_handle;
    int32_t texture_flags;
    int32_t projection_handle, cam_handle, tex0_handle; // uniforms
  };

class terrain_material : public material
  {
  public:
    terrain_material();
    virtual ~terrain_material();

    virtual void compile(RenderDoos::render_engine* engine);
    virtual void bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir);
    virtual void destroy(RenderDoos::render_engine* engine);

    void set_texture_heightmap(int32_t id);
    void set_texture_normalmap(int32_t id);
    void set_texture_colormap(int32_t id);
    void set_texture_noise(int32_t id);
    void set_resolution(uint32_t w, uint32_t h);

    uint32_t get_resolution_width() const { return res_w; }
    uint32_t get_resolution_height() const { return res_h; }

  private:
    int32_t vs_handle, fs_handle;
    int32_t shader_program_handle;
    int32_t proj_handle, res_handle, cam_handle;
    int32_t texture_heightmap, texture_normalmap, texture_colormap, texture_noise;
    int32_t heightmap_handle, normalmap_handle, colormap_handle, noise_handle;
    uint32_t res_w, res_h;
  };

class blit_material : public material
  {
  public:
    blit_material();
    virtual ~blit_material();

    void set_textures(int32_t foreground_texture, int32_t background_texture, int32_t flags);

    virtual void compile(RenderDoos::render_engine* engine);
    virtual void bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir);
    virtual void destroy(RenderDoos::render_engine* engine);

  private:
    int32_t vs_handle, fs_handle;
    int32_t shader_program_handle;
    int32_t fg_tex_handle, bg_tex_handle; 
    int32_t texture_flags;
    int32_t vp_handle, cam_handle, tex0_handle, tex1_handle; // uniforms
  };


typedef struct text_vert_t {
  float x;
  float y;
  float s;
  float t;
  float r;
  float g;
  float b;
  } text_vert_t;

// Structure to hold cache glyph information
typedef struct char_info_t {
  float ax; // advance.x
  float ay; // advance.y

  float bw; // bitmap.width
  float bh; // bitmap.height

  float bl; // bitmap left
  float bt; // bitmap top

  float tx; // x offset of glyph in texture coordinates
  float ty; // y offset of glyph in texture coordinates
  } char_info_t;

class font_material : public material
  {
  public:
    font_material();
    virtual ~font_material();

    virtual void compile(RenderDoos::render_engine* engine);
    virtual void bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir);
    virtual void destroy(RenderDoos::render_engine* engine);

    void prepare_text(RenderDoos::render_engine* engine, const char* text, float x, float y, float sx, float sy, uint32_t clr);
    void render_text(RenderDoos::render_engine* engine);
    
  private:

    void _init_font(RenderDoos::render_engine* engine);

  private:
    int32_t vs_handle, fs_handle;
    int32_t shader_program_handle;
    int32_t width_handle, height_handle;
    int32_t geometry_id;

    int32_t atlas_texture_id;

    int32_t atlas_width;
    int32_t atlas_height;
    char_info_t char_info[128];

    FT_Library _ft;
    FT_Face _face;
  };
