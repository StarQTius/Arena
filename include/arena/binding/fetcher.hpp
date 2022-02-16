#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>

#include <arena/concept.hpp>
#include <arena/environment.hpp>

namespace arena {

// Associates Python type names to component types
using Fetcher = std::function<pybind11::object(Environment &, entt::entity)>;
using FetcherMap = std::unordered_map<std::string, Fetcher>;

// Observer pointer wrapper for Python (because it seems pybind11 is handling raw pointers in a specific way)
template <typename T> using ObserverPtr = std::unique_ptr<T, decltype([](T *) {})>;

template <typename T> struct WithEnvironment {
  explicit WithEnvironment(Environment &environment, T &value) : environment{environment}, value{value} {}

  std::reference_wrapper<Environment> environment;
  std::reference_wrapper<T> value;
};

// Convenience function to be passed as callback to register_component
template <typename T> auto get_component(Environment &environment, entt::entity self) {
  return pybind11::cast(dereference_if_needed(environment.registry.get<T>(self)),
                        pybind11::return_value_policy::reference);
}
template <typename T> auto get_component_with_environment(Environment &environment, entt::entity self) {
  return pybind11::cast(WithEnvironment{environment, dereference_if_needed(environment.registry.get<T>(self))});
}

} // namespace arena
