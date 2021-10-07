#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL
{
    const double kd = 0.05;
    // const double kd = 0.00005;
    const double damping_factor = 0.00005;
    // const double damping_factor = 0.0005;

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        std::cout << num_nodes << std::endl;
        std::cout << start << ',' << end << std::endl;

        auto dx = (end - start) / (num_nodes - 1);
        for (int i = 0; i <= num_nodes; ++i)
        {
            masses.push_back(
                new Mass(start + dx * i, node_mass, false));
        }
        for (int i = 0; i < num_nodes; ++i)
        {
            springs.push_back(
                new Spring(masses[i], masses[i + 1], k));
        }

        for (auto &i : pinned_nodes)
        {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            auto len = s->m2->position - s->m1->position;
            auto fa = s->k * len.unit() * (len.norm() - s->rest_length);
            s->m1->forces += fa;
            s->m2->forces += -fa;

            // Internal damping
            // const double kd = 0.1;
            // auto dv = s->m2->velocity - s->m1->velocity;
            // auto fb = -kd * dot(len.unit(), dv) * len.unit();
            // s->m1->forces += -fb;
            // s->m2->forces += fb;
        }
        // std::cout << "Add Hooke Force" << std::endl;

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity;
                // TODO (Part 2): Add global damping
                // const double kd = 0.05;
                m->forces += -kd * m->velocity;

                // explicit method
                // m->position += m->velocity * delta_t;
                // m->velocity += m->forces / m->mass * delta_t;
                // semi-implicit method
                m->velocity += m->forces / m->mass * delta_t;
                m->position += m->velocity * delta_t;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        const double INF = std::numeric_limits<double>::max();
        // const double INF = 1e20;

        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            auto len = s->m2->position - s->m1->position;
            auto dx = len.unit() * (len.norm() - s->rest_length);
            auto mass1 = s->m1->pinned ? INF : s->m1->mass;
            auto mass2 = s->m2->pinned ? INF : s->m2->mass;
            s->m1->position += mass2 / (mass1 + mass2) * dx;
            s->m2->position += -mass1 / (mass1 + mass2) * dx;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity;

                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                // TODO (Part 4): Add global Verlet damping
                m->position = m->position +
                              (1 - damping_factor) *
                                  (m->position - m->last_position) +
                              m->forces / m->mass * delta_t * delta_t;
                m->last_position = temp_position;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }
}
