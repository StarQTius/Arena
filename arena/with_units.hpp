#pragma once

#include <arena/binding/fetcher.hpp>

#include "box2d.hpp"
#include "python.hpp"
#include "utility.hpp"
#include <forward.hpp>

//! \brief Wrap a callable `f` into a unit adaptator function in order to bind it more easily
//! This function convert the parameters from scalar and NumPy objects to quantities and quantity vectors before passing
//! them to the wrapped function (and vice-versa for the return value).
auto with_units(auto &&f) {
  auto impl_for_function = [&]<typename R, typename... Args>(std::function<R(Args...)> *) {
    auto retval = [f = FWD(f)](remove_rvalue_reference_t<to_numpy_t<Args>>... args) -> decltype(auto) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, from_numpy<Args>(FWD(args))...);
      } else {
        return copy_rvalue(to_numpy(std::invoke(f, from_numpy<Args>(FWD(args))...)));
      }
    };

    return retval;
  };

  if constexpr (std::is_member_function_pointer_v<std::decay_t<decltype(f)>>) {
    using free_signature_t = get_free_signature_t<std::decay_t<decltype(f)>>;
    return impl_for_function((std::function<free_signature_t> *)nullptr);
  } else {
    return impl_for_function((decltype(std::function{f}) *)nullptr);
  }
}

template <typename C, typename... Args> auto ctor_with_units() {
  return with_units([](Args... args) { return C{FWD(args)...}; });
}

template <typename Signature_F> auto normalize_box2d(auto &&f) {
  auto impl = [&]<typename R, typename... Args>(R(*)(Args...)) {
    return [f = FWD(f)](Args... args) -> decltype(auto) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, to_box2d(FWD(args))...);
      } else {
        return copy_rvalue(from_box2d<R>(std::invoke(f, to_box2d(FWD(args))...)));
      }
    };
  };

  auto impl_for_members = [&]<typename R, typename C, typename... Args>(R(*)(Args...), auto(C::*)) {
    return impl((R(*)(C &, Args...)) nullptr);
  };

  if constexpr (std::is_member_function_pointer_v<std::decay_t<decltype(f)>>) {
    return impl_for_members((Signature_F *)nullptr, f);
  } else {
    return impl((Signature_F *)nullptr);
  }
}
