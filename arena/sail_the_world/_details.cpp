#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <type_traits>
#include <unordered_set>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
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

  register_fetcher("C21_CupGrabber", get_component_with_environment<component::c21::CupGrabber>);
  register_fetcher("C21_CupColor", get_component_with_environment<component::c21::CupColor>);

  py::class_<WithEnvironment<component::c21::CupGrabber>>(pymodule, "C21_CupGrabber")
      .def("grab", [](WithEnvironment<component::c21::CupGrabber> &self,
                      entt::entity target) { return self.value.get().grab(self.environment.get(), target); })
      .def("drop", [](WithEnvironment<component::c21::CupGrabber> &self,
                      const entity::c21::Cup &cup) { return self.value.get().drop(self.environment.get(), cup); })
      .def("get_count",
           [](WithEnvironment<component::c21::CupGrabber> &self, component::c21::CupColor color) {
             return std::count_if(self.value.get().storage.begin(), self.value.get().storage.end(), [&](auto entity) {
               return self.environment.get().registry.get<component::c21::CupColor>(entity) == color;
             });
           })
      .def_property_readonly(
          "storage", [](const WithEnvironment<component::c21::CupGrabber> &self) { return self.value.get().storage; });

  py::class_<entity::c21::Cup>(pymodule, "C21_Cup")
      .def(py::init([](precision_t x, precision_t y, component::c21::CupColor color) {
             return entity::c21::Cup{x * m, y * m, color};
           }),
           "x"_a, "y"_a, "color"_a);

  py::enum_<component::c21::CupColor>(pymodule, "C21_CupColor")
      .value("RED", component::c21::CupColor::RED)
      .value("GREEN", component::c21::CupColor::GREEN);
}