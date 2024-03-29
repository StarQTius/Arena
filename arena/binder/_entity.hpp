#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>

#include <arena/environment.hpp>
#include <arena/traits/invocable.hpp>

#include "../binder/def.hpp"
#include <forward.hpp>

template <typename T>
concept Entity = requires(arena::Environment e, T x) {
  { e.create(x) } -> std::convertible_to<entt::entity>;
};

namespace kind {

template <typename Entity_T>
auto entity(pybind11::module_ &pymodule, const char *name, arena::WithSignature auto &&create) {
  return pybind11::class_<Entity_T>(pymodule, name) | def("__create", FWD(create));
}

template <Entity Entity_T> auto entity(pybind11::module_ &pymodule, const char *name) {
  auto create = [](const Entity_T &self, arena::Environment &environment) { return environment.create(self); };

  return entity<Entity_T>(pymodule, name, create);
}

} // namespace kind
