#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>

#include <arena/environment.hpp>
#include <arena/traits/invocable.hpp>

#include "../component_ref.hpp"
#include <forward.hpp>

namespace kind {

template <typename Component_T>
auto internal_component(pybind11::module_ &pymodule, const char *name, arena::WithSignature auto &&get) {
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
