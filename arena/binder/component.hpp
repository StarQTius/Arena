#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>

#include <arena/environment.hpp>
#include <arena/traits/invocable.hpp>

#include "../component_ref.hpp"
#include "def.hpp"
#include "static_def.hpp"
#include <forward.hpp>

namespace kind {

template <typename Component_T>
auto component(pybind11::module_ &pymodule, const char *name, arena::WithSignature auto &&attach,
               arena::WithSignature auto &&get, arena::WithSignature auto &&has) {
  return pybind11::class_<ComponentRef<Component_T>>(pymodule, name) | def("__attach", FWD(attach)) |
         static_def("__get", FWD(get), pybind11::return_value_policy::reference_internal) |
         static_def("__has", FWD(has)) |
         property("environment", &ComponentRef<Component_T>::environment,
                  pybind11::return_value_policy::reference_internal);
}

template <typename Component_T> auto component(pybind11::module_ &pymodule, const char *name) {
  auto attach = [](ComponentRef<Component_T> &self, arena::Environment &environment, entt::entity entity) {
    self.attach(environment, entity);
  };
  auto get = [](arena::Environment &environment, entt::entity entity) {
    return ComponentRef<Component_T>{environment, entity};
  };
  auto has = [](arena::Environment &environment, entt::entity entity) {
    return environment.all_of<Component_T>(entity);
  };

  return component<Component_T>(pymodule, name, attach, get, has);
}

} // namespace kind
