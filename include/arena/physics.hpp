#pragma once

#include <box2d/b2_shape.h>
#include <type_traits>
#include <units/generic/angle.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>

#include <arena/concept.hpp> // IWYU pragma: keep

namespace arena {

// Precision for calculation
using precision_t = float;

// Quantity used for the simulation
using length_t = units::isq::si::length<units::isq::si::metre, precision_t>;
using mass_t = units::isq::si::mass<units::isq::si::kilogram, precision_t>;
using duration_t = units::isq::si::time<units::isq::si::second, precision_t>;
using angle_t = units::angle<units::radian, precision_t>;
using density_t = decltype(mass_t{} / (length_t{} * length_t{}));
using speed_t = decltype(length_t{} / duration_t{});
using angular_speed_t = decltype(angle_t{} / duration_t{});

auto box2d_number(auto &&x) {
  using namespace units;
  using namespace units::isq;
  using namespace units::angle_references;
  using namespace units::isq::si::time_references;

  using T = std::remove_cvref_t<decltype(x)>;
  if constexpr (Length<T>) {
    return quantity_cast<si::metre>(x).number();
  } else if constexpr (Mass<T>) {
    return quantity_cast<si::kilogram>(x).number();
  } else if constexpr (Time<T>) {
    return quantity_cast<si::second>(x).number();
  } else if constexpr (Angle<T>) {
    return quantity_cast<radian>(x).number();
  } else if constexpr (Speed<T>) {
    return quantity_cast<si::metre_per_second>(x).number();
  } else if constexpr (QuantityEquivalentTo<angular_speed_t, T>) {
    return quantity_cast<decltype(rad / s)::unit>(x).number();
  } else {
    static_assert(always_false<T>, "Unsupported dimension for Box2D");
  }
}

// Compute the uniform density of a shape given its mass
density_t compute_shape_density(const b2Shape &, mass_t);

} // namespace arena
