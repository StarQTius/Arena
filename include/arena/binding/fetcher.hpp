#pragma once

#include <functional>
#include <memory>

#include <entt/entity/entity.hpp>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

#include <arena/environment.hpp>

namespace arena {

// Observer pointer wrapper for Python (because it seems pybind11 is handling raw pointers in a specific way)
template <typename T> using ObserverPtr = std::unique_ptr<T, decltype([](T *) {})>;

template <typename T> struct WithEnvironment {
  explicit WithEnvironment(Environment &environment, T &value) : environment{environment}, value{value} {}

  std::reference_wrapper<Environment> environment;
  std::reference_wrapper<T> value;
};

inline pybind11::object get_pycomponent(Environment &environment, entt::entity entity, pybind11::object pytype) {
  return pytype.attr("__get")(environment, entity);
}

} // namespace arena
