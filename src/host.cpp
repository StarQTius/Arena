#include <arena/component/host.hpp>

#include <unordered_map>

#include <ltl/Range/Map.h>
#include <ltl/Range/enumerate.h>
#include <ltl/operator.h>

namespace py = pybind11;

////
//// PyHost
////

//
// PyHost internals
//

static auto get_components_as_pyobjects(const auto &pytypes, const arena::FetcherMap &fetchers,
                                        arena::Environment &environment, entt::entity self) {
  auto get_component_by_pytype = [&](const auto &pytype) { return fetchers.at(pytype)(environment, self); };
  py::list component_pylist{size(pytypes)};

  for (auto &&[i, pyobject] : ltl::enumerate(pytypes | ltl::map(get_component_by_pytype)))
    component_pylist[i] = pyobject;

  return component_pylist;
}

//
// PyHost definitions
//

void arena::component::PyHost::invoke(Environment &environment, entt::entity self, const FetcherMap &fetchers) {
  m_pycallback(*get_components_as_pyobjects(m_pytypes, fetchers, environment, self));
}

std::vector<std::string> arena::component::PyHost::get_annotations(const py::function &pycallback) {
  auto inspect_pymodule = py::module::import("inspect");
  auto signature_pyfunction = inspect_pymodule.attr("signature");
  auto parameter_pylist = py::dict{signature_pyfunction(pycallback).attr("parameters")};

  auto fetch_pytype = [](auto, auto &parameter_pyobject) -> std::string {
    return py::str(parameter_pyobject.attr("annotation").attr("__name__"));
  };
  return parameter_pylist | ltl::map(ltl::unzip(fetch_pytype)) | ltl::to_vector;
}
