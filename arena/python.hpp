#pragma once

#include <pybind11/numpy.h>

#include <arena/physics.hpp>
#include <units/generic/angle.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>

#include "physics.hpp" // IWYU pragma: keep
#include <forward.hpp>

//! \brief Satisfied when `T` is an instance of `pybind11::array_t`
template <typename T>
concept NumpyArray = requires(T x) {
  {
    pybind11::array_t { x }
    } -> std::convertible_to<T>;
};

//! \brief Convert a quantity to a numeric representation of this quantity in the system of units of the Python
//! interface
auto numpy_number(auto &&x) {
  using namespace units;
  using namespace units::isq;
  using namespace units::angle_references;
  using namespace units::isq::si::time_references;

  using T = std::remove_cvref_t<decltype(x)>;
  if constexpr (Length<T>) {
    return quantity_cast<si::metre>(FWD(x)).number();
  } else if constexpr (Mass<T>) {
    return quantity_cast<si::kilogram>(FWD(x)).number();
  } else if constexpr (Time<T>) {
    return quantity_cast<si::second>(FWD(x)).number();
  } else if constexpr (Angle<T>) {
    return quantity_cast<radian>(FWD(x)).number();
  } else if constexpr (Speed<T>) {
    return quantity_cast<si::metre_per_second>(FWD(x)).number();
  } else if constexpr (QuantityEquivalentTo<arena::angular_speed_t, T>) {
    return quantity_cast<decltype(rad / s)::unit>(FWD(x)).number();
  } else {
    static_assert(arena::always_false<T>, "Unsupported dimension for NumPy");
  }
}

//! \brief Convert quantities to Python objects
//! - A conversion from a quantity will result in the numeric representation of this quantity in the system of units of
//! the Python interface.
//! - A conversion from `quantity_vector` will result in a `pybind11::array_t` whose content is the converted quantity
//! coordinates.
//! - Otherwise, no conversion is performed.
decltype(auto) to_numpy(auto &&x) {
  using T = std::remove_cvref_t<decltype(x)>;
  if constexpr (units::Quantity<T>) {
    return numpy_number(FWD(x));
  } else if constexpr (QuantityVector<T>) {
    arena::precision_t buf[] = {numpy_number(x.x), numpy_number(x.y)};
    return pybind11::array_t{sizeof buf / sizeof *buf, buf};
  } else {
    return FWD(x);
  }
}

//! \brief Convert Python objects to quantities
//! It is the inverse function of `to_numpy`.
template <typename To, typename From> decltype(auto) from_numpy(From &&x) {
  using T = std::remove_cvref_t<From>;
  if constexpr (std::is_arithmetic_v<T> && units::Quantity<To>) {
    return To{x / numpy_number(To{1})};
  } else if constexpr (NumpyArray<T> && QuantityVector<To>) {
    using Q = typename To::quantity;
    auto buf_info = x.request();
    auto *buf = static_cast<arena::precision_t *>(buf_info.ptr);
    return To{from_numpy<Q>(buf[0]), from_numpy<Q>(buf[1])};
  } else {
    return FWD(x);
  }
}
