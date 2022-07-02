#pragma once

//! \brief Wrap a callable `f` into a Box2D unit adaptator function in order to bind it more easily
//! This function solve the following problem : let's say you want to bind a Box2D member function. However, it will
//! probably use `b2Vec2` and `float` to denote scalar quantity with units. Therefore, you must perform unit conversion
//! for each parameter from the system of units used by the Python interface to the system of units of Box2D, which
//! might be tricky. This function can be used to wrap the member function by providing the dimension of each argument,
//! and perform a NumPy -> Box2D unit conversion and vice-versa. Example :
//! - `with_box2d_units<void(length_vec)>(&b2Body::SetPosition)` indicates that the parameter for the member function
//! `SetPosition` is a length vector. `with_box2d_units<length_vec()>(&b2Body::GetPosition)` indicates that the return
//! value of `GetPosition` is a length vector.
template <typename Signature> auto with_box2d_units(auto &&f) {
  auto impl_for_function = [&]<typename R, typename... Args>(R(*)(Args...)) {
    return [f = FWD(f)](decltype(to_numpy(std::declval<Args>()))... args) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, to_box2d(from_numpy<Args>(FWD(args)))...);
      } else {
        return to_numpy(from_box2d<R>(std::invoke(f, to_box2d(from_numpy<Args>(FWD(args)))...)));
      }
    };
  };

  auto impl_for_member_function = [&]<typename R, typename... Args, typename C>(R(*)(Args...), auto C::*) {
    return impl_for_function((R(*)(C &, Args...)) nullptr);
  };

  if constexpr (std::is_member_function_pointer_v<std::decay_t<decltype(f)>>) {
    return impl_for_member_function((Signature *)nullptr, f);
  } else {
    return impl_for_function((Signature *)nullptr);
  }
}

//! \brief Wrap a callable `f` into a unit adaptator function in order to bind it more easily
//! This function convert the parameters from scalar and NumPy objects to quantities and quantity vectors before passing
//! them to the wrapped function (and vice-versa for the return value).
template <typename Signature> auto with_units(auto &&f) {
  auto impl_for_function = [&]<typename R, typename... Args>(R(*)(Args...)) {
    return [f = FWD(f)](decltype(to_numpy(std::declval<Args>()))... args) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, from_numpy<Args>(FWD(args))...);
      } else {
        return to_numpy(std::invoke(f, from_numpy<Args>(FWD(args))...));
      }
    };
  };

  auto impl_for_member_function = [&]<typename R, typename... Args, typename C>(R(*)(Args...), auto C::*) {
    return impl_for_function((R(*)(C &, Args...)) nullptr);
  };

  if constexpr (std::is_member_function_pointer_v<std::decay_t<decltype(f)>>) {
    return impl_for_member_function((Signature *)nullptr, f);
  } else {
    return impl_for_function((Signature *)nullptr);
  }
}

template <typename C, typename... Args> auto ctor_with_units() {
  return with_units<C(Args...)>([](auto &&...args) { return C{FWD(args)...}; });
}
