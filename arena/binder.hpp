#pragma once

#include <variant>

#include <box2d/b2_body.h>
#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>

#include <arena/environment.hpp>

#include "binder/def.hpp"
#include "binder/traits.hpp" // IWYU pragma: keep
#include "component_ref.hpp" // IWYU pragma: export
#include "traits/invocable.hpp"
#include "utility.hpp"
#include "with_units.hpp"
#include <forward.hpp>

//! \brief Add a docstring to a class binding
template <typename... Ts> decltype(auto) operator|(PybindClass auto &&pybind_class, std::string_view doc) {
  pybind_class.doc() = doc;

  return FWD(pybind_class);
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
template <typename... Ts>
decltype(auto) operator|(PybindClass auto &&pybind_class, property_t<Ts...> &&parameters) requires(
    !std::same_as<decltype(pybind_class), pybind11::class_<InternalComponentRef<b2Body *>> &&>) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    pybind_class.def_property(parameters.name, with_units(std::move(parameters.read)),
                              with_units(std::move(parameters.write)), std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 2>{});

  return FWD(pybind_class);
}

template <typename D, typename Read_F, typename Write_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<D> &&binding, property_t<Read_F, Write_F, Ts...> &&parameters) requires(
    std::derived_from<D, ComponentRef_base<D>> &&
    !std::constructible_from<D, typename signature_info<Read_F>::template free_arg_t<0>>) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    binding.def_property(parameters.name, with_units(D::make_wrapper(std::move(parameters.read))),
                         with_units(D::make_wrapper(std::move(parameters.write))),
                         std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts)>{});

  return std::move(binding);
}

template <typename Read_F, typename... Ts> struct readonly_property_t {
  const char *name;
  Read_F read;
  std::tuple<Ts...> extras;
};

template <typename... Ts>
decltype(auto) operator|(PybindClass auto &&pybind_class, readonly_property_t<Ts...> &&parameters) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    pybind_class.def_property_readonly(parameters.name, with_units(std::move(parameters.read)),
                                       std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

  return FWD(pybind_class);
}

template <typename D, typename Read_F, typename... Ts>
decltype(auto) operator|(pybind11::class_<D> &&binding, readonly_property_t<Read_F, Ts...> &&parameters) requires(
    std::derived_from<D, ComponentRef_base<D>> &&
    !std::constructible_from<D, typename signature_info<Read_F>::template free_arg_t<0>>) {
  auto impl = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    binding.def_property_readonly(parameters.name, with_units(D::make_wrapper(std::move(parameters.read))),
                                  std::move(std::get<Is>(parameters.extras))...);
  };

  impl(std::make_index_sequence<sizeof...(Ts) - 1>{});

  return std::move(binding);
}

auto static_def(const char *name, WithSignature auto &&f, auto &&...extras) {
  return static_def_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{
      .name = name, .f = FWD(f), .extras = {FWD(extras)...}};
}

//! \brief Define a constructor bound to `f` for a class binding
auto ctor(WithSignature auto &&f, auto &&...extras) {
  return ctor_t<std::decay_t<decltype(f)>, std::decay_t<decltype(extras)>...>{.f = FWD(f), .extras = {FWD(extras)...}};
}

template <typename... Args> auto ctor(auto &&...extras) {
  return choose_ctor_t<std::tuple<Args...>, std::decay_t<decltype(extras)>...>{.args = (std::tuple<Args...> *)nullptr,
                                                                               .extras = std::tuple{FWD(extras)...}};
}

//! \brief Define a property `name` for a class binding whose getter is bound to `read` and is setter is bound to
//! `write`
auto property(const char *name, WithSignature auto &&read, WithSignature auto &&write, auto &&...extras) {
  return property_t<std::decay_t<decltype(read)>, std::decay_t<decltype(write)>, std::decay_t<decltype(extras)>...>{
      .name = name, .read = FWD(read), .write = FWD(write), .extras = {FWD(extras)...}};
}

auto property(const char *name, WithSignature auto &&read, auto &&...extras) {
  return readonly_property_t<std::decay_t<decltype(read)>, std::decay_t<decltype(extras)>...>{
      .name = name, .read = FWD(read), .extras = {FWD(extras)...}};
}

auto property(const char *name, Getter auto *fm, auto &&...extras) {
  using fm_t = decltype(fm);
  using class_t = get_class_t<fm_t>;
  using fm_return_t = std::invoke_result<fm_t, class_t &>;

  auto read = [fm](class_t & self) -> const auto & { return (self.*fm)(); };

  if constexpr (Setter<fm_t>) {
    auto write = [fm](class_t &self, const fm_return_t &rhs) { (self.*fm)() = rhs; };
    return property(name, std::move(read), std::move(write), FWD(extras)...);
  } else {
    return property(name, std::move(read), FWD(extras)...);
  }
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

namespace kind {

template <typename Entity_T> auto entity(pybind11::module_ &pymodule, const char *name, WithSignature auto &&create) {
  return pybind11::class_<Entity_T>(pymodule, name) | def("__create", FWD(create));
}

template <typename Entity_T> auto entity(pybind11::module_ &pymodule, const char *name) {
  auto create = [](const Entity_T &self, arena::Environment &environment) { return environment.create(self); };

  return entity<Entity_T>(pymodule, name, create);
}

template <typename Component_T>
auto component(pybind11::module_ &pymodule, const char *name, WithSignature auto &&attach, WithSignature auto &&get) {
  return pybind11::class_<ComponentRef<Component_T>>(pymodule, name) | def("__attach", FWD(attach)) |
         static_def("__get", FWD(get), pybind11::return_value_policy::reference_internal);
}

template <typename Component_T> auto component(pybind11::module_ &pymodule, const char *name) {
  auto attach = [](ComponentRef<Component_T> &self, arena::Environment &environment, entt::entity entity) {
    self.attach(environment, entity);
  };
  auto get = [](arena::Environment &environment, entt::entity entity) {
    return ComponentRef<Component_T>{environment, entity};
  };

  return component<Component_T>(pymodule, name, attach, get);
}

template <typename Component_T>
auto internal_component(pybind11::module_ &pymodule, const char *name, WithSignature auto &&get) {
  return pybind11::class_<InternalComponentRef<Component_T *>>(pymodule, name) |
         static_def("__get", FWD(get), pybind11::return_value_policy::reference_internal);
}

template <typename Component_T> auto internal_component(pybind11::module_ &pymodule, const char *name) {
  auto get = [](arena::Environment &environment, entt::entity entity) {
    return InternalComponentRef<Component_T *>{environment, entity};
  };

  return internal_component<Component_T>(pymodule, name, get);
}

} // namespace kind
