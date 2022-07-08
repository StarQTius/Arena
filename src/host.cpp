#include <arena/component/host.hpp>

#include <array>
#include <iterator>

#include <ltl/Range/Map.h>
#include <ltl/Range/Zip.h>
#include <ltl/Range/enumerate.h>
#include <ltl/functional.h>
#include <ltl/ltl.h>
#include <ltl/operator.h>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <arena/binding/fetcher.hpp>
#include <arena/environment.hpp>

namespace py = pybind11;

////
//// PyHost
////

//
// PyHost internals
//

static auto get_components_as_pyobjects(const auto &pytypes, arena::Environment &environment, entt::entity self) {
  auto get = [&](const auto &pytype) { return get_pycomponent(environment, self, pytype); };
  py::list component_pylist{size(pytypes)};

  for (auto &&[i, pyobject] : ltl::enumerate(pytypes | ltl::map(get)))
    component_pylist[i] = pyobject;

  return component_pylist;
}

//
// PyHost definitions
//

void arena::component::PyHost::invoke(Environment &environment, entt::entity self) {
  m_pycallback(*get_components_as_pyobjects(m_pytypes, environment, self));
}

std::vector<py::object> arena::component::PyHost::get_annotations(const py::function &pycallback) {
  auto inspect_pymodule = py::module::import("inspect");
  auto signature_pyfunction = inspect_pymodule.attr("signature");
  auto parameter_pylist = py::dict{signature_pyfunction(pycallback).attr("parameters")};

  auto fetch_pytype = [](auto, auto &parameter_pyobject) -> py::object {
    return parameter_pyobject.attr("annotation");
  };
  return parameter_pylist | ltl::map(ltl::unzip(fetch_pytype)) | ltl::to_vector;
}
