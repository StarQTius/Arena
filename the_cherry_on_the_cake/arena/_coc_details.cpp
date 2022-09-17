#include <variant>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <tl/expected.hpp>

#include <arena/component/body.hpp>
#include <arena/component/stackable.hpp>
#include <arena/component/storage.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>
#include <arena/the_cherry_on_the_cake/component/flavor.hpp>
#include <arena/the_cherry_on_the_cake/entity/cake_layer.hpp>

#include <binder/_entity.hpp>
#include <binder/component.hpp>
#include <binder/ctor.hpp>
#include <binder/def.hpp>
#include <binder/doc.hpp>
#include <binder/property.hpp>

namespace py = pybind11;

using namespace arena;
using namespace py::literals;

using storage_t = component::Storage<b2Body *, coc::component::Flavor, component::Stackable>;

template <> struct arena_component_info<storage_t> {};

namespace {

py::iterator storage_owned_pyiterator(storage_t &self, Environment &environment) {
  auto range = self.owned(environment);
  return py::make_iterator(range.begin(), range.end());
}

} // namespace

PYBIND11_MODULE(_coc_details, pymodule) {
  kind::entity<coc::entity::CakeLayer>(pymodule, "CakeLayer") | R"(Single cake layer)"                          //
      | ctor<length_t, length_t, coc::component::Flavor, std::size_t>("x"_a, "y"_a, "flavor"_a, "stack"_a = 1); //

  kind::component<storage_t>(pymodule, "Storage") | R"(Cake layers storage)" //
      | ctor<std::size_t>("capacity"_a)                                      //
      | def("store", &storage_t::store)                                      //
      | def("remove", &storage_t::remove)                                    //
      | def("pick", &storage_t::pick, "x"_a, "y"_a)                          //
      | property("owned", storage_owned_pyiterator);                         //

  py::enum_<coc::component::Flavor>{pymodule, "Flavor"}
      .value("ICING", coc::component::Flavor::ICING)              //
      .value("CREAM", coc::component::Flavor::CREAM)              //
      .value("SPONGE_CAKE", coc::component::Flavor::SPONGE_CAKE); //
}
