#include "material.h"
#include <string>

#include "gl_shaders.h"

#include "RenderDoos/render_engine.h"
#include "RenderDoos/types.h"

#include <vector>

#define MAX_WIDTH 2048 // Maximum texture width on pi

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
  texture_noise = -1;
  heightmap_handle = -1;
  normalmap_handle = -1;
  colormap_handle = -1;
  noise_handle = -1;
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
  engine->remove_uniform(noise_handle);
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

void terrain_material::set_texture_noise(int32_t id)
  {
  texture_noise = id;
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
  noise_handle = engine->add_uniform("Noise", RenderDoos::uniform_type::sampler, 1);
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
  tex = 3;
  engine->set_uniform(noise_handle, (void*)&tex);

  engine->bind_texture_to_channel(texture_heightmap, 0, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);
  engine->bind_texture_to_channel(texture_normalmap, 1, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);
  engine->bind_texture_to_channel(texture_colormap, 2, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);
  engine->bind_texture_to_channel(texture_noise, 3, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);

  engine->bind_uniform(shader_program_handle, proj_handle);
  engine->bind_uniform(shader_program_handle, cam_handle);
  engine->bind_uniform(shader_program_handle, res_handle);
  engine->bind_uniform(shader_program_handle, heightmap_handle);
  engine->bind_uniform(shader_program_handle, normalmap_handle);
  engine->bind_uniform(shader_program_handle, colormap_handle);
  engine->bind_uniform(shader_program_handle, noise_handle);
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


font_material::font_material()
  {
  vs_handle = -1;
  fs_handle = -1;
  shader_program_handle = -1;
  width_handle = -1;
  height_handle = -1;
  geometry_id = -1;
  atlas_texture_id = -1;
  }

font_material::~font_material()
  {
  }

void font_material::_init_font(RenderDoos::render_engine* engine)
  {
  if (FT_Init_FreeType(&_ft)) {
    printf("Error initializing FreeType library\n");
    exit(EXIT_FAILURE);
    }

  if (FT_New_Face(_ft, "assets/fonts/Karla-Regular.ttf", 0, &_face)) {
    printf("Error loading font face\n");
    exit(EXIT_FAILURE);
    }

  // Set pixel size
  FT_Set_Pixel_Sizes(_face, 0, 24);

  // Get atlas dimensions
  FT_GlyphSlot g = _face->glyph;
  int w = 0; // full texture width
  int h = 0; // full texture height
  int row_w = 0; // current row width
  int row_h = 0; // current row height

  int i;
  for (i = 32; i < 128; ++i)
    {
    if (FT_Load_Char(_face, i, FT_LOAD_RENDER))
      {
      printf("Loading Character %d failed\n", i);
      exit(EXIT_FAILURE);
      }

    // If the width will be over max texture width
    // Go to next row
    if (row_w + g->bitmap.width + 1 >= MAX_WIDTH)
      {
      w = std::max(w, row_w);
      h += row_h;
      row_w = 0;
      row_h = 0;
      }
    row_w += g->bitmap.width + 1;
    row_h = std::max<unsigned int>(row_h, g->bitmap.rows);
    }

  // final texture dimensions
  w = std::max(row_w, w);
  h += row_h;

  atlas_width = w;
  atlas_height = h;

  uint8_t* raw_bitmap = new uint8_t[w * h];

  // Fill texture with glyph bitmaps and cache placements
  int offset_x = 0;
  int offset_y = 0;
  row_h = 0;

  for (i = 32; i < 128; ++i)
    {
    if (FT_Load_Char(_face, i, FT_LOAD_RENDER))
      {
      printf("Loading Character %d failed\n", i);
      exit(EXIT_FAILURE);
      }

    // Set correct row
    if (offset_x + g->bitmap.width + 1 >= MAX_WIDTH)
      {
      offset_y += row_h;
      row_h = 0;
      offset_x = 0;
      }

    // fill raw bitmap with glyph
    for (unsigned int y = 0; y < g->bitmap.rows; ++y)
      {
      const unsigned char* p_bitmap = g->bitmap.buffer + y * g->bitmap.width;
      uint8_t* p_tex = raw_bitmap + (y + offset_y) * w + offset_x;
      for (unsigned int x = 0; x < g->bitmap.width; ++x)
        {
        *p_tex++ = *p_bitmap++;
        }
      }

    // Cache values
    char_info[i].ax = g->advance.x >> 6;
    char_info[i].ay = g->advance.y >> 6;
    char_info[i].bw = g->bitmap.width;
    char_info[i].bh = g->bitmap.rows;
    char_info[i].bl = g->bitmap_left;
    char_info[i].bt = g->bitmap_top;
    char_info[i].tx = offset_x / (float)w;
    char_info[i].ty = offset_y / (float)h;

    // Update current position
    row_h = std::max<unsigned int>(row_h, g->bitmap.rows);
    offset_x += g->bitmap.width + 1;
    }

  atlas_texture_id = engine->add_texture(w, h, RenderDoos::texture_format_r8ui, (const uint8_t*)raw_bitmap, TEX_USAGE_READ | TEX_USAGE_RENDER_TARGET);
  delete[] raw_bitmap;
  }

void font_material::compile(RenderDoos::render_engine* engine)
  {
  if (engine->get_renderer_type() == RenderDoos::renderer_type::METAL)
    {
    vs_handle = engine->add_shader(nullptr, SHADER_VERTEX, "font_material_vertex_shader");
    fs_handle = engine->add_shader(nullptr, SHADER_FRAGMENT, "font_material_fragment_shader");
    }
  else if (engine->get_renderer_type() == RenderDoos::renderer_type::OPENGL)
    {
    vs_handle = engine->add_shader(get_font_material_vertex_shader().c_str(), SHADER_VERTEX, nullptr);
    fs_handle = engine->add_shader(get_font_material_fragment_shader().c_str(), SHADER_FRAGMENT, nullptr);
    }
  shader_program_handle = engine->add_program(vs_handle, fs_handle);
  width_handle = engine->add_uniform("width", RenderDoos::uniform_type::integer, 1);
  height_handle = engine->add_uniform("height", RenderDoos::uniform_type::integer, 1);
  _init_font(engine);
  }

void font_material::bind(RenderDoos::render_engine* engine, float* projection, float* camera_space, float* light_dir)
  {
  engine->set_blending_enabled(true);
  engine->set_blending_function(RenderDoos::blending_type::src_alpha, RenderDoos::blending_type::one_minus_src_alpha);
  engine->set_blending_equation(RenderDoos::blending_equation_type::add);

  engine->bind_program(shader_program_handle);

  engine->set_uniform(width_handle, (void*)&atlas_width);
  engine->set_uniform(height_handle, (void*)&atlas_height);

  engine->bind_texture_to_channel(atlas_texture_id, 0, TEX_FILTER_NEAREST | TEX_WRAP_REPEAT);

  engine->bind_uniform(shader_program_handle, width_handle);
  engine->bind_uniform(shader_program_handle, height_handle);
  }

void font_material::destroy(RenderDoos::render_engine* engine)
  {
  engine->remove_shader(vs_handle);
  engine->remove_shader(fs_handle);
  engine->remove_program(shader_program_handle);
  engine->remove_uniform(width_handle);
  engine->remove_uniform(height_handle);
  engine->remove_geometry(geometry_id);
  }

namespace
  {
  text_vert_t make_text_vert(float x, float y, float s, float t, float r, float g, float b)
    {
    text_vert_t out;
    out.x = x;
    out.y = y;
    out.s = s;
    out.t = t;
    out.r = r;
    out.g = g;
    out.b = b;
    return out;
    }
  }

void font_material::prepare_text(RenderDoos::render_engine* engine, const char* text, float x, float y, float sx, float sy, uint32_t clr)
  {
  if (geometry_id >= 0)
    engine->remove_geometry(geometry_id);

  const float x_orig = x;

  std::vector<text_vert_t> verts(6 * strlen(text));
  int n = 0;

  char_info_t* c = char_info;

  float red = (clr & 255) / 255.f;
  float green = ((clr >> 8) & 255) / 255.f;
  float blue = ((clr >> 16) & 255) / 255.f;

  const char* p;
  for (p = text; *p; ++p)
    {
    if (*p == 10)
      {
      y -= c['@'].bh*sy;
      x = x_orig;
      continue;
      }
    float x2 = x + c[*p].bl * sx;
    float y2 = -y - c[*p].bt * sy;
    float w = c[*p].bw * sx;
    float h = c[*p].bh * sy;

    // Advance cursor to start of next char
    x += c[*p].ax * sx;
    y += c[*p].ay * sy;

    // Skip 0 pixel glyphs
    if (!w || !h)
      continue;

    verts[n++] = (text_vert_t)make_text_vert(x2, -y2, c[*p].tx, c[*p].ty, red, green, blue);
    verts[n++] = (text_vert_t)make_text_vert(x2 + w, -y2, c[*p].tx + c[*p].bw / atlas_width, c[*p].ty, red, green, blue);
    verts[n++] = (text_vert_t)make_text_vert(x2, -y2 - h, c[*p].tx, c[*p].ty + c[*p].bh / atlas_height, red, green, blue);
    verts[n++] = (text_vert_t)make_text_vert(x2 + w, -y2, c[*p].tx + c[*p].bw / atlas_width, c[*p].ty, red, green, blue);
    verts[n++] = (text_vert_t)make_text_vert(x2, -y2 - h, c[*p].tx, c[*p].ty + c[*p].bh / atlas_height, red, green, blue);
    verts[n++] = (text_vert_t)make_text_vert(x2 + w, -y2 - h, c[*p].tx + c[*p].bw / atlas_width, c[*p].ty + c[*p].bh / atlas_height, red, green, blue);
    }

  geometry_id = engine->add_geometry(VERTEX_2_2_3);

  text_vert_t* vp;
  uint32_t* ip;

  engine->geometry_begin(geometry_id, verts.size(), verts.size() * 6, (float**)&vp, (void**)&ip);
  memcpy(vp, verts.data(), sizeof(float) * 7 * verts.size());
  for (uint32_t i = 0; i < verts.size(); ++i)
    {
    *ip++ = i * 6;
    *ip++ = i * 6 + 1;
    *ip++ = i * 6 + 2;
    *ip++ = i * 6 + 3;
    *ip++ = i * 6 + 4;
    *ip++ = i * 6 + 5;
    }
  engine->geometry_end(geometry_id);
  //engine->geometry_draw(geometry_id);
  }

void font_material::render_text(RenderDoos::render_engine* engine)
  {
  engine->geometry_draw(geometry_id);
  }
