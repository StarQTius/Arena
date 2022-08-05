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
#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

#include "../binder/component.hpp"
#include "../binder/ctor.hpp"
#include "../binder/def.hpp"
#include "../binder/doc.hpp"
#include "../binder/entity.hpp"
#include "../binder/internal_component.hpp"
#include "../binder/property.hpp"
#include "../binder/static_def.hpp"
#include "../box2d.hpp"
#include "../common.hpp"
#include "../physics.hpp"

namespace py = pybind11;

using namespace arena;

void initialize_sail_the_world(py::module_ &pymodule) {
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
            self.view<b2Body *, stw::component::CupColor>().each() |
            ltl::filter([](auto &&tuple) { return std::get<1>(tuple)->IsEnabled(); }) | ltl::map([&](auto &&tuple) {
              return std::tuple{std::get<0>(tuple), InternalComponentRef<b2Body *>{self, std::get<0>(tuple)},
                                std::get<2>(tuple)};
            });
        return py::make_iterator(component_view.begin(), component_view.end());
      },
      py::return_value_policy::reference_internal);

  kind::component<stw::component::CupGrabber>(pymodule, "C21_CupGrabber") //
      | ctor<std::size_t>("capacity"_a)                                   //
      | def("grab", &stw::component::CupGrabber::grab)                    //
      | def("drop", &stw::component::CupGrabber::drop)                    //
      | def("get_count", &stw::component::CupGrabber::get_count)          //
      | property("storage", &stw::component::CupGrabber::storage);        //

  kind::entity<stw::entity::Cup>(pymodule, "C21_Cup")                                //
      | ctor<length_t, length_t, stw::component::CupColor>("x"_a, "y"_a, "color"_a); //

  py::enum_<stw::component::CupColor>(pymodule, "C21_CupColor")
      .value("RED", stw::component::CupColor::RED)
      .value("GREEN", stw::component::CupColor::GREEN);
}
