#include <string>
#include <type_traits>
#include <unordered_set>

#include <box2d/b2_body.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <ltl/Range/Filter.h>
#include <ltl/Range/Map.h>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/2021/cup.hpp>
#include <arena/binding/fetcher.hpp>
#include <arena/concept.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

#include "../binder.hpp"
#include "../common.hpp"

namespace py = pybind11;

using namespace arena;

void initialize_c21(py::module_ &pymodule) {
  using namespace py::literals;
  using namespace std::literals::string_literals;
  using namespace units::isq::si::literals;
  using namespace units::isq::si::length_references;
  using namespace units::isq::si::mass_references;
  using namespace units::isq::si::time_references;

  pymodule.def(
      "C21_cups",
      [](Environment &self) {
        auto component_view =
            self.registry.view<b2Body *, component::c21::CupColor>().each() |
            ltl::filter([](auto &&tuple) { return std::get<1>(tuple)->IsEnabled(); }) | ltl::map([&](auto &&tuple) {
              return std::tuple{std::get<0>(tuple), InternalComponentRef<b2Body *>{self, std::get<0>(tuple)},
                                std::get<2>(tuple)};
            });
        return py::make_iterator(component_view.begin(), component_view.end());
      },
      py::return_value_policy::reference_internal);

  kind::component<component::c21::CupGrabber>(pymodule, "C21_CupGrabber") //
      | ctor<std::size_t>("capacity"_a)                                   //
      | def("grab", &component::c21::CupGrabber::grab)                    //
      | def("drop", &component::c21::CupGrabber::drop)                    //
      | def("get_count", &component::c21::CupGrabber::get_count)          //
      | property("storage", &component::c21::CupGrabber::storage);        //

  kind::entity<entity::c21::Cup>(pymodule, "C21_Cup")                                //
      | ctor<length_t, length_t, component::c21::CupColor>("x"_a, "y"_a, "color"_a); //

  py::enum_<component::c21::CupColor>(pymodule, "C21_CupColor")
      .value("RED", component::c21::CupColor::RED)
      .value("GREEN", component::c21::CupColor::GREEN);
}
