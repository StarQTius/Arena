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
#include <arena/the_cherry_on_the_cake/component/cherry_like.hpp>
#include <arena/the_cherry_on_the_cake/component/flavor.hpp>
#include <arena/the_cherry_on_the_cake/entity/cake_layer.hpp>
#include <arena/the_cherry_on_the_cake/entity/cherry.hpp>

#include <binder/_entity.hpp>
#include <binder/component.hpp>
#include <binder/ctor.hpp>
#include <binder/def.hpp>
#include <binder/doc.hpp>
#include <binder/property.hpp>
#include <common.hpp>

namespace py = pybind11;

using namespace arena;
using namespace py::literals;

using cake_storage_t = component::Storage<component::Stackable, b2Body *, coc::component::Flavor>;
using cherry_storage_t = component::Storage<component::Stackable, b2Body *, coc::component::CherryLike>;

template <> struct arena_component_info<cake_storage_t> {};
template <> struct arena_component_info<cherry_storage_t> {};

namespace {

template <typename T> py::iterator storage_owned_pyiterator(T &self, Environment &environment) {
  auto range = self.owned(environment);
  return py::make_iterator(range.begin(), range.end());
}

} // namespace

void initialize_the_cherry_on_the_cake(pybind11::module_ &&pymodule) {
  kind::entity<coc::entity::CakeLayer>(pymodule, "CakeLayer") | R"(Single cake layer)"                          //
      | ctor<length_t, length_t, coc::component::Flavor, std::size_t>("x"_a, "y"_a, "flavor"_a, "stack"_a = 1); //

  kind::entity<coc::entity::Cherry>(pymodule, "Cherry") | R"(Cherry entity)" | ctor<length_t, length_t>("x"_a, "y"_a);

  kind::component<cake_storage_t>(pymodule, "CakeStorage") | R"(Cake layers storage)" //
      | ctor<std::size_t>("capacity"_a)                                               //
      | def("store", &cake_storage_t::store)                                          //
      | def("remove", &cake_storage_t::remove)                                        //
      | def("pick", &cake_storage_t::pick, "x"_a, "y"_a)                              //
      | property("owned", storage_owned_pyiterator<cake_storage_t>);                  //

  kind::component<cherry_storage_t>(pymodule, "CherryStorage") | R"(Cherries storage)" //
      | ctor<std::size_t>("capacity"_a)                                                //
      | def("store", &cherry_storage_t::store)                                         //
      | def("remove", &cherry_storage_t::remove)                                       //
      | def("pick", &cherry_storage_t::pick, "x"_a, "y"_a)                             //
      | property("owned", storage_owned_pyiterator<cherry_storage_t>);                 //

  py::enum_<coc::component::Flavor>{pymodule, "Flavor"}
      .value("ICING", coc::component::Flavor::ICING)              //
      .value("CREAM", coc::component::Flavor::CREAM)              //
      .value("SPONGE_CAKE", coc::component::Flavor::SPONGE_CAKE); //
}
