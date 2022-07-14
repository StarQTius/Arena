#pragma once

#include <tuple>

#include <pybind11/pybind11.h>

#include "../traits/invocable.hpp"
#include "../traits/type.hpp" // IWYU pragma: keep
#include "normalize.hpp"
#include <forward.hpp>

namespace detail {

template <typename F, typename Tuple_T> struct ctor_t {
  F f;
  Tuple_T extras;
};

template <typename F, typename Tuple_T> ctor_t(F, Tuple_T) -> ctor_t<F, Tuple_T>;

template <typename T, WithSignature F, typename Tuple_T>
requires(std::convertible_to<return_t<F>, T> || std::convertible_to<return_t<F>, T *>) decltype(auto)
operator|(pybind11::class_<T> &&binding, ctor_t<F, Tuple_T> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def(pybind11::init(detail::normalize(binding, std::move(parameters.f))), std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

template <typename Tuple_T, typename... Ts> struct choose_ctor_t {
  explicit choose_ctor_t(std::tuple<Ts...> *, Tuple_T &&extras) : extras{std::move(extras)} {};

  Tuple_T extras;
};

template <typename T, typename Tuple_T, typename... Args>
requires ListInitializableFrom<T, Args...>
decltype(auto) operator|(pybind11::class_<T> &&binding, choose_ctor_t<Tuple_T, Args...> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def(pybind11::init(detail::normalize(binding, [](Args... args) { return T{FWD(args)...}; })),
                std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

} // namespace detail

auto ctor(WithSignature auto &&f, auto &&...extras) { return detail::ctor_t{FWD(f), std::tuple{FWD(extras)...}}; }

template <typename... Args> auto ctor(auto &&...extras) {
  return detail::choose_ctor_t{(std::tuple<Args...> *)nullptr, std::tuple{FWD(extras)...}};
}
