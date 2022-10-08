#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/pytypes.h>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <arena/environment.hpp>

#include "../async.hpp"

struct wait_next_t {};

class PyHost {
public:
  explicit PyHost(const pybind11::function &);
  explicit PyHost(pybind11::function &&);

  void invoke(arena::Environment &, entt::entity);

private:
  PyAsyncSystem m_pyasyncsystem;
};

template <> struct arena_component_info<PyHost> {};
