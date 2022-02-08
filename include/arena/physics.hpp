#pragma once

#include <box2d/b2_shape.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

namespace arena {

// Precision for calculation
using precision_t = float;

// Quantity used for the simulation
using distance_t = units::isq::si::length<units::isq::si::metre, precision_t>;
using mass_t = units::isq::si::mass<units::isq::si::kilogram, precision_t>;
using time_t = units::isq::si::time<units::isq::si::second, precision_t>;
using density_t = decltype(mass_t{} / (distance_t{} * distance_t{}));

// Quantity used by Box2D
using box2d_distance_t = distance_t;
using box2d_mass_t = mass_t;
using box2d_time_t = time_t;
using box2d_density_t = density_t;

// Compute the uniform density of a shape given its mass
box2d_density_t compute_shape_density(const b2Shape &, mass_t);

} // namespace arena
