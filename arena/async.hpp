#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/pytypes.h>

#include <arena/environment.hpp>

class PyAsyncSystem {
public:
  explicit PyAsyncSystem(arena::Environment &, entt::entity, const pybind11::function &);
  explicit PyAsyncSystem(const pybind11::function &);

  void prepare_coroutine(arena::Environment &, entt::entity);
  bool is_ready() const;
  pybind11::object next();

private:
  pybind11::object m_pycoroutine;
};
