#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <vector>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/pybind11.h>

#include <arena/binding/fetcher.hpp>

namespace arena {
namespace component {

// Holds a Python invocable to be called on the entity components
class PyHost {
public:
  // Copy initialize the stored callback
  explicit PyHost(const pybind11::function &pycallback)
      : m_pytypes{get_annotations(pycallback)}, m_pycallback{pycallback} {}

  // Move initialize the stored callback
  explicit PyHost(pybind11::function &&pycallback)
      : m_pytypes{get_annotations(pycallback)}, m_pycallback{std::move(pycallback)} {}

  // Call the stored invocable on the entity component
  void invoke(entt::registry &, entt::entity, const FetcherMap &);

private:
  // Get the annotations (type hints) of a Python function arguments
  static std::vector<std::string> get_annotations(const pybind11::function &);

  std::vector<std::string> m_pytypes;
  pybind11::function m_pycallback;
};

} // namespace component
} // namespace arena
