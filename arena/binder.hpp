#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include "utility.hpp"
#include "with_units.hpp"
#include <forward.hpp>

//! \brief Satisfied when `T` can be called
template <typename T>
concept Invocable = requires(T x) {
  std::function{x};
}
|| std::is_member_function_pointer_v<T>;

template <typename Mf>
concept Getter = !std::is_void_v<std::invoke_result<Mf, get_class_t<Mf> &>>;

template <typename Mf>
concept Setter = Getter<Mf> && std::assignable_from < std::invoke_result < Mf,
        get_class_t<Mf>
& >, std::invoke_result<Mf, get_class_t<Mf> &> > ;

//! \brief Satisfied when `T` is an instance of `pybind11:class_`
template <typename T>
concept PybindClass = requires(T x) {
  {
    pybind11::class_ { x }
    } -> std::convertible_to<T>;
};

//! \brief Satisfied when `T` is a binding to which function can be bound to with a function `def`
template <typename T>
concept BindableTo = requires(T x) {
  {
    x.def("", [](T &) {})
    } -> std::convertible_to<T>;
};

//! \brief Add a docstring to a class binding
template <typename... Ts> decltype(auto) operator|(PybindClass auto &&pybind_class, std::string_view doc) {
  pybind_class.doc() = doc;

  return FWD(pybind_class);
}

//! \brief Holds method definition data
template <typename F, typename... Ts> struct def_t {
  const char *name;
  F f;
  std::tuple<Ts...> extras;
};

//! \brief Define a method or function for a binding with a `def` member function
template <typename... Ts> decltype(auto) operator|(BindableTo auto &&binding, def_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    binding.def(parameters.name, with_units(std::move(parameters.f)), std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

  return FWD(binding);
}

template <typename F, typename... Ts> struct static_def_t {
  const char *name;
  F f;
  std::tuple<Ts...> extras;
};

template <typename... Ts> decltype(auto) operator|(BindableTo auto &&binding, static_def_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    binding.def_static(parameters.name, with_units(std::move(parameters.f)),
                       std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

  return FWD(binding);
}

//! \brief Holds constructor definition data
template <typename F, typename... Ts> struct ctor_t {
  F f;
  std::tuple<Ts...> extras;
};

//! \brief Define a constructor for a class binding
template <typename... Ts> decltype(auto) operator|(PybindClass auto &&pybind_class, ctor_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    pybind_class.def(pybind11::init(with_units(std::move(parameters.f))),
                     std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

  return FWD(pybind_class);
}

template <typename Tuple_T, typename... Args> struct choose_ctor_t {
  Tuple_T *args;
  std::tuple<Args...> extras;
};

template <typename... Args, typename... Ts>
decltype(auto) operator|(PybindClass auto &&pybind_class, choose_ctor_t<std::tuple<Args...>, Ts...> &&parameters) {
  auto impl = [&]<typename C, auto... Is>(pybind11::class_<C> &, std::index_sequence<Is...>) {
    pybind_class.def(pybind11::init(with_units([](Args... args) { return C{FWD(args)...}; })),
                     std::move(std::get<Is>(parameters.extras))...);
  };

  impl(pybind_class, std::make_index_sequence<sizeof...(Ts)>{});

  return FWD(pybind_class);
}

//! \brief Holds property definition data
template <typename Read_F, typename Write_F, typename... Ts> struct property_t {
  const char *name;
  Read_F read;
  Write_F write;
  std::tuple<Ts...> extras;
};

//! \brief Define a property for a class binding
template <typename... Ts> decltype(auto) operator|(PybindClass auto &&pybind_class, property_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    pybind_class.def_property(parameters.name, with_units(std::move(parameters.read)),
                              with_units(std::move(parameters.write)), std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 2>{});

  return FWD(pybind_class);
}

//! \brief Define a method `name` bound to `f` for a class binding
auto def(const char *name, Invocable auto &&f, auto &&...extras) {
  return def_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{
      .name = name, .f = FWD(f), .extras = {FWD(extras)...}};
}

auto static_def(const char *name, Invocable auto &&f, auto &&...extras) {
  return def_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{
      .name = name, .f = FWD(f), .extras = {FWD(extras)...}};
}

//! \brief Define a constructor bound to `f` for a class binding
auto ctor(Invocable auto &&f, auto &&...extras) {
  return ctor_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{.f = FWD(f), .extras = {FWD(extras)...}};
}

template <typename... Args> auto ctor(auto &&...extras) {
  return choose_ctor_t<std::tuple<Args...>, std::decay_t<decltype(extras)>...>{.args = (std::tuple<Args...> *)nullptr,
                                                                               .extras = std::tuple{FWD(extras)...}};
}

//! \brief Define a property `name` for a class binding whose getter is bound to `read` and is setter is bound to
//! `write`
auto property(const char *name, Invocable auto &&read, Invocable auto &&write, auto &&...extras) {
  return property_t<std::decay_t<decltype(read)>, std::decay_t<decltype(write)>, std::decay_t<decltype(extras)>...>{
      .name = name, .read = FWD(read), .write = FWD(write), .extras = {FWD(extras)...}};
}

template <typename C> auto property(const char *name, auto C::*fm, auto &&...extras) requires Getter<decltype(fm)> {
  using fm_t = decltype(fm);
  using fm_return_t = std::invoke_result<fm_t, C &>;

  auto read = [fm](C &self) -> decltype(auto) { return (self.*fm)(); };

  if constexpr (Setter<fm_t>) {
    auto write = [fm](C &self, const fm_return_t &rhs) { (self.*fm)() = rhs; };
    return property(name, std::move(read), std::move(write), FWD(extras)...);
  } else {
    return property(
        name, std::move(read), [](pybind11::object) {}, FWD(extras)...);
  }
}

template <typename Cast_T>
auto box2d_property(const char *name, Invocable auto &&read, Invocable auto &&write, auto &&...extras) {
  return property(name, normalize_box2d<Cast_T()>(FWD(read)), normalize_box2d<void(Cast_T)>(FWD(write)),
                  FWD(extras)...);
}

template <typename Cast_T> auto box2d_property(const char *name, Invocable auto &&read, auto &&...extras) {
  return property(
      name, normalize_box2d<Cast_T()>(FWD(read)), [](pybind11::object) {}, FWD(extras)...);
}
