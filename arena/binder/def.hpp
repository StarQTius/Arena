#pragma once

#include <arena/traits/invocable.hpp>
#include <arena/traits/template.hpp>

#include "normalize.hpp"
#include "traits.hpp"
#include <forward.hpp>

namespace detail {

template <typename F, typename Tuple_T> struct def_t {
  const char *name;
  F f;
  Tuple_T extras;
};

template <typename F, typename Tuple_T> def_t(const char *, F, Tuple_T) -> def_t<F, Tuple_T>;

template <arena::WithSignature F, arena::InstanceOf<std::tuple> Tuple_T>
decltype(auto) operator|(Binding auto &&binding, def_t<F, Tuple_T> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def(parameters.name, detail::normalize(binding, std::move(parameters.f)), std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return FWD(binding);
}

} // namespace detail

auto def(const char *name, arena::WithSignature auto &&f, auto &&...extras) {
  return detail::def_t{name, FWD(f), std::tuple{FWD(extras)...}};
}
