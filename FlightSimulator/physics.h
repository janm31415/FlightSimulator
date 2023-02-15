#pragma once

#include "jtk/vec.h"
#include "jtk/qbvh.h"
#include "jtk/mat.h"

#include <vector>

namespace physics
  {

  typedef float seconds;
  typedef float radians;
  typedef float degrees;

  constexpr float g = 9.81f; // gravity of earth,  m/s^2
  constexpr float rho = 1.225f;   // air density, kg/m^3
  constexpr float epsilon = 1e-8f;

  // directions in body space
  const jtk::vec3<float> UP(0.0f, 1.0f, 0.0f);
  const jtk::vec3<float> DOWN(0.0f, -1.0f, 0.0f);
  const jtk::vec3<float> RIGHT(0.0f, 0.0f, 1.0f);
  const jtk::vec3<float> LEFT(0.0f, 0.0f, -1.0f);
  const jtk::vec3<float> FORWARD(1.0f, 0.0f, 0.0f);
  const jtk::vec3<float> BACKWARD(-1.0f, 0.0f, 0.0f);

  const jtk::vec3<float> ORIGIN(0.0f, 0.0f, 0.0f);
  const jtk::vec3<float> X_AXIS(1.0f, 0.0f, 0.0f);
  const jtk::vec3<float> Y_AXIS(0.0f, 1.0f, 0.0f);
  const jtk::vec3<float> Z_AXIS(0.0f, 0.0f, 1.0f);

  template<typename T>
  constexpr inline T sq(T a)
    {
    return a * a;
    }

  namespace inertia
    {

    struct element
      {
      float mass;
      jtk::vec3<float> position;// position in design coordinates
      jtk::vec3<float> inertia;
      jtk::vec3<float> offset;   // offset from center of gravity
      };

    jtk::vec3<float> cube(const jtk::vec3<float>& size, float mass);
    jtk::vec3<float> cylinder(float radius, float length, float mass);
    jtk::matf9 tensor(const jtk::vec3<float>& moment_of_inertia);
    element cube_element(const jtk::vec3<float>& position, const jtk::vec3<float>& size, float mass);

    // calculate inertia tensor from list of connected masses
    jtk::matf9 tensor(std::vector<element>& elements, bool precomputed_offset = false);

    } // namespace inertia

  namespace utils 
    {

    inline float scale(float input, float in_min, float in_max, float out_min, float out_max)
      {
      return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
      }

    inline float lerp(float a, float b, float t)
      {
      return a + t * (b - a);
      }

    template <typename T>
    inline T max(T a, T b)
      {
      return a > b ? a : b;
      }

    template <typename T>
    inline T min(T a, T b)
      {
      return a < b ? a : b;
      }

    inline float sign(float a)
      {
      return a >= 0.f ? 1.f : -1.f;
      }

    template <typename T>
    inline T clamp(T v, T lo, T hi)
      {
      return v < lo ? lo : v > hi ? hi : v;
      }

    jtk::vec3<float> transform_point(const jtk::float4x4& pose, const jtk::vec3<float>& pt);
    jtk::vec3<float> transform_vector(const jtk::float4x4& pose, const jtk::vec3<float>& v);
    jtk::vec3<float> transform_vector(const jtk::matf9& pose, const jtk::vec3<float>& v);

    } // namespace utils

  namespace units 
    {

    inline float knots(float meter_per_second)
      {
      return meter_per_second * 1.94384f;
      }

    inline float meter_per_second(float kilometer_per_hour)
      {
      return kilometer_per_hour / 3.6f;
      }

    inline float kilometer_per_hour(float meter_per_second)
      {
      return meter_per_second * 3.6f;
      }

    inline float kelvin(float celsius)
      {
      return celsius - 273.15f;
      }

    inline float radians(float deg)
      {
      return deg/180.f*3.1415926535897f;
      }

    inline float degrees(float rad)
      {
      return rad/3.1415926535897f*180.f;
      }

    } // namespace units


  struct RigidBodyParams 
    {
    RigidBodyParams();
    float mass;
    jtk::matf9 inertia;
    jtk::vec3<float> position;
    jtk::vec3<float> velocity;
    jtk::vec3<float> angular_velocity;
    jtk::float4x4 orientation;
    bool apply_gravity;
    };

  class RigidBody
    {
    public:
      RigidBody();
      RigidBody(const RigidBodyParams& params);

      // get velocity of point in body space
      jtk::vec3<float> get_point_velocity(const jtk::vec3<float>& point) const;
      
      // force and point vectors are in body space 
      void add_force_at_point(const jtk::vec3<float>& force, const jtk::vec3<float>& point);

      // transform direction from body space to world space 
      jtk::vec3<float> transform_direction(const jtk::vec3<float>& direction) const;

      jtk::vec3<float> get_body_velocity() const;

      // transform direction from world space to body space 
      jtk::vec3<float> inverse_transform_direction(const jtk::vec3<float>& direction) const;

      void set_inertia(const jtk::matf9& inertia_tensor);

      // force vector in world space
      void add_force(const jtk::vec3<float>& force);

      // force vector in body space
      void add_relative_force(const jtk::vec3<float>& force);

      // torque vector in world space
      void add_torque(const jtk::vec3<float>& torque);

      // torque vector in body space
      void add_relative_torque(const jtk::vec3<float>& torque);

      // get torque in body space
      jtk::vec3<float> get_torque() const;

      // get torque in world space
      jtk::vec3<float> get_force() const;

      jtk::vec3<float> get_position() const;

      jtk::vec3<float> get_velocity() const;

      jtk::vec3<float> get_angular_velocity() const;

      void set_position(const jtk::vec3<float>& pos);

      void set_velocity(const jtk::vec3<float>& vel);

      void set_angular_velocity(const jtk::vec3<float>& vel);

      void update(seconds dt);

    private:
      jtk::vec3<float> m_force;
      jtk::vec3<float> m_torque;
      float m_mass;
      jtk::vec3<float> m_position;
      jtk::float4x4 m_orientation;
      jtk::float4x4 m_orientation_inverse;
      jtk::vec3<float> m_velocity;
      jtk::vec3<float> m_angular_velocity;
      jtk::matf9 m_inertia;
      jtk::matf9 m_inertia_inverse;
      bool m_apply_gravity;
    };
  }