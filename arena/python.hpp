#pragma once

#include <pybind11/numpy.h>
#include <units/generic/angle.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/physics.hpp>

#include "physics.hpp"
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
    return quantity_cast<si::millimetre>(FWD(x)).number();
  } else if constexpr (Mass<T>) {
    return quantity_cast<si::kilogram>(FWD(x)).number();
  } else if constexpr (Time<T>) {
    return quantity_cast<si::second>(FWD(x)).number();
  } else if constexpr (Angle<T>) {
    return quantity_cast<radian>(FWD(x)).number();
  } else if constexpr (Quantity<T> && DerivedDimension<typename T::dimension>) {
    return arena::detail::cast_derived_unit(x, [](auto x) { return numpy_number(x); });
  } else {
    static_assert(ARENA_ALWAYS_FALSE, "Unsupported dimension for NumPy");
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

template <typename T> using from_numpy_t = decltype(from_numpy(std::declval<T>()));

template <typename T> using to_numpy_t = decltype(to_numpy(std::declval<T>()));
