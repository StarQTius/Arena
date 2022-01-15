#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

template <typename T> concept IsUniquePtr = requires(T x) {
  {
    std::unique_ptr { std::move(x) }
  }
  ->std::same_as<T>;
};

// Associates Python type names to component types
using Fetcher = std::function<py::object(entt::registry &, entt::entity)>;
using FetcherMap = std::unordered_map<std::string, Fetcher>;

// Observer pointer wrapper for Python (because it seems pybind11 is handling raw pointers in a specific way)
template <typename T> class ObserverPtr {
public:
  ObserverPtr(T *ptr) : m_ptr{ptr} {}

  auto &operator*() { return *m_ptr; }
  const auto &operator*() const { return *m_ptr; }

  auto *operator->() { return m_ptr; }
  const auto *operator->() const { return m_ptr; }

private:
  T *m_ptr;
};

// Convenience function to be passed as callback to register_component
template <typename T> requires(!IsUniquePtr<T>) auto get_component(entt::registry &world, entt::entity self) {
  return py::cast(world.get<T>(self));
}
template <IsUniquePtr T> auto get_component(entt::registry &world, entt::entity self) {
  return py::cast(ObserverPtr{world.get<T>(self).get()});
}
