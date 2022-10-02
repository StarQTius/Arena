#pragma once

#include <pybind11/pybind11.h>
#include <units/quantity.h>

#include <arena/arena.hpp>
#include <arena/physics.hpp>

#include "python.hpp"
#include "utility.hpp"

namespace PYBIND11_NAMESPACE {
namespace detail {
template <typename Dim, typename U, typename Rep>
struct type_caster<units::quantity<Dim, U, Rep>> : public type_caster_base<units::quantity<Dim, U, Rep>> {
  using type = units::quantity<Dim, U, Rep>;

public:
  PYBIND11_TYPE_CASTER(type, const_name("Quantity"));

  bool load(handle src, bool) {
    auto pyval = reinterpret_borrow<float_>(src).cast<arena::precision_t>();
    value = from_numpy<type>(pyval);

    return true;
  }

  static handle cast(type src, return_value_policy, handle) { return float_{to_numpy(src)}.release(); }
};

template <units::Quantity Q> struct type_caster<quantity_vector<Q>> {
  using type = quantity_vector<Q>;

public:
  PYBIND11_TYPE_CASTER(type, const_name("QuantityVector"));

  bool load(handle src, bool) {
    if (!isinstance<iterable>(src))
      return false;

    auto pyval = reinterpret_borrow<iterable>(src).cast<array_t<arena::precision_t, 2>>();
    value = from_numpy<type>(pyval);

    return true;
  }

  static handle cast(const type &src, return_value_policy, handle) { return to_numpy(src).release(); }
};

template <typename T> struct type_caster<arena::Expected<T>> {
  using type = arena::Expected<T>;

public:
  PYBIND11_TYPE_CASTER(type, const_name("Monad"));

  bool load(handle src, bool) { return false; }

  template <typename Xp_T> static handle cast(Xp_T &&src, return_value_policy policy, handle parent) {
    src.or_else(pyraise);

    if (!std::is_lvalue_reference<Xp_T>::value) {
      policy = return_value_policy_override<T>::policy(policy);
    }

    if constexpr (std::is_void_v<T>) {
      return none{}.inc_ref();
    } else {
      return pybind11::cast(FWD(src).value(), policy, parent).release();
    }
  }
};

} // namespace detail
} // namespace PYBIND11_NAMESPACE
