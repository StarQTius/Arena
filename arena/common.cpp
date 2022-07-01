#include "common.hpp"

#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

#include <arena/binding/fetcher.hpp>
#include <arena/concept.hpp>
#include <arena/environment.hpp>

namespace py = pybind11;

using namespace arena;

namespace {

FetcherMap fetchers;

} // namespace

void register_fetcher(const std::string &pytype_name, const Fetcher &fetcher) {
  fetchers.emplace(pytype_name, fetcher);
}

const FetcherMap &get_fetchers() { return fetchers; }

py::object fetch_component(Environment &environment, entt::entity id, py::object pytype) {
  return fetchers.at(pytype.attr("__name__").cast<std::string>())(environment, id);
}
