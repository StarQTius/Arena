#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

#include <arena/environment.hpp>

namespace arena {

inline pybind11::object get_pycomponent(Environment &environment, entt::entity entity, pybind11::object pytype) {
  return pytype.attr("__get")(environment, entity);
}

inline pybind11::object create_pyentity(Environment &environment, pybind11::object pyentity_data) {
  return pyentity_data.attr("__create")(environment);
}

inline pybind11::object attach_pycomponent(Environment &environment, entt::entity entity,
                                           pybind11::object pycomponent) {
  return pycomponent.attr("__attach")(environment, entity);
}

} // namespace arena
