#include "async.hpp"

#include <ltl/Range/Map.h>
#include <ltl/Range/Taker.h>
#include <pybind11/cast.h>
#include <pybind11/eval.h>
#include <pybind11/pybind11.h>

#include "binder/fetcher.hpp"
#include <forward.hpp>

namespace py = pybind11;

namespace {

auto provide_components_to_async(arena::Environment &environment, entt::entity entity,
                                 const py::function &async_pyfunction) {
  using ltl::map, ltl::drop_n;

  auto inspect_pymodule = py::module_::import("inspect");
  auto getfullargspec_pyfunction = inspect_pymodule.attr("getfullargspec");
  auto ismethod_pyfunction = inspect_pymodule.attr("ismethod");

  auto pyargspec = getfullargspec_pyfunction(async_pyfunction);
  auto pyargs = pyargspec.attr("args");
  auto pyannotations_by_arg = pyargspec.attr("annotations");

  auto get_arg_annotation = [&](const py::handle &a) {
    if (!pyannotations_by_arg.contains(a))
      throw std::logic_error{(std::string) "Argument `" + py::repr(a).cast<std::string>() + "` of function `" +
                             py::repr(async_pyfunction).cast<std::string>() + "` is not annotated"};
    return arena::get_pycomponent(environment, entity, pyannotations_by_arg[a]);
  };

  py::list pyannotations;
  auto pyannotations_view =
      pyargs | drop_n(ismethod_pyfunction(async_pyfunction).cast<bool>() ? 1 : 0) | map(get_arg_annotation);
  for (auto &&pyannotation : pyannotations_view)
    pyannotations.append(FWD(pyannotation));

  return async_pyfunction(*pyannotations);
}

} // namespace

PyAsyncSystem::PyAsyncSystem(arena::Environment &environment, entt::entity entity, const py::function &async_pyfunction)
    : PyAsyncSystem{async_pyfunction} {
  prepare_coroutine(environment, entity);
}

PyAsyncSystem::PyAsyncSystem(const pybind11::function &async_pyfunction) {
  auto iscoroutinefunction_pyfunction = py::module_::import("inspect").attr("iscoroutinefunction");

  if (!iscoroutinefunction_pyfunction(async_pyfunction).cast<bool>())
    throw std::invalid_argument{"`PyAsyncSystem` constructor only accepts async functions as parameters"};

  m_pycoroutine = async_pyfunction;
}

void PyAsyncSystem::prepare_coroutine(arena::Environment &environment, entt::entity entity) {
  auto iscoroutine_pyfunction = py::module_::import("inspect").attr("iscoroutine");

  if (iscoroutine_pyfunction(m_pycoroutine).cast<bool>())
    throw std::logic_error{"This instance of `PyAsyncSystem` already holds a coroutine"};

  m_pycoroutine = provide_components_to_async(environment, entity, m_pycoroutine);
}

bool PyAsyncSystem::is_ready() const {
  auto iscoroutine_pyfunction = py::module_::import("inspect").attr("iscoroutine");

  return iscoroutine_pyfunction(m_pycoroutine).cast<bool>();
}

py::object PyAsyncSystem::next() {
  using namespace pybind11::literals;

  auto iscoroutine_pyfunction = py::module_::import("inspect").attr("iscoroutine");

  if (!iscoroutine_pyfunction(m_pycoroutine).cast<bool>())
    throw std::logic_error{
        "Underlying async function was not given parameters yet (use `prepare_coroutine()` for this)"};

  return py::eval("m_pycoroutine.send(None)", py::dict{"m_pycoroutine"_a = m_pycoroutine});
}
