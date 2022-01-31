#pragma once

#include <memory>

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>

namespace arena {

// Precision for calculation
using precision_t = float;

// Quantity used for the simulation
using distance_t = units::isq::si::length<units::isq::si::metre, precision_t>;
using mass_t = units::isq::si::mass<units::isq::si::kilogram, precision_t>;
;
using density_t = decltype(mass_t{} / (distance_t{} * distance_t{}));

// Quantity used by Box2D
using box2d_distance_t = distance_t;
using box2d_mass_t = mass_t;
using box2d_density_t = density_t;

// Compute the uniform density of a shape given its mass
box2d_density_t compute_shape_density(const b2Shape &, mass_t);

namespace component {

// Deleters for Box2D objects
// WARNING : be careful to delete any 'b2World' instance AFTER deleting the Box2D objects managed by that instance
inline auto delete_body = [](b2Body *body_ptr) { body_ptr->GetWorld()->DestroyBody(body_ptr); };

// Thin wrapper around Box2D classes which allows automatic destruction
using BodyPtr = std::unique_ptr<b2Body, decltype(delete_body)>;

// Convenience function to create Box2D shapes
b2CircleShape make_circle_shape(distance_t);

} // namespace component
} // namespace arena
