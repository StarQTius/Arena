#pragma once

#include <pybind11/pybind11.h>

#include <arena/binding/fetcher.hpp>

#include "../with_units.hpp"
#include "traits.hpp" // IWYU pragma: keep
#include <forward.hpp>

namespace detail {

template <typename F, typename Tuple_T> struct def_t {
  const char *name;
  F f;
  Tuple_T extras;
};

template <typename F, typename Tuple_T> def_t(const char *, F, Tuple_T) -> def_t<F, Tuple_T>;

template <typename T, typename Tuple_T>
decltype(auto) operator|(BindableTo auto &&binding, def_t<T, Tuple_T> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    binding.def(parameters.name, with_units(std::move(parameters.f)), std::get<Is>(std::move(parameters.extras))...);
  };

  impl(std::make_index_sequence<std::tuple_size_v<Tuple_T>>{});

  return FWD(binding);
}

template <typename D, typename F, typename Tuple_T>
decltype(auto) operator|(pybind11::class_<D> &&binding, def_t<F, Tuple_T> &&parameters) requires(
    std::derived_from<D, arena::ComponentRef_base<D>> &&
    !std::same_as<std::remove_cvref_t<typename signature_info<F>::template free_arg_t<0>>, D>) {
  auto impl = [&]<std::size_t... Is, typename... Args>(std::index_sequence<Is...>) {
    binding.def(parameters.name, with_units(D::make_wrapper(std::move(parameters.f))),
                std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<std::tuple_size_v<Tuple_T>>{});

  return std::move(binding);
}

} // namespace detail

auto def(const char *name, Invocable auto &&f, auto &&...extras) {
  return detail::def_t{name, FWD(f), std::tuple{FWD(extras)...}};
}
