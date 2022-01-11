#pragma once

#include <string>
#include <utility>
#include <vector>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Associates Python type names to component types
using Fetcher = std::function<py::object(entt::registry &, entt::entity)>;
using FetcherMap = std::unordered_map<std::string, Fetcher>;

namespace component {

// Holds a Python invocable to be called on the entity components
class PyHost {
public:
  // Copy initialize the stored callback
  explicit PyHost(const py::function &pycallback) : m_pytypes{get_annotations(pycallback)}, m_pycallback{pycallback} {}

  // Move initialize the stored callback
  explicit PyHost(py::function &&pycallback)
      : m_pytypes{get_annotations(pycallback)}, m_pycallback{std::move(pycallback)} {}

  // Call the stored invocable on the entity component
  void invoke(entt::registry &, entt::entity, const FetcherMap &);

private:
  // Get the annotations (type hints) of a Python function arguments
  static std::vector<std::string> get_annotations(const py::function &);

  std::vector<std::string> m_pytypes;
  py::function m_pycallback;
};

} // namespace component

// Convenience function to be passed as callback to register_component
template <typename T> auto get_component(entt::registry &world, entt::entity self) {
  return py::cast(world.get<T>(self));
}
