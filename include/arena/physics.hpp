#pragma once

#include <type_traits>

#include <box2d/b2_shape.h>
#include <units/exponent.h>
#include <units/generic/angle.h>
#include <units/generic/dimensionless.h>
#include <units/isq/si/acceleration.h>
#include <units/isq/si/force.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/speed.h> // IWYU pragma: keep
#include <units/isq/si/torque.h>
#include <units/math.h>

#include <arena/arena.hpp> // IWYU pragma: export

// IWYU pragma: no_include <units/isq/si/time.h>

namespace arena {

// Precision for calculation
using precision_t = float;

namespace detail {

template <units::Dimension D> constexpr auto base_one_v = units::quantity<D, typename D::base_unit, precision_t>::one();

template <units::Quantity T, typename F> arena::precision_t cast_derived_unit(const T &x, F &&base_caster) {
  using namespace units;

  // When the unit is derived, we use the recipe to decompose it into a product of exponentiated base units.
  // For each of these units `U` and their exponent `E_U`, we multiply `x` by `box2d_scale(1 U) ^ E_U / (1 U) ^ E_U`.

  auto dispatch = [&]<typename... Es>(units::exponent_list<Es...>) {
    auto normal = (pow<Es::num, Es::den>(detail::base_one_v<typename Es::dimension>) * ... * 1);
    auto scale = (std::pow(base_caster(detail::base_one_v<typename Es::dimension>), Es::num / Es::den) * ... * 1);

    return quantity_cast<one>(x / normal).number() * scale;
  };

  return dispatch(typename T::dimension::recipe{});
}

} // namespace detail

// Quantity used for the simulation
using length_t = units::isq::si::length<units::isq::si::metre, precision_t>;
using mass_t = units::isq::si::mass<units::isq::si::kilogram, precision_t>;
using duration_t = units::isq::si::time<units::isq::si::second, precision_t>;
using angle_t = units::angle<units::radian, precision_t>;
using density_t = decltype(mass_t{} / (length_t{} * length_t{}));
using speed_t = decltype(length_t{} / duration_t{});
using angular_speed_t = decltype(angle_t{} / duration_t{});
using force_t = units::isq::si::force<units::isq::si::newton, precision_t>;
using torque_t = units::isq::si::torque<units::isq::si::newton_metre_per_radian, precision_t>;

constexpr angle_t pi = M_PI * angle_t{};

float box2d_number(auto &&x) {
  using namespace units;
  using namespace units::isq;
  using namespace units::isq::si;

  using T = std::remove_cvref_t<decltype(x)>;
  if constexpr (Length<T>) {
    return quantity_cast<metre>(x).number();
  } else if constexpr (Mass<T>) {
    return quantity_cast<kilogram>(x).number();
  } else if constexpr (Time<T>) {
    return quantity_cast<second>(x).number();
  } else if constexpr (Angle<T>) {
    return quantity_cast<radian>(x).number();
  } else if constexpr (Acceleration<T>) {
    return quantity_cast<metre_per_second_sq>(x).number();
  } else if constexpr (Force<T>) {
    return quantity_cast<newton>(x).number();
  } else if constexpr (Torque<T>) {
    return quantity_cast<newton_metre_per_radian>(x).number();
  } else if constexpr (Quantity<T> && DerivedDimension<typename T::dimension>) {
    return detail::cast_derived_unit(x, [](auto x) { return box2d_number(x); });
  } else {
    static_assert(ARENA_ALWAYS_FALSE, "Unsupported dimension for Box2D");
  }
}

// Compute the uniform density of a shape given its mass
density_t compute_shape_density(const b2Shape &, mass_t);

} // namespace arena
