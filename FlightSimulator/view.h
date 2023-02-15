#pragma once

#include "SDL.h"
#if defined(RENDERDOOS_METAL)
#include "SDL_metal.h"
#endif
#include "RenderDoos/render_engine.h"
#include "RenderDoos/material.h"

class view
  {
  public:
    view(int argc, char** argv);
    ~view();

    void loop();

  private:
    SDL_Window* _window;
    #if defined(RENDERDOOS_METAL)
    SDL_MetalView _metalView;
    #endif
    uint32_t _w, _h;
    bool _quit;
    RenderDoos::render_engine _engine;   
  };
