#pragma once

#include "box2d.hpp"
#include "python.hpp"
#include "traits/invocable.hpp"
#include "utility.hpp"
#include <forward.hpp>

//! \brief Wrap a callable `f` into a unit adaptator function in order to bind it more easily
//! This function convert the parameters from scalar and NumPy objects to quantities and quantity vectors before passing
//! them to the wrapped function (and vice-versa for the return value).
auto with_units(WithSignature auto &&f) {
  auto impl = [&]<typename R, typename... Args>(R (*)(Args...)) {
    auto retval = [f = FWD(f)](remove_rvalue_reference_t<to_numpy_t<Args>>... args) -> decltype(auto) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, from_numpy<Args>(FWD(args))...);
      } else {
        return copy_rvalue(to_numpy(std::invoke(f, from_numpy<Args>(FWD(args))...)));
      }
    };

    return retval;
  };

  return impl(free_signature_ptr(f));
}

template <typename C, typename... Args> auto ctor_with_units() {
  return with_units([](Args... args) { return C{FWD(args)...}; });
}

template <WithSignature Signature_F> auto normalize_box2d(auto &&f) {
  auto impl = [&]<typename R, typename... Args>(R (*)(Args...)) {
    return [f = FWD(f)](Args... args) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, to_box2d(FWD(args))...);
      } else {
        return copy_rvalue(from_box2d<R>(std::invoke(f, to_box2d(FWD(args))...)));
      }
    };
  };

  using f_t = std::decay_t<decltype(f)>;

  if constexpr (std::is_member_function_pointer_v<f_t>) {
    return impl((append_args_t<Signature_F *, class_t<f_t> &>)nullptr);
  } else {
    return impl((Signature_F *)nullptr);
  }
}
