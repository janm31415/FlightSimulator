#include "physics.h"

namespace physics
  {

  namespace inertia
    {

    jtk::vec3<float> cube(const jtk::vec3<float>& size, float mass)
      {
      const float C = (1.0f / 12.0f) * mass;
      jtk::vec3<float> I(0.0f);
      I.x = C * (sq(size.y) + sq(size.z));
      I.y = C * (sq(size.x) + sq(size.z));
      I.z = C * (sq(size.x) + sq(size.y));
      return I;
      }

    jtk::vec3<float> cylinder(float radius, float length, float mass)
      {
      const float C = (1.0f / 12.0f) * mass;
      jtk::vec3<float> I(0.0f);
      I.x = (0.5f) * mass * sq(radius);
      I.y = I.z = C * (3 * sq(radius) + sq(length));
      return I;
      }

    jtk::matf9 tensor(const jtk::vec3<float>& moment_of_inertia)
      {
      jtk::matf9 t(3,3);
      t << moment_of_inertia.x, 0.f, 0.f, 0.f, moment_of_inertia.y, 0.f, 0.f, 0.f, moment_of_inertia.z;
      return t;
      }

    element cube_element(const jtk::vec3<float>& position, const jtk::vec3<float>& size, float mass)
      {
      element e;
      e.mass = mass;
      e.position = position;
      e.inertia = cube(size, mass);
      e.offset = position;
      return e;
      }

    jtk::matf9 tensor(std::vector<element>& elements, bool precomputed_offset)
      {
      jtk::matf9 t(3, 3);

      float Ixx = 0, Iyy = 0, Izz = 0;
      float Ixy = 0, Ixz = 0, Iyz = 0;

      float mass = 0;
      jtk::vec3<float> moment(0, 0, 0);

      for (const auto& element : elements)
        {
        mass += element.mass;
        moment = moment + element.mass * element.position;
        }

      const jtk::vec3<float> center_of_gravity = moment / mass;

      for (auto& element : elements)
        {
        jtk::vec3<float> offset;

        if (!precomputed_offset)
          {
          element.offset = element.position - center_of_gravity;
          }

        offset = element.offset;

        Ixx += element.inertia.x + element.mass * (sq(offset.y) + sq(offset.z));
        Iyy += element.inertia.y + element.mass * (sq(offset.z) + sq(offset.x));
        Izz += element.inertia.z + element.mass * (sq(offset.x) + sq(offset.y));
        Ixy += element.mass * (offset.x * offset.y);
        Ixz += element.mass * (offset.x * offset.z);
        Iyz += element.mass * (offset.y * offset.z);
        }

      t << Ixx, -Ixy, -Ixz,
          -Ixy,  Iyy, -Iyz,
          -Ixz, -Iyz,  Izz;
      return t;
      }

    } // namespace inertia

  namespace utils 
    {

    jtk::vec3<float> transform_point(const jtk::float4x4& pose, const jtk::vec3<float>& pt)
      {
      jtk::vec3<float> res(
        pose[0] * pt[0] + pose[4] * pt[1] + pose[8] * pt[2] + pose[12],
        pose[1] * pt[0] + pose[5] * pt[1] + pose[9] * pt[2] + pose[13],
        pose[2] * pt[0] + pose[6] * pt[1] + pose[10] * pt[2] + pose[14]
      );
      return res;
      }

    jtk::vec3<float> transform_vector(const jtk::float4x4& pose, const jtk::vec3<float>& v)
      {
      jtk::vec3<float> res(
        pose[0] * v[0] + pose[4] * v[1] + pose[8] * v[2],
        pose[1] * v[0] + pose[5] * v[1] + pose[9] * v[2],
        pose[2] * v[0] + pose[6] * v[1] + pose[10] * v[2]
      );
      return res;
      }

    jtk::vec3<float> transform_vector(const jtk::matf9& pose, const jtk::vec3<float>& pt)
      {
      jtk::vec3<float> res(
        pose(0, 0) * pt[0] + pose(0, 1) * pt[1] + pose(0, 2) * pt[2],
        pose(1, 0) * pt[0] + pose(1, 1) * pt[1] + pose(1, 2) * pt[2],
        pose(2, 0) * pt[0] + pose(2, 1) * pt[1] + pose(2, 2) * pt[2]
      );
      return res;
      }

    } // namespace utils

  RigidBody::RigidBody()
    {
    m_mass = 1.f;
    m_inertia = inertia::tensor(inertia::cube(jtk::vec3<float>(1.0f), 1.0f));
    m_force = jtk::vec3<float>(0);
    m_torque = jtk::vec3<float>(0);
    m_position = jtk::vec3<float>(0);
    m_velocity = jtk::vec3<float>(0);
    m_angular_velocity = jtk::vec3<float>(0);
    m_apply_gravity = true;
    m_orientation = jtk::get_identity();
    jtk::invert(m_inertia_inverse, m_inertia);
    m_orientation_inverse = jtk::invert_orthonormal(m_orientation);
    }

  RigidBody::RigidBody(const RigidBodyParams& params)
    {
    m_mass = 1.f;
    m_inertia = params.inertia;
    m_force = jtk::vec3<float>(0);
    m_torque = jtk::vec3<float>(0);
    m_position = params.position;
    m_velocity = params.velocity;
    m_angular_velocity = params.angular_velocity;
    m_apply_gravity = params.apply_gravity;
    m_orientation = params.orientation;
    jtk::invert(m_inertia_inverse, m_inertia);
    m_orientation_inverse = jtk::invert_orthonormal(m_orientation);
    }

  jtk::vec3<float> RigidBody::get_point_velocity(const jtk::vec3<float>& point) const
    {
    return inverse_transform_direction(m_velocity) + jtk::cross(m_angular_velocity, point);
    }

  void RigidBody::add_force_at_point(const jtk::vec3<float>& force, const jtk::vec3<float>& point)
    {
    m_force = m_force + transform_direction(force);
    m_torque = m_torque + jtk::cross(point, force);
    }

  jtk::vec3<float> RigidBody::transform_direction(const jtk::vec3<float>& direction) const
    {
    return utils::transform_vector(m_orientation, direction);
    }

  jtk::vec3<float> RigidBody::get_body_velocity() const
    {
    return inverse_transform_direction(m_velocity);
    }

  jtk::vec3<float> RigidBody::inverse_transform_direction(const jtk::vec3<float>& direction) const
    {
    return utils::transform_vector(m_orientation_inverse, direction);
    }

  void RigidBody::set_inertia(const jtk::matf9& inertia_tensor)
    {
    m_inertia = inertia_tensor;
    jtk::invert(m_inertia_inverse, m_inertia);
    }

  void RigidBody::add_force(const jtk::vec3<float>& force)
    {
    m_force = m_force + force;
    }

  void RigidBody::add_relative_force(const jtk::vec3<float>& force)
    {
    m_force = m_force + utils::transform_vector(m_orientation, force);
    }

  void RigidBody::add_torque(const jtk::vec3<float>& torque)
    {
    m_torque = m_torque + inverse_transform_direction(torque);
    }

  void RigidBody::add_relative_torque(const jtk::vec3<float>& torque)
    {
    m_torque = m_torque + torque;
    }

  jtk::vec3<float> RigidBody::get_torque() const
    {
    return m_torque;
    }

  jtk::vec3<float> RigidBody::get_force() const
    {
    return m_force;
    }

  void RigidBody::update(seconds dt)
    {
    jtk::vec3<float> acceleration = m_force / m_mass;

    if (m_apply_gravity)
      acceleration.y -= g;

    m_velocity = m_velocity + acceleration * dt;
    m_position = m_position + m_velocity * dt;


    m_angular_velocity = m_angular_velocity + utils::transform_vector(m_inertia_inverse,
      m_torque - jtk::cross(m_angular_velocity, utils::transform_vector(m_inertia, m_angular_velocity))) * dt;

    jtk::float4x4 rot = jtk::quaternion_to_rotation(jtk::float4(0, m_angular_velocity.x, m_angular_velocity.y, m_angular_velocity.z));

    m_orientation = m_orientation + jtk::matrix_matrix_multiply(m_orientation, rot)*(0.5f*dt);    
    m_orientation_inverse = jtk::invert_orthonormal(m_orientation);

    // reset accumulators
    m_force = jtk::vec3<float>(0,0,0);
    m_torque = jtk::vec3<float>(0, 0, 0);
    }

  } // namespace physics