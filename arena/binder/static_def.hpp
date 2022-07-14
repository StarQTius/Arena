#pragma once

#include "normalize.hpp"
#include "traits.hpp" // IWYU pragma: keep
#include <forward.hpp>

namespace detail {

template <typename F, typename Tuple_T> struct static_def_t {
  const char *name;
  F f;
  Tuple_T extras;
};

template <typename F, typename Tuple_T> static_def_t(const char *, F, Tuple_T) -> static_def_t<F, Tuple_T>;

template <WithSignature F, typename Tuple_T>
decltype(auto) operator|(Binding auto &&binding, static_def_t<F, Tuple_T> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def_static(parameters.name, detail::normalize(std::move(parameters.f)), std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return FWD(binding);
}

} // namespace detail

auto static_def(const char *name, WithSignature auto &&f, auto &&...extras) {
  return detail::static_def_t{name, FWD(f), std::tuple{FWD(extras)...}};
}
