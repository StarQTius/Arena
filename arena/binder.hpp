#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include "python.hpp"
#include "utility.hpp"
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

//! \brief Wrap a callable `f` into a unit adaptator function in order to bind it more easily
//! This function convert the parameters from scalar and NumPy objects to quantities and quantity vectors before passing
//! them to the wrapped function (and vice-versa for the return value).
auto w_units(auto &&f) {
  auto impl_for_function = [&]<typename R, typename... Args>(std::function<R(Args...)> *) {
    return [f = FWD(f)](decltype(to_numpy(std::declval<Args &>()))... args) {
      if constexpr (std::is_void_v<R>) {
        std::invoke(f, from_numpy<Args>(FWD(args))...);
      } else {
        return to_numpy(std::invoke(f, from_numpy<Args>(FWD(args))...));
      }
    };
  };

  auto impl_for_member_function = [&]<typename R, typename... Args, typename C>(R(C::*)(Args...)) {
    return impl_for_function((std::function<R(C &, Args...)> *)nullptr);
  };

  if constexpr (std::is_member_function_pointer_v<std::decay_t<decltype(f)>>) {
    return impl_for_member_function(f);
  } else {
    return impl_for_function((decltype(std::function{f}) *)nullptr);
  }
}

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

//! \brief Define a method for a class binding
template <typename... Ts> decltype(auto) operator|(PybindClass auto &&pybind_class, def_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    pybind_class.def(parameters.name, w_units(std::move(parameters.f)), std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

  return FWD(pybind_class);
}

//! \brief Holds constructor definition data
template <typename F, typename... Ts> struct ctor_t {
  F f;
  std::tuple<Ts...> extras;
};

//! \brief Define a constructor for a class binding
template <typename... Ts> decltype(auto) operator|(PybindClass auto &&pybind_class, ctor_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    pybind_class.def(pybind11::init(w_units(std::move(parameters.f))), std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

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
    pybind_class.def_property(parameters.name, w_units(std::move(parameters.read)),
                              w_units(std::move(parameters.write)), std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 2>{});

  return FWD(pybind_class);
}

//! \brief Define a method `name` bound to `f` for a class binding
auto def(const char *name, Invocable auto &&f, auto &&...extras) {
  return def_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{
      .name = name, .f = FWD(f), .extras = {FWD(extras)...}};
}

//! \brief Define a constructor bound to `f` for a class binding
auto ctor(Invocable auto &&f, auto &&...extras) {
  return ctor_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{.f = FWD(f), .extras = {FWD(extras)...}};
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
    return property(name, std::move(read), noop<C &, pybind11::object>, FWD(extras)...);
  }
}
