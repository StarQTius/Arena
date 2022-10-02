#pragma once

#include <pybind11/pybind11.h>
#include <tl/expected.hpp>

#include <arena/environment.hpp>
#include <arena/traits/crtp.hpp>
#include <arena/traits/invocable.hpp>
#include <arena/traits/template.hpp>

#include "../box2d.hpp"
#include "../component_ref.hpp"
#include "../python.hpp"
#include "../utility.hpp"
#include <forward.hpp>

namespace detail {

template <typename T> using python_arg_t = to_numpy_t<T> &;

template <typename T> remove_rvalue_reference_t<T> forward_from_python(auto &x) {
  return from_numpy<std::remove_cvref_t<T>>(x);
}

template <typename, typename F, typename R, typename... Args> auto normalize_impl(F &&f, R (*)(Args...)) {
  return [f = FWD(f)](Args... args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, FWD(args)...);
    } else {
      auto &&retval = std::invoke(f, FWD(args)...);

      if constexpr (arena::InstanceOf<R, tl::expected>) {
        if constexpr (std::is_void_v<typename R::value_type>) {
          FWD(retval).or_else(pyraise);
        } else {
          return copy_rvalue(FWD(retval).or_else(pyraise).value());
        }
      } else {
        return copy_rvalue(FWD(retval));
      }
    }
  };
}

template <arena::CuriouslyRecurring<ComponentRef_base> D, typename F, typename R,
          std::constructible_from<component_t<D> &> T, typename... Args>
auto normalize_impl(F &&f, R (*)(T, Args...)) {
  return [f = FWD(f)](D &component_ref, Args... args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, *component_ref, FWD(args)...);
    } else {
      auto &&retval = std::invoke(f, *component_ref, FWD(args)...);

      if constexpr (arena::InstanceOf<R, tl::expected>) {
        if constexpr (std::is_void_v<typename R::value_type>) {
          FWD(retval).or_else(pyraise);
        } else {
          return copy_rvalue(FWD(retval).or_else(pyraise).value());
        }
      } else {
        return copy_rvalue(FWD(retval));
      }
    }
  };
}

template <arena::CuriouslyRecurring<ComponentRef_base> D, typename F, typename R,
          std::constructible_from<component_t<D> &> T, typename... Args>
auto normalize_impl(F &&f, R (*)(T, arena::Environment &, Args...)) {
  return [f = FWD(f)](D &component_ref, Args... args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, *component_ref, component_ref.environment(), FWD(args)...);
    } else {
      auto &&retval = std::invoke(f, *component_ref, component_ref.environment(), FWD(args)...);

      if constexpr (arena::InstanceOf<R, tl::expected>) {
        if constexpr (std::is_void_v<typename R::value_type>) {
          FWD(retval).or_else(pyraise);
        } else {
          return copy_rvalue(FWD(retval).or_else(pyraise).value());
        }
      } else {
        return copy_rvalue(FWD(retval));
      }
    }
  };
}

template <typename T> auto normalize(const pybind11::class_<T> &, arena::WithSignature auto &&f) {
  return normalize_impl<T>(FWD(f), arena::free_signature_p(f));
}

auto normalize(const auto &, arena::WithSignature auto &&f) {
  return normalize_impl<void>(FWD(f), arena::free_signature_p(f));
}

auto normalize(arena::WithSignature auto &&f) { return normalize_impl<void>(FWD(f), arena::free_signature_p(f)); }

template <typename F, typename R, typename... Args> auto normalize_box2d_impl(F &&f, R (*)(Args...)) {
  return [f = FWD(f)](Args... args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, to_box2d(FWD(args))...);
    } else {
      auto &&retval = std::invoke(f, to_box2d(FWD(args))...);
      return copy_rvalue(from_box2d<R>(FWD(retval)));
    }
  };
}

template <arena::CallableWithSignature Signature_F> auto normalize_box2d(auto &&f) {
  return normalize_box2d_impl(FWD(f), (arena::signature_t<std::decay_t<Signature_F>>)nullptr);
}

template <arena::CallableWithSignature Signature_F, arena::MemberFunction Mf> auto normalize_box2d(Mf &&f) {
  return normalize_box2d_impl(
      FWD(f), (arena::prepend_args_t<arena::signature_t<std::decay_t<Signature_F>>, arena::class_t<Mf> &>)nullptr);
}

} // namespace detail
