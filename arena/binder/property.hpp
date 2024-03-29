#pragma once

#include <pybind11/pybind11.h>

#include <arena/traits/invocable.hpp>

#include "normalize.hpp"
#include "traits.hpp"
#include <forward.hpp>

namespace detail {

template <typename Read_F, typename Write_F, typename Tuple_T> struct property_t {
  const char *name;
  Read_F read;
  Write_F write;
  Tuple_T extras;
};

template <typename Read_F, typename Write_F, typename Tuple_T>
property_t(const char *, Read_F, Write_F, Tuple_T) -> property_t<Read_F, Write_F, Tuple_T>;

template <typename T, arena::WithSignature Read_F, arena::WithSignature Write_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<T> &&binding, property_t<Read_F, Write_F, Ts...> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def_property(parameters.name, detail::normalize(binding, std::move(parameters.read)),
                         detail::normalize(binding, std::move(parameters.write)), std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

template <typename Read_F, typename Tuple_T> struct readonly_property_t {
  const char *name;
  Read_F read;
  Tuple_T extras;
};

template <typename Read_F, typename Tuple_T>
readonly_property_t(const char *, Read_F, Tuple_T) -> readonly_property_t<Read_F, Tuple_T>;

template <typename T, arena::WithSignature Read_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<T> &&binding, readonly_property_t<Read_F, Ts...> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def_property_readonly(parameters.name, detail::normalize(binding, std::move(parameters.read)),
                                  std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

} // namespace detail

auto property(const char *name, arena::WithSignature auto &&read, arena::WithSignature auto &&write, auto &&...extras) {
  return detail::property_t{name, FWD(read), FWD(write), std::tuple{FWD(extras)...}};
}

template <arena::WithSignature F> auto property(const char *name, F &&read, auto &&...extras) {
  return detail::readonly_property_t{name, FWD(read), std::tuple{FWD(extras)...}};
}

template <Getter Mf> auto property(const char *name, Mf &&mf, auto &&...extras) {
  auto read = [mf](arena::class_t<Mf> &self) -> decltype(auto) { return (self.*mf)(); };

  return property(name, std::move(read), FWD(extras)...);
}

template <Setter Mf> auto property(const char *name, Mf &&mf, auto &&...extras) {
  auto read = [mf](arena::class_t<Mf> &self) -> decltype(auto) { return (self.*mf)(); };
  auto write = [mf](arena::class_t<Mf> &self, arena::return_t<decltype(read)> rhs) { (self.*mf)() = rhs; };

  return property(name, std::move(read), std::move(write), FWD(extras)...);
}

template <std::copy_constructible T, typename C> auto property(const char *name, T C::*vm, auto &&...extras) {
  auto read = [vm](C &self) -> decltype(auto) { return self.*vm; };
  auto write = [vm](C &self, const T &rhs) { return self.*vm = rhs; };

  return property(name, std::move(read), std::move(write), FWD(extras)...);
}

template <typename Cast_T>
auto box2d_property(const char *name, arena::WithSignature auto &&read, arena::WithSignature auto &&write,
                    auto &&...extras) {
  return property(name, detail::normalize_box2d<Cast_T()>(FWD(read)), detail::normalize_box2d<void(Cast_T)>(FWD(write)),
                  FWD(extras)...);
}

template <typename Cast_T> auto box2d_property(const char *name, arena::WithSignature auto &&read, auto &&...extras) {
  return property(name, detail::normalize_box2d<Cast_T()>(FWD(read)), FWD(extras)...);
}
