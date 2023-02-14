#pragma once

#include "SDL.h"

#include "RenderDoos/render_engine.h"
#include "RenderDoos/material.h"

class view
  {
  public:
    view(int argc, char** argv);
    ~view();

    void loop();

  private:
    void _poll_for_events();
    void _prepare_shadertoy_shader();

  private:
    SDL_Window* _window;
    uint32_t _w, _h;
    bool _quit;
    RenderDoos::render_engine _engine;
    RenderDoos::shadertoy_material* _material;
    RenderDoos::model_view_properties _mv_props;
    RenderDoos::shadertoy_material::properties _st_props;
    int32_t _geometry_id;
  };