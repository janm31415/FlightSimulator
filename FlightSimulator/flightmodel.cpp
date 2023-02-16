#include "flightmodel.h"
#include "data.h"
#include <cassert>


Airfoil::Airfoil(const std::vector<ValueTuple>& curve_data) : data(curve_data)
  {
  min = curve_data[0].alpha;
  max = curve_data[curve_data.size() - 1].alpha;
  }

std::tuple<float, float> Airfoil::sample(float alpha) const
  {
  int index = static_cast<int>(physics::utils::scale(alpha, min, max, 0, static_cast<float>(data.size() - 1)));
  index = physics::utils::clamp(index, 0, static_cast<int>(data.size() - 1U));
  if (!(0 <= index && index < data.size()))
    {
    printf("alpha = %f, index = %d, size = %d\n", alpha, index, (int)data.size());
    assert(0);
    }
  return { data[index].cl, data[index].cd };
  }

Airfoil NACA_0012(NACA_0012_data);
Airfoil NACA_2412(NACA_2412_data);

Engine::Engine(float thrust) : thrust(thrust)
  {
  }

void Engine::apply_forces(physics::RigidBody& rigid_body)
  {
  float force = thrust * throttle;
  rigid_body.add_relative_force({ force, 0.0f, 0.0f });
  }

Wing::Wing(const jtk::vec3<float>& position, float area, const Airfoil* aero, const jtk::vec3<float>& normal)
  : position(position),
  area(area),
  airfoil(aero),
  normal(normal)
  {
  }

Wing::Wing(const jtk::vec3<float>& position, float wingspan, float chord, const Airfoil* aero, const jtk::vec3<float>& normal)
  : position(position),
  area(chord* wingspan),
  airfoil(aero),
  normal(normal)
  {
  }

void Wing::apply_forces(physics::RigidBody& rigid_body)
  {
  jtk::vec3<float> local_velocity = rigid_body.get_point_velocity(position);
  float speed = jtk::length(local_velocity);

  if (speed <= 0.0f)
    return;

  jtk::vec3<float> wing_normal = normal;

  if (abs(deflection) > physics::epsilon)
    {
    // set rotation of wing    
    auto axis = jtk::normalize(jtk::cross(physics::FORWARD, normal));    
    jtk::float4x4 rot = jtk::make_rotation(physics::ORIGIN, axis, physics::units::radians(deflection));
    wing_normal = physics::utils::transform_vector(rot, normal);
    }

  // drag acts in the opposite direction of velocity
  jtk::vec3<float> drag_direction = jtk::normalize(-local_velocity);

  // lift is always perpendicular to drag
  jtk::vec3<float> lift_direction
    = jtk::normalize(jtk::cross(jtk::cross(drag_direction, wing_normal), drag_direction));

  // angle between wing and air flow
  float angle_of_attack = physics::units::degrees(std::asin(jtk::dot(drag_direction, wing_normal)));

  // sample our aerodynamic data
  auto [lift_coefficient, drag_coefficient] = airfoil->sample(angle_of_attack);

  float air_density = physics::rho;

  float tmp = 0.5f * physics::sq(speed) * air_density * area;
  jtk::vec3<float> lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
  jtk::vec3<float> drag = drag_direction * drag_coefficient * drag_multiplier * tmp;

  // apply forces
  rigid_body.add_force_at_point(lift + drag, position);
  }

Aircraft::Aircraft(float mass, float thrust, jtk::matf9 inertia, std::vector<Wing> wings) : 
  elements(wings), engine(thrust)
  {
  physics::RigidBodyParams pars;
  pars.mass = mass;
  pars.inertia = inertia;
  rigid_body = physics::RigidBody(pars);
  }

void Aircraft::update(physics::seconds dt)
  {
  Wing& la = elements[1];
  Wing& ra = elements[2];
  Wing& el = elements[4];
  Wing& ru = elements[5];

  float roll = joystick.x;
  float yaw = joystick.y;
  float pitch = joystick.z;
  float max_elevator_deflection = 5.0f, max_aileron_deflection = 15.0f, max_rudder_deflection = 5.0f;
  float aileron_deflection = roll * max_aileron_deflection;

  la.deflection = +aileron_deflection;
  ra.deflection = -aileron_deflection;
  el.deflection = -(pitch * max_elevator_deflection);
  ru.deflection = yaw * max_rudder_deflection;

  for (Wing& wing : elements)
    {
    wing.apply_forces(rigid_body);
    }

  engine.apply_forces(rigid_body);

  if ((log_timer += dt) > 0.5f)
    {
    log_timer = 0;
#if 1
    printf(
      "%.2f km/h, thr: %.2f, alt: %.2f m,  ang. vel: %.2f, %.2f, %.2f\n",
      physics::units::kilometer_per_hour(jtk::length(rigid_body.get_velocity())),
      engine.throttle,
      rigid_body.get_position().y,
      rigid_body.get_angular_velocity().x,
      rigid_body.get_angular_velocity().y,
      rigid_body.get_angular_velocity().z
    );
#endif
    }

  rigid_body.update(dt);
  }