#pragma once

#include <pybind11/pybind11.h>

#include "../component_ref.hpp"
#include "../traits/invocable.hpp"
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

template <WithSignature F, InstanceOf<std::tuple> Tuple_T>
decltype(auto) operator|(BindableTo auto &&binding, def_t<F, Tuple_T> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    binding.def(parameters.name, with_units(std::move(parameters.f)), std::get<Is>(std::move(parameters.extras))...);
  };

  impl(std::make_index_sequence<std::tuple_size_v<Tuple_T>>{});

  return FWD(binding);
}

template <CuriouslyRecurring<ComponentRef_base> D, WithSignature F, InstanceOf<std::tuple> Tuple_T>
requires Accepting < free_signature_t<F>,
0, component_t<D> & > decltype(auto) operator|(pybind11::class_<D> &&binding, def_t<F, Tuple_T> &&parameters) {
  auto impl = [&]<std::size_t... Is, typename... Args>(std::index_sequence<Is...>) {
    binding.def(parameters.name, with_units(D::make_wrapper(std::move(parameters.f))),
                std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<std::tuple_size_v<Tuple_T>>{});

  return std::move(binding);
}

} // namespace detail

auto def(const char *name, WithSignature auto &&f, auto &&...extras) {
  return detail::def_t{name, FWD(f), std::tuple{FWD(extras)...}};
}
