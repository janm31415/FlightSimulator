#include "view.h"
#include <stdexcept>
#include <chrono>

#if defined(RENDERDOOS_METAL)
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "metal/Metal.hpp"
#include "../SDL-metal/SDL_metal.h"
#include "SDL_metal.h"
#else
#include "glew/GL/glew.h"
#endif

#include <thread>

view::view(int /*argc*/, char** /*argv*/) : _w(1600), _h(900), _quit(false)
  {
  // Setup window
#if defined(RENDERDOOS_METAL)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
  
  _window = SDL_CreateWindow("FlightSimulator",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    _w, _h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

  _metalView = SDL_Metal_CreateView(_window);
  void* layer = SDL_Metal_GetLayer(_metalView);
  MTL::Device* metalDevice = MTL::CreateSystemDefaultDevice();
  assign_device(layer, metalDevice);
  if (!_window)
    throw std::runtime_error("SDL can't create a window");
    
    
  _engine.init(metalDevice, nullptr, RenderDoos::renderer_type::METAL);
#else
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  _window = SDL_CreateWindow("FlightSimulator",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    _w, _h,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

  if (!_window)
    throw std::runtime_error("SDL can't create a window");

  SDL_GLContext gl_context = SDL_GL_CreateContext(_window);
  SDL_GL_SetSwapInterval(1); // Enable vsync


  glewExperimental = true;
  GLenum err = glewInit();
  if (GLEW_OK != err)
    throw std::runtime_error("GLEW initialization failed");
  glGetError(); // hack https://stackoverflow.com/questions/36326333/openglglfw-glgenvertexarrays-returns-gl-invalid-operation


  SDL_GL_MakeCurrent(_window, gl_context);

  _engine.init(nullptr, nullptr, RenderDoos::renderer_type::OPENGL);
#endif  

  _mv_props.init(_w, _h);
  _mv_props.orthogonal = 1;
  _mv_props.near_clip = 1.f;
  _mv_props.zoom_x = 1.f;
  _mv_props.zoom_y = 1.f;
  _mv_props.light_dir = RenderDoos::normalize(RenderDoos::float4(0, 0, 1, 0));
  _st_props.frame = 0;
  _st_props.time = 0;
  _st_props.time_delta = 0;
  _material = new RenderDoos::shadertoy_material();

  _prepare_shadertoy_shader();
  }

view::~view()
  {
  _engine.destroy();
  delete _material;
  _material = nullptr;
  }

void view::_prepare_shadertoy_shader()
  {
#if defined(RENDERDOOS_METAL)
  std::string script = std::string(R"(void mainImage(thread float4& fragColor, float2 fragCoord, float iTime, float3 iResolution)
{
  float2 uv = fragCoord / iResolution.xy;
  float3 col = 0.5 + 0.5*cos(iTime + uv.xyx + float3(0, 2, 4));
  
  fragColor = float4(col[0], col[1], col[2], 1);
})");
#else
  std::string script = std::string(R"(void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    // Output to screen
    fragColor = vec4(col,1.0);
})");
#endif
  _material->set_script(script);
  _material->compile(&_engine);
  _geometry_id = _engine.add_geometry(VERTEX_STANDARD);

  RenderDoos::vertex_standard* vp;
  uint32_t* ip;

  _engine.geometry_begin(_geometry_id, 4, 6, (float**)&vp, (void**)&ip);
  // drawing

  vp->x = -1.f;
  vp->y = -1.f;
  vp->z = 0.f;
  vp->nx = 0.f;
  vp->ny = 0.f;
  vp->nz = 1.f;
  vp->u = 0.f;
  vp->v = 0.f;
  ++vp;
  vp->x = 1.f;
  vp->y = -1.f;
  vp->z = 0.f;
  vp->nx = 0.f;
  vp->ny = 0.f;
  vp->nz = 1.f;
  vp->u = 1.f;
  vp->v = 0.f;
  ++vp;
  vp->x = 1.f;
  vp->y = 1.f;
  vp->z = 0.f;
  vp->nx = 0.f;
  vp->ny = 0.f;
  vp->nz = 1.f;
  vp->u = 1.f;
  vp->v = 1.f;
  ++vp;
  vp->x = -1.f;
  vp->y = 1.f;
  vp->z = 0.f;
  vp->nx = 0.f;
  vp->ny = 0.f;
  vp->nz = 1.f;
  vp->u = 0.f;
  vp->v = 1.f;

  ip[0] = 0;
  ip[1] = 1;
  ip[2] = 2;
  ip[3] = 0;
  ip[4] = 2;
  ip[5] = 3;

  _engine.geometry_end(_geometry_id);
  }

void view::loop()
  {
  auto last_tic = std::chrono::high_resolution_clock::now();
  auto start = last_tic;
  while (!_quit)
    {
    auto tic = std::chrono::high_resolution_clock::now();    
    _poll_for_events();


    RenderDoos::render_drawables drawables;
#if defined(RENDERDOOS_METAL)
    void* layer = SDL_Metal_GetLayer(_metalView);
    CA::MetalDrawable* drawable = next_drawable(layer);
    drawables.metal_drawable = (void*)drawable;
    drawables.metal_screen_texture = (void*)drawable->texture();
#endif
    _engine.frame_begin(drawables);
    _st_props.time_delta = (float)(std::chrono::duration_cast<std::chrono::microseconds>(tic - last_tic).count()) / 1000000.f;    
    _st_props.time = (float)(std::chrono::duration_cast<std::chrono::microseconds>(tic - start).count()) / 1000000.f;
    _material->set_shadertoy_properties(_st_props);

    RenderDoos::renderpass_descriptor descr;
    descr.clear_color = 0xff203040;
    descr.clear_flags = CLEAR_COLOR | CLEAR_DEPTH;
    descr.w = _mv_props.viewport_width;
    descr.h = _mv_props.viewport_height;

    _engine.renderpass_begin(descr);

    _engine.set_model_view_properties(_mv_props);
    _material->bind(&_engine);

    _engine.geometry_draw(_geometry_id);

    _engine.renderpass_end();
    _engine.frame_end();
    ++_st_props.frame;

    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1.0));
    //SDL_GL_SwapWindow(_window);
    last_tic = tic;
    }
  }

void view::_poll_for_events()
  {
  SDL_Event event;
  while (SDL_PollEvent(&event))
    {
    switch (event.type)
      {
      case SDL_QUIT:
      {
      _quit = true;
      break;
      }
      default:
        break;
      }
    }
  }
