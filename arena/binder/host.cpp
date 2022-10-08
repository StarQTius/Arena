#include "host.hpp"

#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <arena/environment.hpp>

#include "../async.hpp"

namespace py = pybind11;

using namespace arena;

PyHost::PyHost(const pybind11::function &pycallback) : m_pyasyncsystem{pycallback} {}

PyHost::PyHost(pybind11::function &&pycallback) : m_pyasyncsystem{std::move(pycallback)} {}

void PyHost::invoke(arena::Environment &environment, entt::entity entity) {
  if (!m_pyasyncsystem.is_ready())
    m_pyasyncsystem.prepare_coroutine(environment, entity);
  m_pyasyncsystem.next();
}
