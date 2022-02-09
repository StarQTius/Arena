#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>

#include <arena/concept.hpp>

namespace arena {

// Associates Python type names to component types
using Fetcher = std::function<pybind11::object(entt::registry &, entt::entity)>;
using FetcherMap = std::unordered_map<std::string, Fetcher>;

// Observer pointer wrapper for Python (because it seems pybind11 is handling raw pointers in a specific way)
template <typename T> using ObserverPtr = std::unique_ptr<T, decltype([](T *) {})>;

// Convenience function to be passed as callback to register_component
template <typename T>
requires(!IsUniquePtr<T>) auto get_component(entt::registry &world, entt::entity self) {
  return pybind11::cast(world.get<T>(self));
}
template <IsUniquePtr T> auto get_component(entt::registry &world, entt::entity self) {
  return pybind11::cast(world.get<T>(self).get());
}

} // namespace arena
