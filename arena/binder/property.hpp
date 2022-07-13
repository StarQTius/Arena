#pragma once

#include <pybind11/pybind11.h>

#include "../component_ref.hpp"
#include "../traits/invocable.hpp"
#include "../with_units.hpp"
#include "traits.hpp" // IWYU pragma: keep
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

template <typename T, WithSignature Read_F, WithSignature Write_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<T> &&binding, property_t<Read_F, Write_F, Ts...> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def_property(parameters.name, with_units(std::move(parameters.read)),
                         with_units(std::move(parameters.write)), std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

template <CuriouslyRecurring<ComponentRef_base> D, WithSignature Read_F, WithSignature Write_F, typename... Ts>
requires(!Accepting<Read_F, 0, D &> && !Accepting<Write_F, 0, D &>) decltype(auto)
operator|(pybind11::class_<D> &&binding, property_t<Read_F, Write_F, Ts...> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def_property(parameters.name, with_units(D::make_wrapper(std::move(parameters.read))),
                         with_units(D::make_wrapper(std::move(parameters.write))), std::move(extras)...);
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

template <typename T, WithSignature Read_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<T> &&binding, readonly_property_t<Read_F, Ts...> &&parameters) {
  auto impl = [&](auto &&...extras) {
    binding.def_property_readonly(parameters.name, with_units(std::move(parameters.read)), std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

template <CuriouslyRecurring<ComponentRef_base> D, WithSignature Read_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<D> &&binding,
                         readonly_property_t<Read_F, Ts...> &&parameters) requires(!Accepting<Read_F, 0, D &>) {
  auto impl = [&](auto &&...extras) {
    binding.def_property_readonly(parameters.name, with_units(D::make_wrapper(std::move(parameters.read))),
                                  std::move(extras)...);
  };

  std::apply(impl, std::move(parameters.extras));

  return std::move(binding);
}

} // namespace detail

auto property(const char *name, WithSignature auto &&read, WithSignature auto &&write, auto &&...extras) {
  return detail::property_t{name, FWD(read), FWD(write), std::tuple{FWD(extras)...}};
}

template <WithSignature F> auto property(const char *name, F &&read, auto &&...extras) {
  return detail::readonly_property_t{name, FWD(read), std::tuple{FWD(extras)...}};
}

template <Getter Mf> auto property(const char *name, Mf &&mf, auto &&...extras) {
  auto read = [mf](class_t<Mf> & self) -> const auto & { return (self.*mf)(); };

  return property(name, std::move(read), FWD(extras)...);
}

template <Setter Mf> auto property(const char *name, Mf &&mf, auto &&...extras) {
  auto read = [mf](class_t<Mf> & self) -> const auto & { return (self.*mf)(); };
  auto write = [mf](class_t<Mf> &self, return_t<decltype(read)> rhs) { (self.*mf)() = rhs; };

  return property(name, std::move(read), std::move(write), FWD(extras)...);
}

template <std::copy_constructible T, typename C> auto property(const char *name, T C::*vm, auto &&...extras) {
  auto read = [vm](C & self) -> const auto & { return self.*vm; };
  auto write = [vm](C &self, const T &rhs) { return self.*vm = rhs; };

  return property(name, std::move(read), std::move(write), FWD(extras)...);
}

template <typename Cast_T>
auto box2d_property(const char *name, WithSignature auto &&read, WithSignature auto &&write, auto &&...extras) {
  return property(name, normalize_box2d<Cast_T()>(FWD(read)), normalize_box2d<void(Cast_T)>(FWD(write)),
                  FWD(extras)...);
}

template <typename Cast_T> auto box2d_property(const char *name, WithSignature auto &&read, auto &&...extras) {
  return property(name, normalize_box2d<Cast_T()>(FWD(read)), FWD(extras)...);
}
