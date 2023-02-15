#pragma once

#include <vector>
#include "physics.h"

struct ValueTuple
  {
  float alpha, cl, cd;
  };

struct Airfoil
  {
  float min, max;
  std::vector<ValueTuple> data;

  Airfoil(const std::vector<ValueTuple>& curve_data);
  std::tuple<float, float> sample(float alpha) const;
  };

extern Airfoil NACA_0012;
extern Airfoil NACA_2412;

struct Engine
  {
  float throttle = 0.5f;
  float thrust = 10000.0f;
  float horsepower = 1000.0f;
  float rpm = 2400.0f;
  float propellor_diameter = 1.8f;

  Engine(float thrust);
  void apply_forces(physics::RigidBody& rigid_body);
  };

struct Wing
  {
  const float area;
  const jtk::vec3<float> position;
  const Airfoil* airfoil;

  jtk::vec3<float> normal;
  float lift_multiplier = 1.0f;
  float drag_multiplier = 1.0f;
  physics::degrees deflection = 0.0f;

  Wing(const jtk::vec3<float> & position, float area, const Airfoil* aero, const jtk::vec3<float>& normal = physics::UP);
  Wing(const jtk::vec3<float> & position, float wingspan, float chord, const Airfoil* aero, const jtk::vec3<float>& normal = physics::UP);
  void apply_forces(physics::RigidBody& rigid_body);
  };

struct Aircraft
  {
  Engine engine;
  std::vector<Wing> elements;
  physics::RigidBody rigid_body;
  jtk::vec3<float> joystick; // roll, yaw, pitch

  float log_timer = 1.0f;

  Aircraft(float mass, float thrust, jtk::matf9 inertia, std::vector<Wing> wings);
  void update(physics::seconds dt);
  };