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

#include "scene.h"
#include "flightmodel.h"
#include "material.h"

#include "RenderDoos/types.h"
#include "RenderDoos/float.h"

struct Joystick
  {
  float roll = 0;
  float pitch = 0;
  float yaw = 0;
  float throttle = 0;
  };

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
  }

view::~view()
  {
  _engine.destroy();
  }

void view::loop()
  {
  const float mass = 10000.0f;
  const float thrust = 20000.0f;
  std::vector<physics::inertia::element> elements = {
      physics::inertia::cube_element({-0.5f,  0.0f, -2.7f}, {6.96f, 0.10f, 3.50f}, mass * 0.25f),               // left wing
      physics::inertia::cube_element({ 0.0f,  0.0f, -2.0f}, {3.80f, 0.10f, 1.26f}, mass * 0.05f),               // left aileron
      physics::inertia::cube_element({ 0.0f,  0.0f,  2.0f}, {3.80f, 0.10f, 1.26f}, mass * 0.05f),               // right aileron
      physics::inertia::cube_element({-0.5f,  0.0f,  2.7f}, {6.96f, 0.10f, 3.50f}, mass * 0.25f),               // right wing
      physics::inertia::cube_element({-6.6f, -0.1f,  0.0f}, {6.54f, 0.10f, 2.70f}, mass * 0.2f),                // elevator
      physics::inertia::cube_element({-6.6f,  0.0f,  0.0f}, {5.31f, 3.10f, 0.10f}, mass * 0.2f),                // rudder
    };
  auto inertia = physics::inertia::tensor({ 100000.0f, 400000.0f, 500000.0f });
  auto inertia_tensor = physics::inertia::tensor(elements, true);
  std::vector<Wing> wings = {
    Wing({-0.5f,   0.0f, -2.7f}, 6.96f, 3.50f, &NACA_2412),              // left wing
    Wing({ 0.0f,   0.0f, -2.0f},  3.80f, 1.26f, &NACA_0012),              // left aileron
    Wing({ 0.0f,   0.0f,  2.0f},  3.80f, 1.26f, &NACA_0012),              // right aileron
    Wing({-0.5f,   0.0f,  2.7f}, 6.96f, 3.50f, &NACA_2412),              // right wing
    Wing({-6.6f, -0.1f, 0.0f},  6.54f, 2.70f, &NACA_0012),              // elevator
    Wing({-6.6f,  0.0f,  0.0f},  5.31f, 3.10f, &NACA_0012, physics::RIGHT),  // rudder
    };
  jtk::vec3<float> position = jtk::vec3<float>(0.0f, 2000.0f, 0.0f);
  jtk::vec3<float> velocity = jtk::vec3<float>(physics::units::meter_per_second(600.0f), 0.0f, 0.0f);

  Aircraft aircraft(mass, thrust, inertia_tensor, wings);
  aircraft.rigid_body.set_position(position);
  aircraft.rigid_body.set_velocity(velocity);

  mesh fuselage;
  fuselage.init_from_ply_file(_engine, "assets/models/fuselage.ply");
  mesh propeller;
  propeller.init_from_ply_file(_engine, "assets/models/propeller.ply");

  texture colors;
  colors.init_from_file(_engine, "assets/textures/colors.png");

  cubemap skybox;
  skybox.init_from_file(_engine,
    "assets/textures/skybox/front.jpg",
    "assets/textures/skybox/back.jpg",
    "assets/textures/skybox/left.jpg",
    "assets/textures/skybox/right.jpg",
    "assets/textures/skybox/top.jpg",
    "assets/textures/skybox/bottom.jpg"
  );

  simple_material mat;
  mat.compile(&_engine);

  cubemap_material cmat;
  cmat.compile(&_engine);

  uint32_t framebuffer_id = _engine.add_frame_buffer(_w, _h, false);

  uint32_t quad_id = _engine.add_geometry(VERTEX_STANDARD);
  RenderDoos::vertex_standard* vp;
  uint32_t* ip;

  _engine.geometry_begin(quad_id, 4, 6, (float**)&vp, (void**)&ip);
  // make a quad for drawing the texture

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

  _engine.geometry_end(quad_id);

#if defined(RENDERDOOS_METAL)
  RenderDoos::float4x4 projection_ortho = RenderDoos::orthographic(-1, 1, 1, -1, 0, 1);
#else
  RenderDoos::float4x4 projection_ortho = RenderDoos::orthographic(-1, 1, -1, 1, 0, 1);
#endif

  jtk::float4x4 projection_skybox = perspective(physics::units::radians(45.f), (float)_w / (float)_h, 0.125f, 4096.f);

  Joystick joystick;

  camera cam(physics::units::radians(45.f), (float)_w / (float)_h, 1.f, 50000.f);
  cam.set_position(0, 1, 0);
  cam.set_rotation(0, physics::units::radians(-90.f), 0.f);

  bool orbit = false;
  float propeller_rotation = 0.f;

  auto last_tic = std::chrono::high_resolution_clock::now();
  auto start = last_tic;
  while (!_quit)
    {
    auto tic = std::chrono::high_resolution_clock::now();
    float dt = (float)(std::chrono::duration_cast<std::chrono::microseconds>(tic - last_tic).count()) / 1000000.f;
    if (dt > 0.02f)
      dt = 0.02f;
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
        case SDL_KEYDOWN:
        {
        switch (event.key.keysym.sym)
          {
          case SDLK_ESCAPE:
          {
          _quit = true;
          break;
          }
          case SDLK_o:
            cam.set_position(0, 1, 0);
            cam.set_rotation(0, physics::units::radians(-90.f), 0.f);
            orbit = !orbit;
            break;
          } // switch (event.key.keysym.sym) 
        break;
        } // case SDL_KEYDOWN
        default:
          break;
        }
      }

    joystick.pitch = joystick.roll = joystick.yaw = 0;

    const uint8_t* key_states = SDL_GetKeyboardState(NULL);
    if (key_states[SDL_SCANCODE_A])
      {
      joystick.roll = -1.0f;
      }
    else if (key_states[SDL_SCANCODE_D])
      {
      joystick.roll = 1.0f;
      }

    if (key_states[SDL_SCANCODE_W])
      {
      joystick.pitch = -1.0f;
      }
    else if (key_states[SDL_SCANCODE_S])
      {
      joystick.pitch = 1.0f;
      }

    if (key_states[SDL_SCANCODE_Q])
      {
      joystick.yaw = 1.0f;
      }
    else if (key_states[SDL_SCANCODE_E])
      {
      joystick.yaw = -1.0f;
      }

    if (key_states[SDL_SCANCODE_J])
      {
      joystick.throttle -= 0.01f;
      joystick.throttle = physics::utils::clamp(joystick.throttle, 0.0f, 1.0f);
      }
    else if (key_states[SDL_SCANCODE_K])
      {
      joystick.throttle += 0.01f;
      joystick.throttle = physics::utils::clamp(joystick.throttle, 0.0f, 1.0f);
      }

    aircraft.joystick = jtk::vec3<float>(joystick.roll, joystick.yaw, joystick.pitch);
    aircraft.engine.throttle = joystick.throttle;

    aircraft.update(dt);
    if (orbit)
      {
      const float radius = 20.f;
      float pitch = 0.f;
      float yaw = 0.f;
      const jtk::vec3<float> center(0);
      jtk::vec3<float> front;
      front.x = cos(yaw) * cos(pitch);
      front.y = sin(pitch);
      front.z = sin(yaw) * cos(pitch);
      auto offset = jtk::normalize(front) * radius;
      jtk::vec3<float> pos = center + offset;
      pos = aircraft.rigid_body.inverse_transform_direction(pos);
      cam.set_position(pos.x, pos.y, pos.z);
      jtk::vec3<float> up(0, 1, 0);
      up = aircraft.rigid_body.inverse_transform_direction(up);
      cam.set_up(up);
      cam.look_at(center);
      }
    else
      {
      cam.set_position(-15.0f, 3.0f + aircraft.rigid_body.get_angular_velocity().z * 1.0f, 0.0f);
      }

    RenderDoos::render_drawables drawables;
#if defined(RENDERDOOS_METAL)
    void* layer = SDL_Metal_GetLayer(_metalView);
    CA::MetalDrawable* drawable = next_drawable(layer);
    drawables.metal_drawable = (void*)drawable;
    drawables.metal_screen_texture = (void*)drawable->texture();
#endif
    _engine.frame_begin(drawables);

    RenderDoos::renderpass_descriptor descr;
    descr.clear_color = 0xff203040;
    descr.clear_flags = CLEAR_COLOR | CLEAR_DEPTH;
    descr.w = _w;
    descr.h = _h;
    descr.frame_buffer_handle = framebuffer_id;
    descr.frame_buffer_channel = 10;
    descr.clear_depth = 1;

    mat.set_texture(colors.texture_id, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);

    _engine.renderpass_begin(descr);
    jtk::float4x4 view_matrix = jtk::get_identity();
    jtk::vec3<float> xa(1, 0, 0);
    jtk::vec3<float> ya(0, 1, 0);
    jtk::vec3<float> za(0, 0, 1);
    xa = aircraft.rigid_body.inverse_transform_direction(xa);
    ya = aircraft.rigid_body.inverse_transform_direction(ya);
    za = aircraft.rigid_body.inverse_transform_direction(za);
    jtk::set_x_axis(view_matrix, xa);
    jtk::set_y_axis(view_matrix, ya);
    jtk::set_z_axis(view_matrix, za);
    jtk::float4x4 roty = jtk::make_rotation(physics::ORIGIN, physics::Y_AXIS, physics::units::degrees(-90.f));
    view_matrix = jtk::matrix_matrix_multiply(roty, view_matrix);
    jtk::vec3<float> light(0);
    cmat.set_cubemap(skybox.texture_id, TEX_WRAP_REPEAT | TEX_FILTER_LINEAR);
    cmat.bind(&_engine, &projection_skybox[0], &view_matrix[0], &light[0]);
    _engine.geometry_draw(skybox.geometry_id);
    _engine.renderpass_end();

    descr.clear_flags = CLEAR_DEPTH;
    _engine.renderpass_begin(descr);
    view_matrix = cam.get_view_matrix();
    light = jtk::normalize(jtk::vec3<float>(0, 1, 0));
    light = aircraft.rigid_body.inverse_transform_direction(light);
    light = physics::utils::transform_vector(view_matrix, light);
    mat.bind(&_engine, &cam.get_projection_matrix()[0], &view_matrix[0], &light[0]);
    _engine.geometry_draw(fuselage.geometry_id);
    _engine.renderpass_end();

    descr.clear_flags = 0;
    _engine.renderpass_begin(descr);
    propeller_rotation += 0.5f;
    if (propeller_rotation > 2.f * 3.1415926535f)
      propeller_rotation -= 2.f * 3.1415926535f;

    view_matrix = cam.get_view_matrix();
    jtk::float4x4 rot = jtk::make_rotation(physics::ORIGIN, physics::X_AXIS, propeller_rotation);
    view_matrix = jtk::matrix_matrix_multiply(view_matrix, rot);
    mat.bind(&_engine, &cam.get_projection_matrix()[0], &view_matrix[0], &light[0]);
    _engine.geometry_draw(propeller.geometry_id);

    _engine.renderpass_end();

    descr.frame_buffer_handle = -1;
    descr.clear_color = 0xff00ffff;
    descr.clear_flags = CLEAR_COLOR | CLEAR_DEPTH;

    _engine.renderpass_begin(descr);

    mat.set_texture(_engine.get_frame_buffer(framebuffer_id)->texture_handle, TEX_WRAP_REPEAT | TEX_FILTER_NEAREST);
    view_matrix = jtk::get_identity();
    light = jtk::vec3<float>(0, 0, 1);
    mat.bind(&_engine, &projection_ortho[0], &view_matrix[0], &light[0]);
    _engine.geometry_draw(quad_id);

    _engine.renderpass_end();
    _engine.frame_end();


    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(10.0));

#if defined(RENDERDOOS_OPENGL)
    SDL_GL_SwapWindow(_window);
#endif
    last_tic = tic;
    }

  mat.destroy(&_engine);
  cmat.destroy(&_engine);
  skybox.cleanup(_engine);
  fuselage.cleanup(_engine);
  propeller.cleanup(_engine);
  colors.cleanup(_engine);
  }
