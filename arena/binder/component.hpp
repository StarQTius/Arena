#pragma once

#include <arena/environment.hpp>
#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>

#include "../component_ref.hpp"
#include "../traits/invocable.hpp"
#include "def.hpp"
#include "static_def.hpp"
#include <forward.hpp>

namespace kind {

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

} // namespace kind
