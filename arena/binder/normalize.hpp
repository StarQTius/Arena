#pragma once

#include <pybind11/pybind11.h>

#include <arena/environment.hpp>

#include "../box2d.hpp"
#include "../component_ref.hpp"
#include "../python.hpp"
#include "../traits/crtp.hpp"
#include "../traits/invocable.hpp"
#include "../utility.hpp"
#include <forward.hpp>

namespace detail {

template <typename, typename F, typename R, typename... Args> auto normalize_impl(F &&f, R (*)(Args...)) {
  return [f = FWD(f)](to_numpy_t<Args> &...args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, from_numpy<Args>(args)...);
    } else {
      auto &&retval = std::invoke(f, from_numpy<Args>(args)...);
      return copy_rvalue(to_numpy(FWD(retval)));
    }
  };
}

template <CuriouslyRecurring<ComponentRef_base> D, typename F, typename R, std::constructible_from<component_t<D> &> T,
          typename... Args>
auto normalize_impl(F &&f, R (*)(T, Args...)) {
  return [f = FWD(f)](D &component_ref, to_numpy_t<Args> &...args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, *component_ref, from_numpy<Args>(args)...);
    } else {
      auto &&retval = std::invoke(f, *component_ref, from_numpy<Args>(args)...);
      return copy_rvalue(to_numpy(FWD(retval)));
    }
  };
}

template <CuriouslyRecurring<ComponentRef_base> D, typename F, typename R, std::constructible_from<component_t<D> &> T,
          typename... Args>
auto normalize_impl(F &&f, R (*)(T, arena::Environment &, Args...)) {
  return [f = FWD(f)](D &component_ref, to_numpy_t<Args> &...args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, *component_ref, component_ref.environment(), from_numpy<Args>(args)...);
    } else {
      auto &&retval = std::invoke(f, *component_ref, component_ref.environment(), from_numpy<Args>(args)...);
      return copy_rvalue(to_numpy(FWD(retval)));
    }
  };
}

template <typename T> auto normalize(const pybind11::class_<T> &, WithSignature auto &&f) {
  return normalize_impl<T>(FWD(f), free_signature_ptr(f));
}

auto normalize(const auto &, WithSignature auto &&f) { return normalize_impl<void>(FWD(f), free_signature_ptr(f)); }

auto normalize(WithSignature auto &&f) { return normalize_impl<void>(FWD(f), free_signature_ptr(f)); }

template <typename F, typename R, typename... Args> auto normalize_box2d_impl(F &&f, R (*)(Args...)) {
  return [f = FWD(f)](Args... args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, to_box2d(args)...);
    } else {
      auto &&retval = std::invoke(f, to_box2d(FWD(args))...);
      return copy_rvalue(from_box2d<R>(FWD(retval)));
    }
  };
}

template <CallableWithSignature Signature_F> auto normalize_box2d(auto &&f) {
  return normalize_box2d_impl(FWD(f), (signature_t<std::decay_t<Signature_F>>)nullptr);
}

template <CallableWithSignature Signature_F, MemberFunction Mf> auto normalize_box2d(Mf &&f) {
  return normalize_box2d_impl(FWD(f), (prepend_args_t<signature_t<std::decay_t<Signature_F>>, class_t<Mf> &>)nullptr);
}

} // namespace detail
