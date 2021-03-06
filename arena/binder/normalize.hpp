#pragma once

#include <pybind11/pybind11.h>
#include <tl/expected.hpp>

#include <arena/environment.hpp>

#include "../box2d.hpp"
#include "../component_ref.hpp"
#include "../python.hpp"
#include "../traits/crtp.hpp"
#include "../traits/invocable.hpp"
#include "../traits/template.hpp"
#include "../utility.hpp"
#include <forward.hpp>

namespace detail {

template<typename T>
using python_arg_t = to_numpy_t<T> &;

template<typename T>
remove_rvalue_reference_t<T> forward_from_python(auto &x) {
  return from_numpy<std::remove_cvref_t<T>>(x);
}

template <typename, typename F, typename R, typename... Args> auto normalize_impl(F &&f, R (*)(Args...)) {
  return [f = FWD(f)](python_arg_t<Args> ...args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, forward_from_python<Args>(args)...);
    } else {
      auto &&retval = std::invoke(f, forward_from_python<Args>(FWD(args))...);

      if constexpr (InstanceOf<R, tl::expected>) {
        if constexpr (std::is_void_v<typename R::value_type>) {
          FWD(retval).or_else(pyraise);
        } else {
          return copy_rvalue(to_numpy(FWD(retval).or_else(pyraise).value()));
        }
      } else {
        return copy_rvalue(to_numpy(FWD(retval)));
      }
    }
  };
}

template <CuriouslyRecurring<ComponentRef_base> D, typename F, typename R, std::constructible_from<component_t<D> &> T,
          typename... Args>
auto normalize_impl(F &&f, R (*)(T, Args...)) {
  return [f = FWD(f)](D &component_ref, python_arg_t<Args> ...args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, *component_ref, forward_from_python<Args>(FWD(args))...);
    } else {
      auto &&retval = std::invoke(f, *component_ref, forward_from_python<Args>(FWD(args))...);

      if constexpr (InstanceOf<R, tl::expected>) {
        if constexpr (std::is_void_v<typename R::value_type>) {
          FWD(retval).or_else(pyraise);
        } else {
          return copy_rvalue(to_numpy(FWD(retval).or_else(pyraise).value()));
        }
      } else {
        return copy_rvalue(to_numpy(FWD(retval)));
      }
    }
  };
}

template <CuriouslyRecurring<ComponentRef_base> D, typename F, typename R, std::constructible_from<component_t<D> &> T,
          typename... Args>
auto normalize_impl(F &&f, R (*)(T, arena::Environment &, Args...)) {
  return [f = FWD(f)](D &component_ref, python_arg_t<Args> ...args) -> decltype(auto) {
    if constexpr (std::is_void_v<R>) {
      std::invoke(f, *component_ref, component_ref.environment(), forward_from_python<Args>(FWD(args))...);
    } else {
      auto &&retval = std::invoke(f, *component_ref, component_ref.environment(), forward_from_python<Args>(FWD(args))...);

      if constexpr (InstanceOf<R, tl::expected>) {
        if constexpr (std::is_void_v<typename R::value_type>) {
          FWD(retval).or_else(pyraise);
        } else {
          return copy_rvalue(to_numpy(FWD(retval).or_else(pyraise).value()));
        }
      } else {
        return copy_rvalue(to_numpy(FWD(retval)));
      }
    }
  };
}

template <typename T> auto normalize(const pybind11::class_<T> &, WithSignature auto &&f) {
  return normalize_impl<T>(FWD(f), free_signature_p(f));
}

auto normalize(const auto &, WithSignature auto &&f) { return normalize_impl<void>(FWD(f), free_signature_p(f)); }

auto normalize(WithSignature auto &&f) { return normalize_impl<void>(FWD(f), free_signature_p(f)); }

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
